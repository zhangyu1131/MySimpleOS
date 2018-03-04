#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

//��ʼ������ 
void fifo8_init(struct structFifo *fifo, int size, unsigned char *buf,struct TASK *task)
{
	fifo->size=size;
	fifo->buf=buf;
	fifo->free=size;
	fifo->flags=0;
	fifo->next_r=0;
	fifo->next_w=0;
	fifo->task=task;//������д��ʱҪ���ѵ�����
}


//������д��һ������
int fifo8_put(struct structFifo *fifo, unsigned char data)
{
	if(fifo->free==0)//���
	{
		fifo->flags|=FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->next_w]=data;
	fifo->next_w++;
	if(fifo->next_w==fifo->size)
		fifo->next_w=0;
	fifo->free--;
	
	//��������
	if(fifo->task!=0)
	{
		if(fifo->task->flags!=2)//�����������״̬
		{
			task_run(fifo->task,-1,0);
		}
	}
	return 0;
}

//�Ӷ�����ȡ��һ������
int fifo8_get(struct structFifo *fifo)
{
	int data;
	if(fifo->free==fifo->size)//����Ϊ��
	{
		return -1;
	}
	data=fifo->buf[fifo->next_r];
	fifo->next_r++;
	if(fifo->next_r==fifo->size)
	{
		fifo->next_r=0;
	}
	fifo->free++;
	return data;
}

//�����Ѵ�ŵ���������
int fifo8_status(struct structFifo *fifo)
{
	return fifo->size-fifo->free;
}
