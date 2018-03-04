//pit:�ɱ�̵ļ���Ͷ�ʱ��
#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1//������״̬
#define TIMER_FLAGS_USING 2//��ʱ��������

struct TIMERCTL timerctl;

void init_pit()
{
	int i;
	io_out8(PIT_CTRL,0x34);//���˿�0x0043����0x34
	io_out8(PIT_CNT0,0x9c);//�趨ֵΪ11932�Ļ�ʱ���ж�Ƶ�ʴ��Ϊ100HZ��11932ʮ������Ϊ2e9c
	io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++)
	{
		timerctl.timer[i].flags=0;//δʹ��
	}
	
}

//���䶨ʱ��
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

//�ͷŶ�ʱ��
void timer_free(struct TIMER *timer)
{
	timer->flags=0;
}

//��ʼ����ʱ��
void timer_init(struct TIMER *timer,struct structFifo *fifo,unsigned char data)
{
	timer->fifo=fifo;
	timer->data=data;
}

//���ö�ʱʱ��
void timer_settime(struct TIMER *timer,unsigned int timeout)
{
	timer->timeout=timeout+timerctl.count;
	timer->flags=TIMER_FLAGS_USING;
	if (timerctl.next > timer->timeout) {
		timerctl.next = timer->timeout;
	}
}

//�жϴ���
void inthandler20(int *esp)
{
	int i;
	char ts=0;
	io_out8(PIC0_OCW2,0x60);//���źŽ�����ϵ���Ϣ֪ͨ��PIC����֤����������һ��
	timerctl.count++;
	//û����һʱ��
	if(timerctl.next>timerctl.count)
		return;
	
	timerctl.next=0xffffffff;
	for(i=0;i<MAX_TIMER;i++)
	{
		if(timerctl.timer[i].flags==TIMER_FLAGS_USING)
		{
			//��ʱ
				if(timerctl.timer[i].timeout<=timerctl.count)
				{
					timerctl.timer[i].flags=TIMER_FLAGS_ALLOC;
					//�жϳ�ʱ��ʱ���ǲ���task_timer
					if(&timerctl.timer[i] != task_timer)
					{
						fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
					}
					else
					{
						ts=1;
					}				
				}
				//��û��ʱ
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


