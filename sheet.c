#include "bootpack.h"

#define SHEET_USE 1

struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl=(struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));
	if(ctl==0)
		return ctl;
	
	
	ctl->map=(unsigned char *)memman_alloc_4k(memman,xsize*ysize);
	if(ctl->map==0)
	{
		memman_free_4k(memman,(int)ctl,sizeof(struct SHTCTL));
		return ctl;
	}
	
	ctl->vram=vram;
	ctl->xsize=xsize;
	ctl->ysize=ysize;
	ctl->top=-1;
	
	for(i=0;i<MAX_SHEETS;i++)
	{
		ctl->sheets0[i].flags=0;//初始化时标记为未使用
	}
	return ctl;
}

//设定图层的缓冲区和透明色
void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv)
{
	sht->buf=buf;
	sht->bxsize=xsize;
	sht->bysize=ysize;
	sht->col_inv=col_inv;
}

//取得新的未使用图层
struct SHEET *sheet_get(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i=0;i<MAX_SHEETS;i++)
	{
		//找到未使用的图层
		if(ctl->sheets0[i].flags==0)
		{
			sht=&ctl->sheets0[i];
			sht->flags=SHEET_USE;
			sht->height=-1;
			sht->task=0;
			return sht;			
		}
	}
	return 0;//没有未使用的图层
}

//设置图层高度
void sheet_updown(struct SHTCTL *ctl,struct SHEET *sht,int newHeight)
{
	int h,oldHeight=sht->height;//存储设置前的高度信息
	
	//若高度太高或太低
	if(newHeight>ctl->top+1)
		newHeight=ctl->top+1;
	if(newHeight<-1)
		newHeight=-1;//-1表示不显示
	
	//设置高度
	sht->height=newHeight;
	
	//高度设置后sheets[]重新进行排序
	if(oldHeight>newHeight)//新高度降低
	{
		if(newHeight>=0)//高度不是-1
		{
			//重新排序
			for(h=oldHeight;h>newHeight;h--)
			{
				ctl->sheets[h]=ctl->sheets[h-1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[newHeight]=sht;
			sheet_freshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,newHeight+1);
			sheet_newrefresh(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,
								newHeight+1,oldHeight);
		}
		else//高度-1，即隐藏
		{
			if(ctl->top>oldHeight)
			{
				for(h=oldHeight;h<ctl->top;h++)
				{
					ctl->sheets[h]=ctl->sheets[h+1];
					ctl->sheets[h]->height=h;
				}
			}
			ctl->top--;
		}
		sheet_freshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0);
		sheet_newrefresh(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0,oldHeight-1);//刷新图层集
	}
	else//新高度没有降低
	{
		if(oldHeight>=0)
		{
			for(h=oldHeight;h<newHeight;h++)
			{
				ctl->sheets[h]=ctl->sheets[h+1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[newHeight]=sht;
		}
		else//隐藏变为显示
		{
			for(h=ctl->top;h>=newHeight;h--)
			{
				ctl->sheets[h+1]=ctl->sheets[h];
				ctl->sheets[h+1]->height=h+1;
			}
			ctl->sheets[newHeight]=sht;
			ctl->top++;
		}
		sheet_freshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,newHeight);
		sheet_newrefresh(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,newHeight,newHeight);
	}
}



//平移图层
void sheet_slide(struct SHTCTL *ctl,struct SHEET *sht,int vx0,int vy0)
{
	int old_vx0=sht->vx0, old_vy0=sht->vy0;
	sht->vx0=vx0;
	sht->vy0=vy0;
	if(sht->height>=0)
	{
		sheet_freshmap(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0);
		sheet_freshmap(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height);
		
		sheet_newrefresh(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0,sht->height-1);//刷新旧位置
		sheet_newrefresh(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height,sht->height);
	}
}

//释放已使用图层的内存
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if(sht->height>=0)
	{
		sheet_updown(ctl,sht,-1);
	}
	sht->flags=0;
}

//新的刷新，只刷新需要刷新的部分
void sheet_newrefresh(struct SHTCTL *ctl,int vx0, int vy0, int vx1,int vy1,int h0,int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, bx2, sid4, i, i1, *p, *q, *r;
	unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	/* 若超出屏幕边界，则进行修正 */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= h1; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht - ctl->sheets0;
		//逆算
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		if ((sht->vx0 & 3) == 0) {
			/*4字节形*/
			i  = (bx0 + 3) / 4; /* bx0除以4，小数进位 */
			i1 =  bx1      / 4; /* bx1除以4，小数舍去 */
			i1 = i1 - i;
			sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx++) {	/* 前面被4除多余的部分逐个字节写入 */
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid) {
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
				vx = sht->vx0 + bx;
				p = (int *) &map[vy * ctl->xsize + vx];
				q = (int *) &vram[vy * ctl->xsize + vx];
				r = (int *) &buf[by * sht->bxsize + bx];
				for (i = 0; i < i1; i++) {							/* 4的倍数部分 */
					if (p[i] == sid4) {//大多数情况下四字节颜色都一样，都属于同一图层
						q[i] = r[i];
					} else {//若p[i]!=sid4，则在窗口重叠边缘需一字节一字节检查
						bx2 = bx + i * 4;
						vx = sht->vx0 + bx2;
						if (map[vy * ctl->xsize + vx + 0] == sid) {
							vram[vy * ctl->xsize + vx + 0] = buf[by * sht->bxsize + bx2 + 0];
						}
						if (map[vy * ctl->xsize + vx + 1] == sid) {
							vram[vy * ctl->xsize + vx + 1] = buf[by * sht->bxsize + bx2 + 1];
						}
						if (map[vy * ctl->xsize + vx + 2] == sid) {
							vram[vy * ctl->xsize + vx + 2] = buf[by * sht->bxsize + bx2 + 2];
						}
						if (map[vy * ctl->xsize + vx + 3] == sid) {
							vram[vy * ctl->xsize + vx + 3] = buf[by * sht->bxsize + bx2 + 3];
						}
					}
				}
				for (bx += i1 * 4; bx < bx1; bx++) {				/*  后面被4除多余的部分逐个字节写入 */
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid) {
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		} else {
			/* 1字节型 */
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx++) {
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid) {
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		}
	}
	return;
}
//刷新图层集
void sheet_refresh(struct SHTCTL *ctl,struct SHEET *sht,int bx0,int by0,int bx1,int by1)
{
	if(sht->height>=0)
	{
		sheet_newrefresh(ctl,sht->vx0+bx0,sht->vy0+by0,sht->vx0+bx1,sht->vy0+by1,sht->height,sht->height);
	}
}


//刷新map
void sheet_freshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1,sid4,*p;
	unsigned char *buf,sid,*map=ctl->map;
	struct SHEET *sht;
	
	//边界调整
	if(vx0<=0)
		vx0=0;
	if(vy0<0)
		vy0=0;
	if(vx1>ctl->xsize)
		vx1=ctl->xsize;
	if(vy1>ctl->ysize)
		vy1=ctl->ysize;
	
	for(h=h0;h<=ctl->top;h++)
	{		
		sht=ctl->sheets[h];
		sid=sht-ctl->sheets0;//将进行了减法运算的地址作为图层号码使用
		buf=sht->buf;
		bx0=vx0-sht->vx0;
		by0=vy0-sht->vy0;
		bx1=vx1-sht->vx0;
		by1=vy1-sht->vy0;
		if(bx0<0)
			bx0=0;
		if(by0<0)
			by0=0;
		if(bx1>sht->bxsize)
			bx1=sht->bxsize;
		if(by1>sht->bysize)
			by1=sht->bysize;
		if(sht->col_inv==-1)//无透明色图层专用的高速版,
		{
			if((sht->vx0&3)==0&&(bx0&3)==0&&(bx1&3)==0)//4字节
			{
				bx1=(bx1-bx0)/4;
				sid4=sid | sid<<8 | sid<<16 | sid<<24;
				for(by=by0;by<by1;by++)
				{
					vy=sht->vy0+by;
					vx=sht->vx0+bx0;
					p=(int *)&map[vy*ctl->xsize+vx];
					for(bx=0;bx<bx1;bx++)
					{
						p[bx]=sid4;
					}
				}
				
			}
			else {//一字节
				for(by=by0;by<by1;by++)
				{
					vy=sht->vy0+by;
					for(bx=bx0;bx<bx1;bx++)
					{
						vx=sht->vx0+bx;
						//if(buf[by*sht->bxsize+bx]!=sht->col_inv)
							map[vy*ctl->xsize+vx]=sid;
					}
				}
			}
			
		}
		else {
			for(by=by0;by<by1;by++)
			{
				vy=sht->vy0+by;
				for(bx=bx0;bx<bx1;bx++)
				{
					vx=sht->vx0+bx;
					if(buf[by*sht->bxsize+bx]!=sht->col_inv)
						map[vy*ctl->xsize+vx]=sid;
				}
			}
		}	
	}
}