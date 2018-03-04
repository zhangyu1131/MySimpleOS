//pit:可编程的间隔型定时器
#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1//已配置状态
#define TIMER_FLAGS_USING 2//定时器运行中

struct TIMERCTL timerctl;

void init_pit()
{
	int i;
	io_out8(PIT_CTRL,0x34);//往端口0x0043发送0x34
	io_out8(PIT_CNT0,0x9c);//设定值为11932的话时钟中断频率大概为100HZ，11932十六进制为2e9c
	io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++)
	{
		timerctl.timer[i].flags=0;//未使用
	}
	
}

//分配定时器
struct TIMER *timer_alloc()
{
	int i;
	for(i=0;i<MAX_TIMER;i++)
	{
		if(timerctl.timer[i].flags==0)
		{
			timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}

//释放定时器
void timer_free(struct TIMER *timer)
{
	timer->flags=0;
}

//初始化定时器
void timer_init(struct TIMER *timer,struct structFifo *fifo,unsigned char data)
{
	timer->fifo=fifo;
	timer->data=data;
}

//设置定时时长
void timer_settime(struct TIMER *timer,unsigned int timeout)
{
	timer->timeout=timeout+timerctl.count;
	timer->flags=TIMER_FLAGS_USING;
	if (timerctl.next > timer->timeout) {
		timerctl.next = timer->timeout;
	}
}

//中断处理
void inthandler20(int *esp)
{
	int i;
	char ts=0;
	io_out8(PIC0_OCW2,0x60);//把信号接收完毕的消息通知给PIC，保证继续监听下一个
	timerctl.count++;
	//没到下一时间
	if(timerctl.next>timerctl.count)
		return;
	
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++)
	{
		if(timerctl.timer[i].flags==TIMER_FLAGS_USING)
		{
			//超时
				if(timerctl.timer[i].timeout<=timerctl.count)
				{
					timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
					//判断超时计时器是不是task_timer
					if(&timerctl.timer[i] != task_timer)
					{
						fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
					}
					else
					{
						ts=1;
					}				
				}
				//还没超时
				else{
					if(timerctl.next>timerctl.timer[i].timeout)
						timerctl.next=timerctl.timer[i].timeout;
				}
		}
	}
	if(ts!=0)
	{
		task_switch();
	}
	
}


