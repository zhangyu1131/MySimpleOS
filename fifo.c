#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

//初始化队列 
void fifo8_init(struct structFifo *fifo, int size, unsigned char *buf,struct TASK *task)
{
	fifo->size=size;
	fifo->buf=buf;
	fifo->free=size;
	fifo->flags=0;
	fifo->next_r=0;
	fifo->next_w=0;
	fifo->task=task;//有数据写入时要唤醒的任务
}


//向队列中存放一个数据
int fifo8_put(struct structFifo *fifo, unsigned char data)
{
	if(fifo->free==0)//溢出
	{
		fifo->flags|=FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->next_w]=data;
	fifo->next_w++;
	if(fifo->next_w==fifo->size)
		fifo->next_w=0;
	fifo->free--;
	
	//唤醒任务
	if(fifo->task!=0)
	{
		if(fifo->task->flags!=2)//如果处于休眠状态
		{
			task_run(fifo->task,-1,0);
		}
	}
	return 0;
}

//从队列中取得一个数据
int fifo8_get(struct structFifo *fifo)
{
	int data;
	if(fifo->free==fifo->size)//队列为空
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

//返回已存放的数据数量
int fifo8_status(struct structFifo *fifo)
{
	return fifo->size-fifo->free;
}
