//�ṹ�壬�洢��Ļͼ����Ϣ asmhead.nas
struct BOOTINFO{
	char cyls,leds,vmode,reserve;
	short scrnx,scrny;
	char *vram;//ͼ�񻺳����Ŀ�ʼ��ַ,4���ֽ�
};
#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

//���GDT8�ֽ�����
struct SEGMENT_DESCRIPTOR
{
	short limit_low,base_low;
	char base_mid,access_right;
	char limit_high,base_high;
};

//���IDT8�ֽ�����
struct GATE_DESCRIPTOR
{
	short offset_low,selector;
	char dw_count,access_right;
	short offset_high;
};

//��ɫ
#define COL8_000000		0//��
#define COL8_FF0000		1//����
#define COL8_00FF00		2//����
#define COL8_FFFF00		3//����
#define COL8_0000FF		4//����
#define COL8_FF00FF		5//����
#define COL8_00FFFF		6//ǳ����
#define COL8_FFFFFF		7//��
#define COL8_C6C6C6		8//����
#define COL8_840000		9//����
#define COL8_008400		10//����
#define COL8_848400		11//����
#define COL8_000084		12//����
#define COL8_840084		13//����
#define COL8_008484		14//ǳ����
#define COL8_848484		15//����


/* naskfunc.nas */
void load_idtr(int limit, int addr);
void io_sti(void);
void io_stihlt(void);
void io_hlt();	//��ͣ
void write_mem8(int addr,int data);//���ڴ�ֱ��д���ݣ�����ָ�����
void io_cli();//��ֹ�ж�
void io_out8(int port,int data);//���
int io_load_eflags();//��¼�ж���ɱ�־��ֵ
void io_store_eflags(int eflags);//�ָ��ж���ɱ�־
void init_screen(char *vram, int x, int y);//������Ļ
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler20(void);
int load_cr0();
void store_cr0(int cr0);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void loar_tr(int tr);//����tr
void taskswitch(int eip, int cs);//�����л�
void asm_cons_putchar(void);
void farcall(int eip,int cs);
void asm_hrb_api();
void asm_inthandler0d(void);
void asm_inthandler0c(void);
void start_app(int eip,int cs,int esp,int ds,int *tss_esp0);
void asm_end_app();

void init_palette();//��ʼ����ɫ��
void set_palette(int start,int end ,unsigned char *rgb);//���õ�ɫ��
//������
void boxfill8(unsigned char *vramBeginAddr, int xsize, unsigned char color, int xBegin, int yBegin, int xEnd, int yEnd);
//���һ���ַ�
void putfont8(char *vram, int xsize, int x, int y, char color, char *font);
//����ַ���
void putString(char *vram,int xsize,int x,int y,char color,unsigned char *s);
//��ȡ����ɫ
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize);
//�������
void init_mouse(char *mouse, char backcolor);
void init_screen(char *vram, int x, int y);

//GDT��IDT��غ���	
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_TSS32		0x0089




/* fifo.c */
struct structFifo{
	unsigned char *buf;
	int next_r,next_w,size,free,flags;//size���ֽ�����free���滺������û�����ݵ��ֽ���,flags�Ƿ����
	struct TASK *task;//Ҫ���ѵ�����
};

void fifo8_init(struct structFifo *fifo, int size, unsigned char *buf,struct TASK *task);
int fifo8_put(struct structFifo *fifo, unsigned char data);
int fifo8_get(struct structFifo *fifo);
int fifo8_status(struct structFifo *fifo);





/* int.c */
void init_pic();
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1



/*keyboard.c*/
#define KEY_PORT 				0x0060
#define PORT_KEYDAT 			0x0060
#define PORT_KEYSTA 			0x0064
#define PORT_KEYCMD 			0x0064
#define KEYSTA_SEND_NOTREADY 	0x02
#define KEYCMD_WRITE_MODE 		0x60
#define KBC_MODE 				0x47//��������ģʽ����
void wait_KBC_sendready();
void init_keyboard();
void inthandler21(int *esp);




/*mouse.c*/
struct MOUSE_DEC{
	unsigned char mbuf[3],index;
	int x,y,btn;
};

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
void enable_mouse(struct MOUSE_DEC *mouse_dse);
int mouse_receive(struct MOUSE_DEC *mouse_dse,unsigned char data);
void inthandler2c(int *esp);


/*memory.c*/
#define MEMMAN_ADDR 0x003c0000
#define MEMMAN_FREES 4090
#define EFLAGS_AC_BIT 		0x00040000
#define CR0_CATCH_DISABLE	0x60000000
struct FREEINFO//������Ϣ�ṹ��
{
	unsigned int addr,size;
};
//�ڴ����ṹ��
struct MEMMAN
{
	int frees,maxfrees,lostsize,lostcount;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size);



/*sheet.c*/
#define MAX_SHEETS 256
/*
buf����¼ͼ�������軭���ݵĵ�ַ
bxsize*bysize:ͼ�������С
vx0,xy0:ͼ���ڻ�����λ�õ�����
col_inv��͸��ɫɫ��
height���߶�
flags������й�ͼ��ĸ����趨��Ϣ
*/
struct SHEET{
	unsigned char *buf;
	int bxsize,bysize,vx0,vy0,col_inv,height,flags;
	struct TASK *task;
};

struct SHTCTL{
	unsigned char *vram,*map;
	int xsize,ysize,top;
	struct SHEET *sheets[MAX_SHEETS];//���߶���������
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize);
void sheet_setbuf(struct SHEET *sht,unsigned char *buf,int xsize,int ysize,int col_inv);
struct SHEET *sheet_get(struct SHTCTL *ctl);
void sheet_updown(struct SHTCTL *ctl,struct SHEET *sht,int newHeight);
void sheet_refresh(struct SHTCTL *ctl,struct SHEET *sht,int bx0,int by0,int bx1,int by1);
void sheet_slide(struct SHTCTL *ctl,struct SHEET *sht,int vx0,int vy0);
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
void sheet_newrefresh(struct SHTCTL *ctl,int vx0, int vy0, int vx1,int vy1,int h0,int h1);
void sheet_freshmap(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0);


/*timer.c*/
#define MAX_TIMER 500
struct TIMER{
	unsigned int timeout,flags;
	struct structFifo *fifo;
	unsigned char data;
};
struct TIMERCTL{
	unsigned int count,next;
	struct TIMER timer[MAX_TIMER];
};

void init_pit();
void inthandler20(int *esp);
struct TIMER *timer_alloc();
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer,struct structFifo *fifo,unsigned char data);
void timer_settime(struct TIMER *timer,unsigned int timeout);
int timer_cancel(struct TIMER *timer);


/*mtask.c*/
#define MAX_TASKS 1000
#define TASK_GDT0 3//�����GDT��3�ſ�ʼ�����TSS
#define MAX_TASKL_LV 100
#define MAX_TASKLEVELS 10
//����״̬�Σ�ժ��CPU��������
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK{
	int gdtnum,flags;//gdtnum���gdt���
	int level,priority;//���ȼ�
	struct structFifo fifo;
	struct TSS32 tss;
	struct CONSOLE *cons;
	int ds_base,cons_stack;
};

struct TASKLEVEL{
	int runningnum;
	int now;
	struct TASK *tasks[MAX_TASKL_LV];
};

struct TASKCTL{
	int now_lv;
	char lv_change;//���´������л�ʱ�Ƿ���Ҫ�ı�level
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

extern struct TIMER *task_timer;
struct TASK *task_now();
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchlevel();
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc();
void task_run(struct TASK *task,int level,int priority);
void task_switch();
void task_sleep(struct TASK *task);
void task_idle();


/*file.c*/
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
void file_readfat(int *fat, unsigned char *img);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

/*console.c*/
struct CONSOLE{
	struct SHEET *sht;
	int cur_x,cur_y,cur_c;
	struct TIMER *timer;
};
void console_task(struct SHTCTL *shtctl,struct SHEET *sheet,unsigned int memtotal);
void cons_newline(struct SHTCTL *shtctl,struct CONSOLE *cons);
void cons_putchar(struct SHTCTL *shtctl,struct CONSOLE *cons,int chr,char move);
void cons_runcmd(struct SHTCTL *shtctl,char *cmdline,struct CONSOLE *cons,int *fat,unsigned int memtotal);
void cmd_mem(struct SHTCTL *shtctl,struct CONSOLE *cons,unsigned int memtotal);
void cmd_cls(struct SHTCTL *shtctl,struct CONSOLE *cons);
void cmd_dir(struct SHTCTL *shtctl,struct CONSOLE *cons);
void cmd_type(struct SHTCTL *shtctl,struct CONSOLE *cons, int *fat, char *cmdline);
void cmd_exit(struct CONSOLE *cons,int *fat);
int cmd_app(struct SHTCTL *shtctl,struct CONSOLE *cons,int *fat,char *cmdline);
int *hrb_api(int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax);
void cons_putstr(struct SHTCTL *shtctl,struct CONSOLE *cons,char *s);
int *inthandler0d(int *esp);
int *inthandler0c(int *esp);

/*window.c*/
void putString_refresh(struct SHTCTL *shtctl,struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_window(unsigned char *buf,int xsize,int ysize,char *title,char act);
void make_wtitle(unsigned char *buf, int xsize,char *title,char act);
void make_textbox8(struct SHEET *sht,int x0, int y0,int sx, int sy, int c);
void change_wtitle8(struct SHEET *sht, char act);


static char keytable0[54]={
		0,0,'1','2','3','4','5','6','7','8','9','0','-','=',//2---13
		0,'	','q','w','e','r','t','y','u','i','o','p','[',']',//16-27
		0,0,'a','s','d','f','g','h','j','k','l',';','\'','`',//30-41
		0,'\\','z','x','c','v','b','n','m',',','.','/'			//43-53
	};

static char keytable1[54]={
		0,0,'!','@','#','$','%','^','&','*','(',')','_','+',//2---13
		0,'	','q','w','e','r','t','y','u','i','o','p','{','}',//16-27
		0,0,'a','s','d','f','g','h','j','k','l',':','"','~',//30-41
		0,'\\','z','x','c','v','b','n','m','<','>','?'			//43-53
	};
static char keytable2[54]={
		0,0,'1','2','3','4','5','6','7','8','9','0','-','=',//2---13
		0,'	','Q','W','E','R','T','Y','U','I','O','P','[',']',//16-27
		0,0,'A','S','D','F','G','H','J','K','L',';','\'','`',//30-41
		0,'\\','Z','X','C','V','B','N','M',',','.','/'			//43-53
	};
static char keytable3[54]={
		0,0,'!','@','#','$','%','^','&','*','(',')','_','+',//2---13
		0,'	','Q','W','E','R','T','Y','U','I','O','P','{','}',//16-27
		0,0,'A','S','D','F','G','H','J','K','L',':','"','~',//30-41
		0,'\\','Z','X','C','V','B','N','M','<','>','?'			//43-53
	};
	
	struct FILEINFO{
	unsigned char name[8],ext[3],type;
	char reserver[10];
	unsigned short time,date,clustno;
	unsigned int size;
};

/*ac_nask.nas*/
//void api_putchar(int c);
//void api_end();