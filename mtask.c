#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
//获取当前任务
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}
//添加任务
void task_add(struct TASK *task)
{
	
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->runningnum] = task;
	tl->runningnum++;
	task->flags = 2; 
	return;
}
//移除任务
void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	//寻找
	for (i = 0; i < tl->runningnum; i++) {
		if (tl->tasks[i] == task) {
			break;
		}
	}

	tl->runningnum--;
	if (i < tl->now) {
		tl->now--;//需要移动成员
	}
	if (tl->now >= tl->runningnum) {
		//如果now 的值出现错误，则进行修正
		tl->now = 0;
	}
	task->flags = 1; //使任务休眠

	//移动该level中其余任务
	for (; i < tl->runningnum; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}


//决定接下来切换到哪个level，即找到有效的最高层
void task_switchlevel(void)
{
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].runningnum > 0) {
			break; 
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}


struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task,*idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].gdtnum = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].runningnum = 0;
		taskctl->level[i].now = 0;
	}
	
	task = task_alloc();
	task->flags = 2;	//活动中标志
	task->priority = 2; // 0.02s
	task->level = 0;	//最高level
	task_add(task);
	task_switchlevel();	
	load_tr(task->gdtnum);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	
	//初始化闲置任务
	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);
	
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; //正在使用标志
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			task->tss.ss0=0;
			return task;
		}
	}
	return 0; 
}

void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; 
	}
	if (priority > 0) {
		task->priority = priority;
	}

	if (task->flags == 2 && task->level != level) { //改变level
		task_remove(task); 
	}
	if (task->flags != 2) {
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1; //level改变标志
	return;
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2) {//处于活动状态
		now_task = task_now();
		task_remove(task);//执行此举后flags变为1
		if (task == now_task) {//若是让自己休眠，则进行任务切换
			task_switchlevel();
			now_task = task_now(); 
			taskswitch(0, now_task->gdtnum);
		}
	}
	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->runningnum) {
		tl->now = 0;
	}
	if (taskctl->lv_change != 0) {
		task_switchlevel();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task) {
		taskswitch(0, new_task->gdtnum);
	}
	return;
}


void task_idle()
{
	for(;;)
	{
		io_hlt();
	}
}

