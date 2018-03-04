#include<stdio.h>
#include<string.h>
#include"bootpack.h"

#define KEYCMD_LED 0xed

extern struct structFifo keyfifo;
extern struct structFifo mousefifo;
extern struct TIMERCTL timerctl;

//void task_b_main(struct SHTCTL *shtctl,struct SHEET * sht_win_b);
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);
struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal);
void close_constask(struct TASK *task);
void close_console(struct SHEET *sht);

int shiftflag=0;//shift键的状态，0未按下
int capslockflag=0;//capslock状态，0off

void HariMain() //主函数名在作者自定义编译器里中写死
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;	
	struct structFifo fifo,timerfifo,timerfifo_cusor,keycmd;
	extern char hankaku[4096];
	char s[40],keybuf[32],mousebuf[128],fifobuf[128],timerbuf_cusor[8],keycmd_buf[32],*cons_fifo[2];
	struct TIMER *timer_1,*timer_cusor;
	int mx,my,i,j,x,y,mmx=-1,mmy=-1,mmx2=0;
	struct SHEET *sht=0,*key_win;
	struct MOUSE_DEC mouse_dse;
	unsigned int memtotal;
	struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
	//图层变量
	struct SHTCTL *shtctl;
	struct SHEET *sht_back,*sht_mouse,*sht_win,*sht_win_b[3];
	unsigned char *buf_back,buf_mouse[256],*buf_win,*buf_win_b,*buf_cons[2];
	struct CONSOLE *cons;
	//int cursor_x,cursor_color;
	
	struct TASK *task_b[3],*task_a,*task_cons[2],*task;
	int key_to=0;//切换窗口标志量
	
	int key_leds = (binfo->leds >> 4) & 7;//记录键盘指示灯状态
	int keycmd_wait=-1;
	
	init_gdtidt();
	init_pic();
	io_sti();//解除禁止中断
	
	
	//初始化队列
	fifo8_init(&fifo, 128, fifobuf, 0);
	*((int *) 0x0fec) = (int) &fifo;
	fifo8_init(&keyfifo, 32, keybuf,0);
	fifo8_init(&mousefifo,128,mousebuf,0);
	fifo8_init(&keycmd,32,keycmd_buf,0);
	
	init_pit();//初始化定时器
	io_out8(PIC0_IMR, 0xf8); //11111000，接收0、1和2号，1号表示键盘，2号表示从PIC,0为定时器
	io_out8(PIC1_IMR, 0xef); //11101111,接收12号中断，即鼠标
	
	//定时器2,光标
	/*fifo8_init(&timerfifo_cusor,8,timerbuf_cusor,0);
	timer_cusor=timer_alloc();
	timer_init(timer_cusor,&timerfifo_cusor,1);
	timer_settime(timer_cusor,50);//0.5s*/
	
	
	init_keyboard();
	enable_mouse(&mouse_dse);
	memtotal=memtest(0x00400000,0xbfffffff);
	memman_init(memman);
	memman_free(memman,0x00001000,0x0009e000);
	memman_free(memman,0x00400000,memtotal-0x00400000);
	
	init_palette();
	//初始化图层管理
	shtctl=shtctl_init(memman,binfo->vram,binfo->scrnx,binfo->scrny);
	
	task_a = task_init(memman);
	keyfifo.task=task_a;
	mousefifo.task=task_a;
	//timerfifo_cusor.task=task_a;
	task_run(task_a,1,2);
	//task_a->fifo=timerfifo_cusor;
	
	//sht_back
	sht_back=sheet_get(shtctl);
	buf_back=(unsigned char *)memman_alloc_4k(memman,binfo->scrnx*binfo->scrny);
	sheet_setbuf(sht_back,buf_back,binfo->scrnx,binfo->scrny,-1);//-1表示没有透明色
	init_screen(buf_back,binfo->scrnx,binfo->scrny);
	
	//sht_cons命令行窗口
	key_win=open_console(shtctl,memtotal);
	
	
	/*for(i=0;i<2;i++)
	{
		sht_cons[i]=sheet_get(shtctl);
		buf_cons[i]=(unsigned char *)memman_alloc_4k(memman,256*165);
		sheet_setbuf(sht_cons[i],buf_cons[i],256,165,-1);
		make_window(buf_cons[i],256,165,"console",0);
		make_textbox8(sht_cons[i],8,28,240,128,COL8_000000);
		task_cons[i]=task_alloc();
		task_cons[i]->tss.esp=memman_alloc_4k(memman,64*1024)+64*1024-16;
		task_cons[i]->tss.eip=(int)&console_task;
		task_cons[i]->tss.es=1*8;
		task_cons[i]->tss.cs=2*8;
		task_cons[i]->tss.ss=1*8;
		task_cons[i]->tss.ds=1*8;
		task_cons[i]->tss.fs=1*8;
		task_cons[i]->tss.gs=1*8;
		//传递参数
		*((int *) (task_cons[i]->tss.esp + 4)) = (int) shtctl;
		*((int *) (task_cons[i]->tss.esp + 8)) = (int) sht_cons[i];
		*((int *) (task_cons[i]->tss.esp + 12)) = (int) memtotal;
		task_run(task_cons[i],2,2);
		sht_cons[i]->task=task_cons[i];
		sht_cons[i]->flags|=0x20;
		cons_fifo[i]=(int *)memman_alloc_4k(memman,128*4);
		fifo8_init(&task_cons[i]->fifo,128,cons_fifo[i],task_cons[i]);
	}*/
	
	
	*((int *)0x0fe4)=(int)shtctl;//存入内存	
	
	//sht_win_b
	//task_b[3]
	/*for(i=0;i<3;i++)
	{
		sht_win_b[i]=sheet_get(shtctl);
		buf_win_b=(unsigned char *)memman_alloc_4k(memman,144*52);
		sheet_setbuf(sht_win_b[i],buf_win_b,144,52,-1);
		sprintf(s,"task_b%d",i);
		make_window(buf_win_b,144,52,s,0);
		
		task_b[i] = task_alloc();
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es=1*8;
		task_b[i]->tss.cs=2*8;
		task_b[i]->tss.ss=1*8;
		task_b[i]->tss.ds=1*8;
		task_b[i]->tss.fs=1*8;
		task_b[i]->tss.gs=1*8;
		//传递参数
		*((int *) (task_b[i]->tss.esp + 4)) = (int) shtctl;
		*((int *) (task_b[i]->tss.esp + 8)) = (int) sht_win_b[i];

		task_run(task_b[i],3,i+1);
		
	}*/
	
	
	//sht_win
	/*sht_win=sheet_get(shtctl);
	buf_win=(unsigned char *)memman_alloc_4k(memman,160*52);//分配内存记忆图层控制变量
	sheet_setbuf(sht_win,buf_win,144,52,-1);//指定颜色
	make_window(buf_win,144,52,"task_a",1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x=8;
	cursor_color=COL8_FFFFFF;*/
	
	
	//sht_mouse
	sht_mouse=sheet_get(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,16,16,99);//透明色号99
	init_mouse(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	*((int *)0x0ff0)=(int)sht_mouse;//存入内存	
	
	
	//平移位置
	sheet_slide(shtctl,sht_back,0,0);
	sheet_slide(shtctl,key_win,48,4);
	//sheet_slide(shtctl,sht_cons[1],48,10);
	sheet_slide(shtctl,sht_mouse,mx,my);
	//sheet_slide(shtctl,sht_win_b[0],168,56);
	//sheet_slide(shtctl,sht_win_b[1],8,116);
	//sheet_slide(shtctl,sht_win_b[2],168,116);
//	sheet_slide(shtctl,sht_win,64,56);
	//定义图层高度
	sheet_updown(shtctl,sht_back,0);
	sheet_updown(shtctl,key_win,1);
	//sheet_updown(shtctl,sht_cons[1],2);
	//sheet_updown(shtctl,sht_win_b[0],2);
	//sheet_updown(shtctl,sht_win_b[1],3);
	//sheet_updown(shtctl,sht_win_b[2],4);
	//sheet_updown(shtctl,sht_win,2);
	sheet_updown(shtctl,sht_mouse,2);	
	keywin_on(key_win);
	
	//sprintf(s, "(%d, %d)", mx, my);
	//putString(buf_back,binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	fifo8_put(&keycmd,KEYCMD_LED);
	fifo8_put(&keycmd,key_leds);
	
	for (;;) {
		//如果存在向键盘控制器发送的数据
		if(fifo8_status(&keycmd)>0&&keycmd_wait<0)
		{
			keycmd_wait=fifo8_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT,keycmd_wait);
		}
		
		//运行时间计数
		//sprintf(s,"run %ds",timerctl.count/100);
	//	putString_refresh(shtctl,sht_back,210,32,COL8_FFFFFF,COL8_008484,s,10);
		
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo)+
				fifo8_status(&fifo) == 0) {//鼠标键盘定时器缓冲区都为空
			//io_stihlt();
			task_sleep(task_a);
			io_sti();
		} else {
			if(key_win!=0&&key_win->flags==0)
			{
				if(shtctl->top==1)
				{
					key_win=0;
				}
				else {
					key_win=shtctl->sheets[shtctl->top-1];
					keywin_on(key_win);
				}
			}
					
					
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
			//	sprintf(s,"%02x",i);
			//	putString_refresh(shtctl,sht_back,0,32,COL8_FFFFFF,COL8_008484,s,2);
				//窗口输入字符									
					if(i<54&&i!=1&&i!=14&&i!=15&&i!=28&&i!=29&&i!=42&&key_win!=0)
					{
						/*if(key_win==sht_win)
						{
							if(shiftflag==0&&capslockflag==0)//shift off  capslock off
								s[0]=keytable0[i];
							else if(shiftflag!=0&&capslockflag==0)
								s[0]=keytable3[i];
							else if(shiftflag==0&&capslockflag!=0)
								s[0]=keytable2[i];
							else if(shiftflag!=0&&capslockflag!=0)
								s[0]=keytable1[i];
						s[1]=0;
						putString_refresh(shtctl,sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);						
						//光标后移
						cursor_x+=8;
						}*/
						fifo8_put(&key_win->task->fifo,i);						
					}
					else if(i==57&&key_win!=0)//空格
					{
						/*if(key_win==sht_win)
						{
						putString_refresh(shtctl,sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);
						cursor_x+=8;
						}*/
						fifo8_put(&key_win->task->fifo,i);
					}
					else if(i==14&&key_win!=0)//后退键
					{
						/*if(key_win==sht_win)
						{
							if(cursor_x>8)
							{
								//即在光标位置填入空格，再把光标前移
								putString_refresh(shtctl,sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);
								cursor_x-=8;
							}
						
						}*/
						fifo8_put(&key_win->task->fifo,i);
					}
					else if(i==28&&key_win!=0)//回车键
					{				
						fifo8_put(&key_win->task->fifo,104);			
					}
					else if(i==15&&key_win!=0)//tab键
					{
						keywin_off(key_win);
						j=key_win->height-1;
						if(j==0)
						{
							j=shtctl->top-1;
						}
						key_win=shtctl->sheets[j];
						keywin_on(key_win);
						/*
						if(key_to==0)
						{
							key_to=1;
							make_wtitle(buf_win,sht_win->bxsize,"task_a",0);
							make_wtitle(buf_cons,sht_cons->bxsize,"console",1);
							cursor_color=-1;//负数不显示
							boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
							fifo8_put(&task_cons->fifo,102);//命令行窗口光标开启
						}
						else{
							key_to=0;
							make_wtitle(buf_win,sht_win->bxsize,"task_a",1);
							make_wtitle(buf_cons,sht_cons->bxsize,"console",0);
							cursor_color=COL8_000000;
							fifo8_put(&task_cons->fifo,103);//命令行窗口光标关闭
						}
						sheet_refresh(shtctl,sht_win,0,0,sht_win->bxsize,21);
						sheet_refresh(shtctl,sht_cons,0,0,sht_cons->bxsize,21);
						if(cursor_color>=0)
						{
							boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
						}*/
						//sheet_refresh(shtctl,sht_win,cursor_x,28,cursor_x+8,44);
					}
					else if(i==0x2a||i==0x36)//shift按下
					{
						shiftflag=1;
					}
					else if(i==0xaa||i==0xbb)//shift弹起
					{
						shiftflag=0;
					}
					else if(i==0x3a)//按下capslock
					{
						capslockflag^=0x01;
						key_leds^=4;
						fifo8_put(&keycmd,KEYCMD_LED);
						fifo8_put(&keycmd,key_leds);
					}
					else if(i==0xfa)//键盘成功收到数据
					{
						keycmd_wait=-1;
					}
					else if(i==0xfe)//键盘未成功收到数据
					{
						wait_KBC_sendready();
						io_out8(PORT_KEYDAT,keycmd_wait);
					}
					else if(shiftflag!=0&&i==0x3b&&key_win!=0)//shift+F1强制结束
					{
						task=key_win->task;
						if(task!=0&&task->tss.ss0!=0)
						{
							io_cli();
							task->tss.eax=(int)  &(task->tss.esp0);
							task->tss.eip=(int) asm_end_app;
							io_sti();
						}
					}
					else if(shiftflag!=0&&i==0x3c&&task_cons[1]==0)//shift+F2打开新的命令行窗口
					{
						if(key_win!=0)
						{
							keywin_off(key_win);
						}	
						key_win=open_console(shtctl,memtotal);
						sheet_slide(shtctl,key_win,40,12);
						sheet_updown(shtctl,key_win,shtctl->top);
						keywin_on(key_win);
					}
				
			} 
			//鼠标数据
			else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_receive(&mouse_dse,i)==1)
				{
					//移动鼠标
					//1先隐藏鼠标
					//boxfill8(buf_back, binfo->scrnx, COL8_008484, mx, my, mx+15 , my+15);
					//2计算新的鼠标坐标
					mx+=mouse_dse.x;
					my+=mouse_dse.y;
					//3边界调整
					if(mx<0)
						mx=0;
					if(my<0)
						my=0;
					if(mx>binfo->scrnx-1)
						mx=binfo->scrnx-1;
					if(my>binfo->scrny-1)
						my=binfo->scrny-1;
					//4绘制
					//sprintf(s,"%3d,%3d",mx,my);
					boxfill8(buf_back,binfo->scrnx,COL8_008484,0,0,79,15);
					//putString(buf_back,binfo->scrnx,0,0,COL8_FFFFFF,s);
					sheet_refresh(shtctl,sht_back,0,0,80,16);
					sheet_slide(shtctl,sht_mouse,mx,my);
				}
				//点击左键
				if((mouse_dse.btn&0x01)!=0)
				{
					if(mmx<0)//寻常模式
					{
						//从上到下寻找鼠标所指图层
						for(j=shtctl->top-1;j>0;j--)
						{
							sht=shtctl->sheets[j];
							x=mx-sht->vx0;
							y=my-sht->vy0;
							if(0<=x&&x<sht->bxsize&&0<=y&&y<sht->bysize)
							{
								if(sht->buf[y*sht->bxsize+x]!=sht->col_inv)
								{
									sheet_updown(shtctl,sht,shtctl->top-1);
									if(sht!=key_win)
									{
										keywin_off(key_win);
										key_win=sht;
										keywin_on(key_win);
									}
									if(3<=x&&x<sht->bxsize-3&&3<=y&&y<21)//窗口移动模式
									{
										mmx=mx;
										mmy=my;
										mmx2=sht->vx0;
									}
									//点击叉
									if(sht->bxsize-21<=x&&x<sht->bxsize-5&&5<=y&&y<19)
									{
										if((sht->flags&0x10)!=0)//应用程序窗口
										{
											task=sht->task;
											io_cli();
											task->tss.eax=(int)&(task->tss.esp0);
											task->tss.eip=(int)asm_end_app;
											io_sti();
										}
										else{
											task=sht->task;
											io_cli();
											fifo8_put(&task->fifo,105);
											io_sti();
										}
									}
									break;
								}
							}
						}
					}
					else{
						x=mx-mmx;
						y=my-mmy;
						sheet_slide(shtctl,sht,(mmx2+x+2)&~3,sht->vy0+y);
						mmy=my;
					}
					
				}
				//未按下左键
				else{
					mmx=-1;
				}
				
			}
			else if(fifo8_status(&fifo)!=0)
			{
				i = fifo8_get(&fifo);
				//sprintf(s,"%0d",i);
				//putString_refresh(shtctl,sht_back,0,64,COL8_FFFFFF,COL8_008484,s,1);
				close_console(shtctl->sheets0+i);
			}
			/*else if(fifo8_status(&timerfifo)!=0)
			{
				i=fifo8_get(&timerfifo);
				io_sti();
				sprintf(s,"data=%d:5s",i);
				putString(buf_back,binfo->scrnx,200,16,COL8_FFFFFF,s);
				sheet_refresh(shtctl,sht_back,200,16,320,32);	
			}*/
			/*else if(fifo8_status(&timerfifo_cusor)!=0)//光标闪烁
			{
				i=fifo8_get(&timerfifo_cusor);
				io_sti();
				if(i==102)//开启光标
				{
					cursor_color=COL8_000000;
				}
				if(i==103)//关闭光标
				{
					boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cursor_x,28,cursor_x+7,43);
					cursor_color=-1;
				}
				if(i!=0&&i!=102&&i!=103)
				{
					timer_init(timer_cusor,&timerfifo_cusor,0);
					if(cursor_color>=0)
						cursor_color=COL8_000000;
				}
				else
				{
					timer_init(timer_cusor,&timerfifo_cusor,1);
					if(cursor_color>=0)
						cursor_color=COL8_FFFFFF;
				}
				timer_settime(timer_cusor,50);
				if(cursor_color>=0)
				{
					boxfill8(sht_win->buf,sht_win->bxsize,cursor_color,cursor_x,28,cursor_x+7,43);
					sheet_refresh(shtctl,sht_win,cursor_x,28,cursor_x+8,44);
				}
				sheet_refresh(shtctl,sht_win,cursor_x,28,cursor_x+8,44);
			}*/
		}
	}
		
		
}

//任务b运行函数
/*void task_b_main(struct SHTCTL *shtctl,struct SHEET * sht_win_b)
{
	struct structFifo fifoB;
	struct TIMER *timer_put;
	int i,fifobuf[128];
	int count=0;
	char s[12];
	
	fifo8_init(&fifoB,128,fifobuf,0);
	timer_put=timer_alloc();
	timer_init(timer_put,&fifoB,2);
	timer_settime(timer_put,1);
	
	for(;;)
	{
		count++;
		io_cli();
		if (fifo8_status(&fifoB)== 0)
		{
			io_stihlt();
		}
		else
		{
			i=fifo8_get(&fifoB);
			io_sti();
			if(i==2)
			{
				sprintf(s,"%10d",count);
				putString_refresh(shtctl,sht_win_b,24,28,COL8_000000,COL8_C6C6C6,s,12);
				timer_settime(timer_put,1);
			}
		}
	}
}*/

/*
int keywin_off(struct SHEET *key_win,struct SHEET *sht_win,int cur_c,int cur_x)
{
	change_wtitle8(key_win,0);
	if(key_win==sht_win)
	{
		cur_c=-1;
		boxfill8(sht_win->buf,sht_win->bxsize,COL8_FFFFFF,cur_x,28,cur_x+7,43);
	}
	else 
	{
		if((key_win->flags&0x20)!=0)
		{
			fifo8_put(&key_win->task->fifo,103);
		}
	}
	return cur_c;
}

int keywin_on(struct SHEET *key_win,struct SHEET *sht_win,int cur_c)
{
	change_wtitle8(key_win,1);
	if(key_win==sht_win)
	{
		cur_c=COL8_000000;
	}
	else 
	{
		if((key_win->flags&0x20)!=0)
		{
			fifo8_put(&key_win->task->fifo,102);
		}
	}
	return cur_c;
}*/

void keywin_off(struct SHEET *key_win)
{
	change_wtitle8(key_win,0);
	if((key_win->flags&0x20)!=0)
	{
		fifo8_put(&key_win->task->fifo,103);
	}
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win,1);
	if((key_win->flags&0x20)!=0)
	{
		fifo8_put(&key_win->task->fifo,102);
	}
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{		
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_get(shtctl);
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	struct TASK *task = task_alloc();
	//int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	char *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	sheet_setbuf(sht, buf, 256, 165, -1);
	make_window(buf, 256, 165, "console", 0);
	make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
	task->cons_stack=memman_alloc_4k(memman,64*1024);
	task->tss.esp = task->cons_stack + 64 * 1024 - 16;
	task->tss.eip = (int) &console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4))=(int)shtctl;
	*((int *) (task->tss.esp + 8)) = (int) sht;
	*((int *) (task->tss.esp + 12)) = memtotal;
	task_run(task, 2, 2);
	sht->task = task;
	sht->flags |= 0x20;	
	fifo8_init(&task->fifo, 128, cons_fifo, task);
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman,task->cons_stack,64*1024);
	memman_free_4k(memman,(int)task->fifo.buf,128*4);
	task->flags=0;
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl=(struct SHTCTL *) *((int *)0x0fe4);
	struct TASK *task=sht->task;
	memman_free_4k(memman,(int )sht->buf,256*165);
	sheet_free(shtctl,sht);
	close_constask(task);
}