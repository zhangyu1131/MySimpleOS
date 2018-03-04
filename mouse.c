#include"bootpack.h"

void enable_mouse(struct MOUSE_DEC *mouse_dse)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//往键盘控制电路发送0xd4，下一个数据就会自动发往鼠标，即MOUSECMD_ENABLE发往鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//若成功，则键盘会控制其返回0xfa
	mouse_dse->index=0;
}


int mouse_receive(struct MOUSE_DEC *mouse_dse,unsigned char data)
{
	if(mouse_dse->index==0)//等待鼠标就绪
				{
					if(data==0xfa)
					    mouse_dse->index=1;
					return 0;
				}
			 if(mouse_dse->index==1)//等待鼠标第一字节
				{
					//规范：0~3,8~F
					//data和1100 1000相与看接收到的数据是否符合要求
					if((data & 0xc8)==0x08)
					{
						mouse_dse->mbuf[0]=data;
						mouse_dse->index=2;
					}					
					return 0;
				}
				if(mouse_dse->index==2)
				{
					mouse_dse->mbuf[1]=data;
					mouse_dse->index=3;
					return 0;
				}
				if(mouse_dse->index==3)
				{
					mouse_dse->mbuf[2]=data;
					mouse_dse->index=1;
					
					mouse_dse->btn=mouse_dse->mbuf[0]&0x07;
					mouse_dse->x=mouse_dse->mbuf[1];
					mouse_dse->y=mouse_dse->mbuf[2];
					
					if((mouse_dse->mbuf[0] & 0x10)!=0)
					{
						mouse_dse->x |= 0xffffff00;
					}
					if((mouse_dse->mbuf[0]&0x20)!=0)
					{
						mouse_dse->y|= 0xffffff00;
					}
					
					mouse_dse->y= -mouse_dse->y;
					return 1;
				}
}


//来自PS/2鼠标的中断
struct structFifo mousefifo;

void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x64);//12号中断，即从PIC第四号中断
	io_out8(PIC0_OCW2,0x62);//从2号通知主PIC
	data=io_in8(KEY_PORT);
	fifo8_put(&mousefifo,data);
}