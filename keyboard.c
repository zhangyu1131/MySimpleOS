#include"bootpack.h"

//�ȴ����̿��Ƶ�·׼�����
void wait_KBC_sendready()
{
	for(;;)
	{
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY)==0)//���豸����0x0064������ȡ�����ݵĵ����ڶ�λ��0�������
		{
			break;
		}
	}
}

//һ��ȷ�Ͽɷ������̿��Ƶ�·������Ϣ��һ�߷���ģʽ�趨ָ��
void init_keyboard()
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);//���豸��0x0064��0x60�趨ģʽ
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);//�������ģʽ��ģʽ������0x47�����͵�0x0064
	
}


struct structFifo keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x61);//֪ͨPIC�Ѿ�������IRQ1��0x60+�ţ��жϣ���������
	data=io_in8(KEY_PORT);
	fifo8_put(&keyfifo,data);
	
}
