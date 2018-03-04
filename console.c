#include"bootpack.h"
#include <stdio.h>
#include <string.h>

extern int shiftflag;
extern int capslockflag;


//�����д������к���
void console_task(struct SHTCTL *shtctl,struct SHEET *sheet,unsigned int memtotal)
{
	struct TASK *task=task_now();
	struct FILEINFO *fileinfo=(struct FILEINFO *)(ADR_DISKIMG+0x002600);
	int i,x,y;
	char s[30],cmdline[30],*p;
	struct MEMMAN *memman=(struct MEMMAN *)MEMMAN_ADDR;
	int *fat=(int *)memman_alloc_4k(memman,4*2880);
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	int filefound=1;
	struct CONSOLE cons;
	cons.sht=sheet;
	cons.cur_c=-1;
	cons.cur_x=8;
	cons.cur_y=28;
	task->cons=&cons;
	//*((int *)0x0fec)=(int)&cons;//�����ڴ�
	//*((int *)0x0fe4)=(int)shtctl;//�����ڴ�	
	//fifo8_init(&task->fifo,128,fifobuf,task);
	cons.timer=timer_alloc();
	timer_init(cons.timer,&task->fifo,101);
	timer_settime(cons.timer,50);
	file_readfat(fat,(unsigned char *)(ADR_DISKIMG+0x000200));//��ѹ��
	
	//��ʾ��ʾ��
	cons_putchar(shtctl,&cons,'>',1);
	
	for(;;)
	{
		io_cli();
		if(fifo8_status(&task->fifo)==0)
		{
			task_sleep(task);
			io_sti;
		}
		else{
			i=fifo8_get(&task->fifo);
			io_sti();
			if(i==100||i==101)
			{
				if(i!=100)
				{
					timer_init(cons.timer,&task->fifo,100);
					if(cons.cur_c>=0)
						cons.cur_c=COL8_FFFFFF;
				}
				else{
					timer_init(cons.timer,&task->fifo,101);
					if(cons.cur_c>=0)
						cons.cur_c=COL8_000000;
				}
				timer_settime(cons.timer,50);		
			}
			if(i==102)//�������
			{
				cons.cur_c=COL8_FFFFFF;
			}
			if(i==103)//�رչ��
			{
				boxfill8(sheet->buf,sheet->bxsize,COL8_000000,cons.cur_x,cons.cur_y,cons.cur_x+7,cons.cur_y+15);
				cons.cur_c=-1;
			}
			if(i==104)//�յ��س���
			{	
				//����
				cons_putchar(shtctl,&cons,' ',0);
				cmdline[cons.cur_x/8-2]=0;
				cons_newline(shtctl,&cons);
				//ִ������
				cons_runcmd(shtctl,cmdline,&cons,fat,memtotal);
				//��ʾ��ʾ��
				cons_putchar(shtctl,&cons,'>',1);			
			}
			if(i<54&&i!=1&&i!=14&&i!=15&&i!=28&&i!=29&&i!=42&&cons.cur_x<240)
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
				cmdline[cons.cur_x/8-2]=s[0];
				putString_refresh(shtctl,sheet,cons.cur_x,cons.cur_y,COL8_FFFFFF,COL8_000000,s,1);						
					//������
					cons.cur_x+=8;
				
				
					
			}
			else if(i==57&&cons.cur_x<240)//�ո�
			{	
				cmdline[cons.cur_x/8-2]=' ';			
				putString_refresh(shtctl,sheet,cons.cur_x,cons.cur_y,COL8_FFFFFF,COL8_000000," ",1);
				cons.cur_x+=8;
			}
			else if(i==14&&cons.cur_x>16)//���˼�
			{
					//���ڹ��λ������ո��ٰѹ��ǰ��
					putString_refresh(shtctl,sheet,cons.cur_x,cons.cur_y,COL8_FFFFFF,COL8_000000," ",1);
					cons.cur_x-=8;
			}
			else if(i==105)//���ˡ�
			{
				cmd_exit(&cons,fat);
			}
			//������ʾ���
			if(cons.cur_c>=0)
			{
				boxfill8(sheet->buf,sheet->bxsize,cons.cur_c,cons.cur_x,cons.cur_y,cons.cur_x+7,cons.cur_y+15);
			}
			sheet_refresh(shtctl,sheet,cons.cur_x,cons.cur_y,cons.cur_x+8,cons.cur_y+16);
		}
	}
}

void cons_putchar(struct SHTCTL *shtctl,struct CONSOLE *cons,int chr,char move)
{
	char s[2];
	s[0]=chr;
	s[1]=0;
	if (s[0] == 0x09) {	//�Ʊ��
		for (;;) {
			putString_refresh(shtctl,cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(shtctl,cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	
			}
		}
	} else if (s[0] == 0x0a) {	//����
		cons_newline(shtctl,cons);
	} else if (s[0] == 0x0d) {	//�س�
	
	} else {
		putString_refresh(shtctl,cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(shtctl,cons);
			}
		}
	}
	return;
}

void cons_newline(struct SHTCTL *shtctl,struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < 28 + 112) {
		cons->cur_y += 16; //����
	} else {
		//���һ��
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(shtctl,sheet,8,28,8+240,28+128);
	}
	cons->cur_x = 8;
}


void cons_runcmd(struct SHTCTL *shtctl,char *cmdline,struct CONSOLE *cons,int *fat,unsigned int memtotal)
{
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(shtctl,cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0) {
		cmd_cls(shtctl,cons);
	} else if (strcmp(cmdline, "dir") == 0) {
		cmd_dir(shtctl,cons);
	} else if (strncmp(cmdline, "type ", 5) == 0) {
		cmd_type(shtctl,cons, fat, cmdline);
	}else if(strcmp(cmdline, "exit") == 0){
		cmd_exit(cons,fat);
	}else if (cmdline[0] != 0) {
		if(cmd_app(shtctl,cons,fat,cmdline)==0)
		{
			putString_refresh(shtctl,cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Wrong command", 13);
			cons_newline(shtctl,cons);
			cons_newline(shtctl,cons);
		}
		
	}
}

//mem����
void cmd_mem(struct SHTCTL *shtctl,struct CONSOLE *cons,unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[30];
	sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
	putString_refresh(shtctl,cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(shtctl,cons);
	sprintf(s, "free %dKB", memman_total(memman) / 1024);
	putString_refresh(shtctl,cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(shtctl,cons);
	cons_newline(shtctl,cons);
}

void cmd_cls(struct SHTCTL *shtctl,struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(shtctl,sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
}

void cmd_dir(struct SHTCTL *shtctl,struct CONSOLE *cons)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				putString_refresh(shtctl,cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
				cons_newline(shtctl,cons);
			}
		}
	}
	cons_newline(shtctl,cons);
	return;
}


void cmd_type(struct SHTCTL *shtctl,struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	int i;
	if (finfo != 0) {
		//�ҵ�
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		for (i = 0; i < finfo->size; i++) {
			cons_putchar(shtctl,cons, p[i], 1);
		}
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		putString_refresh(shtctl,cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		cons_newline(shtctl,cons);
	}
	cons_newline(shtctl,cons);
	return;
}

void cmd_exit(struct CONSOLE *cons,int *fat)
{
	struct SHTCTL *shtctl=(struct SHTCTL *) *((int *)0x0fe4);
	struct MEMMAN *memman=(struct MEMMAN*)MEMMAN_ADDR;
	struct TASK *task=task_now();
	struct structFifo *fifo=(struct structFifo *) *((int *)0x0fec);
	//timer_free(cons->timer);
	memman_free_4k(memman,(int )fat,4*2880);
	io_cli();
	fifo8_put(fifo,cons->sht-shtctl->sheets0+768);//768~1023
	io_sti();
	for(;;)
	{
		task_sleep(task);
	}
}


int cmd_app(struct SHTCTL *shtctl,struct CONSOLE *cons,int *fat,char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char name[18], *p,*q;
	struct TASK *task=task_now();
	int segsiz,datsiz,esp,dathrb;
	int i;
	struct SHEET *sht;
	//�����ļ���
	for (i = 0; i < 13; i++) {
		if(cmdline[i]<=' ')//�ո���ǰ��ascii�붼�������ļ���
			break;
		name[i] = cmdline[i];
	}
	name[i] = 0; 

	//Ѱ���ļ�
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	//�ҵ�
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		if(finfo->size>=36&&strncmp(p+4,"Hari",4)==0 && *p==0x00)
		{
			//����hrb�ļ���ʽ�޸�
			segsiz=*((int *)(p+0x0000));//Ӧ�ó������ݶδ�С
			esp=*((int *)(p+0x000c));//esp��ʼֵ���������ַ֮ǰ�Ĳ��ֶ���ջ
			datsiz=*((int *)(p+0x0010));//hrb�ļ������ݲ��ִ�С
			dathrb=*((int *)(p+0x0014));//���ݿ�ʼ��ַ
			q = (char *) memman_alloc_4k(memman, segsiz);
			task->ds_base=(int)q;
			//Ϊ��Ӧ�ó���������Σ�Ȩ�޼���0x60��ʾ�Ѹö�����ΪӦ�ó����ã����ô������ϵͳ��ֵ
			set_segmdesc(gdt + task->gdtnum/8+1000, finfo->size - 1, (int) p, AR_CODE32_ER+0x60);
			//Ϊ��Ӧ�ó�������������ݶ�
			set_segmdesc(gdt + task->gdtnum/8+2000,	segsiz-1,		 (int) q, AR_DATA32_RW+0x60);
			//��Ӧ�ó��������ȸ��Ƶ����ݶ�����������
			for(i=0;i<datsiz;i++)
			{
				q[esp+i]=p[dathrb+i];
			}
			start_app(0x1b,task->gdtnum+1000*8,esp,task->gdtnum+2000*8,&(task->tss.esp0));//0x1b,hrimain��ַ
			
			for(i=0;i<MAX_SHEETS;i++)
			{
				sht=&(shtctl->sheets0[i]);
				if((sht->flags&0x11)==0x11 && sht->task==task)
				{
					sheet_free(shtctl,sht);
				}
			}
			memman_free_4k(memman, (int)q,segsiz );
		}
		else {
			cons_putstr(shtctl,cons,".hrb file format error\n");
		}
		memman_free_4k(memman, (int) p, finfo->size);
		cons_newline(shtctl,cons);
		return 1;
	}

	return 0;//û�ҵ�
}

void cons_putstr(struct SHTCTL *shtctl,struct CONSOLE *cons,char *s)
{
	for(;*s!=0;s++)
	{
		cons_putchar(shtctl,cons,*s,1);
	}
}

int * hrb_api(int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax)
{
	struct TASK *task=task_now();
	struct CONSOLE *cons = task->cons;
	struct SHTCTL *shtctl;
	
	int ds_base=task->ds_base;
	shtctl=(struct SHTCTL *)*((int *)0x0fe4);
	struct SHEET *sht;
	int *reg=&eax+1;
	struct SHEET *sht_mouse;
	sht_mouse=(struct SHEET *)*((int *)0x0ff0);
	if (edx == 1) {
		cons_putchar(shtctl,cons, eax & 0xff, 1);
	} else if (edx == 2) {
		cons_putstr(shtctl,cons, (char *) ebx+ds_base);
	}
	else if(edx==4)//ǿ�ƽ���
	{
		return &(task->tss.esp0);
	}
	else if(edx==5)//��ʾ����
	{
		sht=sheet_get(shtctl);
		sheet_setbuf(sht,(char *)ebx+ds_base,esi,edi,eax);
		sht->task=task;
		sht->flags|=0x10;
		make_window((char *)ebx+ds_base,esi,edi,(char*) ecx+ds_base,0);
		sheet_slide(shtctl,sht,((shtctl->xsize-esi)/2)&~3,(shtctl->ysize-edi)/2);
		sheet_updown(shtctl,sht,shtctl->top);
		reg[7]=(int )sht;
	}
	else if(edx==14)
	{
		sheet_free(shtctl,(struct SHEET *)ebx);
	}
	return 0;
}


int *inthandler0d(int *esp)
{
	struct SHTCTL *shtctl=(struct SHTCTL *) *((int *)0x0fe4);
	struct TASK *task=task_now();
	struct CONSOLE *cons=task->cons;
	char s[30];
	cons_putstr(shtctl,cons,"\nINT 0D\n General Protected Exception\n");
	sprintf(s,"EIP=%08x\n",esp[11]);
	cons_putstr(shtctl,cons,s);
	return &(task->tss.esp0);
}

int *inthandler0c(int *esp)
{
	struct SHTCTL *shtctl=(struct SHTCTL *) *((int *)0x0fe4);
	struct TASK *task=task_now();
	struct CONSOLE *cons=task->cons;
	char s[30];
	cons_putstr(shtctl,cons,"\nINT 0C\n Stack Exception\n");
	sprintf(s,"EIP=%08x\n",esp[11]);
	cons_putstr(shtctl,cons,s);
	return &(task->tss.esp0);
}