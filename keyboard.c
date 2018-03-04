#include"bootpack.h"

//等待键盘控制电路准备完毕
void wait_KBC_sendready()
{
	for(;;)
	{
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0)//若设备号码0x0064处所读取的数据的倒数第二位是0，则完毕
		{
			break;
		}
	}
}

//一边确认可否往键盘控制电路传送信息，一边发送模式设定指令
void init_keyboard()
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);//往设备号0x0064发0x60设定模式
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);//利用鼠标模式的模式号码是0x47，发送到0x0064
	
}


struct structFifo keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x61);//通知PIC已经发生了IRQ1（0x60+号）中断，继续监听
	data=io_in8(KEY_PORT);
	fifo8_put(&keyfifo,data);
	
}
