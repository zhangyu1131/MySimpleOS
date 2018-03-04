#include"bootpack.h"

void enable_mouse(struct MOUSE_DEC *mouse_dse)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//�����̿��Ƶ�·����0xd4����һ�����ݾͻ��Զ�������꣬��MOUSECMD_ENABLE�������
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//���ɹ�������̻�����䷵��0xfa
	mouse_dse->index=0;
}


int mouse_receive(struct MOUSE_DEC *mouse_dse,unsigned char data)
{
	if(mouse_dse->index==0)//�ȴ�������
				{
					if(data==0xfa)
					    mouse_dse->index=1;
					return 0;
				}
			 if(mouse_dse->index==1)//�ȴ�����һ�ֽ�
				{
					//�淶��0~3,8~F
					//data��1100 1000���뿴���յ��������Ƿ����Ҫ��
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


//����PS/2�����ж�
struct structFifo mousefifo;

void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x64);//12���жϣ�����PIC���ĺ��ж�
	io_out8(PIC0_OCW2,0x62);//��2��֪ͨ��PIC
	data=io_in8(KEY_PORT);
	fifo8_put(&mousefifo,data);
}