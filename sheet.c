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
		ctl->sheets0[i].flags=0;//��ʼ��ʱ���Ϊδʹ��
	}
	return ctl;
}

//�趨ͼ��Ļ�������͸��ɫ
void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv)
{
	sht->buf=buf;
	sht->bxsize=xsize;
	sht->bysize=ysize;
	sht->col_inv=col_inv;
}

//ȡ���µ�δʹ��ͼ��
struct SHEET *sheet_get(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i=0;i<MAX_SHEETS;i++)
	{
		//�ҵ�δʹ�õ�ͼ��
		if(ctl->sheets0[i].flags==0)
		{
			sht=&ctl->sheets0[i];
			sht->flags=SHEET_USE;
			sht->height=-1;
			sht->task=0;
			return sht;			
		}
	}
	return 0;//û��δʹ�õ�ͼ��
}

//����ͼ��߶�
void sheet_updown(struct SHTCTL *ctl,struct SHEET *sht,int newHeight)
{
	int h,oldHeight=sht->height;//�洢����ǰ�ĸ߶���Ϣ
	
	//���߶�̫�߻�̫��
	if(newHeight>ctl->top+1)
		newHeight=ctl->top+1;
	if(newHeight<-1)
		newHeight=-1;//-1��ʾ����ʾ
	
	//���ø߶�
	sht->height=newHeight;
	
	//�߶����ú�sheets[]���½�������
	if(oldHeight>newHeight)//�¸߶Ƚ���
	{
		if(newHeight>=0)//�߶Ȳ���-1
		{
			//��������
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
		else//�߶�-1��������
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
		sheet_newrefresh(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,0,oldHeight-1);//ˢ��ͼ�㼯
	}
	else//�¸߶�û�н���
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
		else//���ر�Ϊ��ʾ
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



//ƽ��ͼ��
void sheet_slide(struct SHTCTL *ctl,struct SHEET *sht,int vx0,int vy0)
{
	int old_vx0=sht->vx0, old_vy0=sht->vy0;
	sht->vx0=vx0;
	sht->vy0=vy0;
	if(sht->height>=0)
	{
		sheet_freshmap(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0);
		sheet_freshmap(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height);
		
		sheet_newrefresh(ctl,old_vx0,old_vy0,old_vx0+sht->bxsize,old_vy0+sht->bysize,0,sht->height-1);//ˢ�¾�λ��
		sheet_newrefresh(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize,sht->height,sht->height);
	}
}

//�ͷ���ʹ��ͼ����ڴ�
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if(sht->height>=0)
	{
		sheet_updown(ctl,sht,-1);
	}
	sht->flags=0;
}

//�µ�ˢ�£�ֻˢ����Ҫˢ�µĲ���
void sheet_newrefresh(struct SHTCTL *ctl,int vx0, int vy0, int vx1,int vy1,int h0,int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, bx2, sid4, i, i1, *p, *q, *r;
	unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	/* ��������Ļ�߽磬��������� */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= h1; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht - ctl->sheets0;
		//����
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		if ((sht->vx0 & 3) == 0) {
			/*4�ֽ���*/
			i  = (bx0 + 3) / 4; /* bx0����4��С����λ */
			i1 =  bx1      / 4; /* bx1����4��С����ȥ */
			i1 = i1 - i;
			sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx++) {	/* ǰ�汻4������Ĳ�������ֽ�д�� */
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid) {
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
				vx = sht->vx0 + bx;
				p = (int *) &map[vy * ctl->xsize + vx];
				q = (int *) &vram[vy * ctl->xsize + vx];
				r = (int *) &buf[by * sht->bxsize + bx];
				for (i = 0; i < i1; i++) {							/* 4�ı������� */
					if (p[i] == sid4) {//�������������ֽ���ɫ��һ����������ͬһͼ��
						q[i] = r[i];
					} else {//��p[i]!=sid4�����ڴ����ص���Ե��һ�ֽ�һ�ֽڼ��
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
				for (bx += i1 * 4; bx < bx1; bx++) {				/*  ���汻4������Ĳ�������ֽ�д�� */
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid) {
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		} else {
			/* 1�ֽ��� */
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
//ˢ��ͼ�㼯
void sheet_refresh(struct SHTCTL *ctl,struct SHEET *sht,int bx0,int by0,int bx1,int by1)
{
	if(sht->height>=0)
	{
		sheet_newrefresh(ctl,sht->vx0+bx0,sht->vy0+by0,sht->vx0+bx1,sht->vy0+by1,sht->height,sht->height);
	}
}


//ˢ��map
void sheet_freshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0)
{
	int h,bx,by,vx,vy,bx0,by0,bx1,by1,sid4,*p;
	unsigned char *buf,sid,*map=ctl->map;
	struct SHEET *sht;
	
	//�߽����
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
		sid=sht-ctl->sheets0;//�������˼�������ĵ�ַ��Ϊͼ�����ʹ��
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
		if(sht->col_inv==-1)//��͸��ɫͼ��ר�õĸ��ٰ�,
		{
			if((sht->vx0&3)==0&&(bx0&3)==0&&(bx1&3)==0)//4�ֽ�
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
			else {//һ�ֽ�
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