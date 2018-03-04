#include"bootpack.h"

void init_palette()
{
	static unsigned char table_rgb[16*3]={		//调色板数组
		0x00, 0x00, 0x00,	//0黑
		0xff, 0x00, 0x00,	//1亮红
		0x00, 0xff, 0x00,	//2亮绿
		0xff, 0xff, 0x00,	//3亮黄
		0x00, 0x00, 0xff,	//4亮蓝
		0xff, 0x00, 0xff,	//5亮紫
		0x00, 0xff, 0xff,	//6浅亮蓝
		0xff, 0xff, 0xff,	//7白
		0xc6, 0xc6, 0xc6,	//8亮灰
		0x84, 0x00, 0x00,	//9暗红
		0x00, 0x84, 0x00,	//10暗绿
		0x84, 0x84, 0x00,	//11暗黄
		0x00, 0x00, 0x84,	//12暗青
		0x84, 0x00, 0x84,	//13暗紫
		0x00, 0x84, 0x84,	//14浅暗蓝
		0x84, 0x84, 0x84	//15暗灰
		
	};
	set_palette(0,15,table_rgb);
	return;
}

void set_palette(int start,int end,unsigned char *rgb)
{
	int i,eflags;
	eflags=io_load_eflags();//记录中断许可标志的值
	io_cli();               //将中断许可标志置为0，禁止中断
	io_out8(0x03c8,start);	//将要设定的调色板号码写入端口0x03c8，由于只有一个调色板，因此编号默认为0
	
	for(i=start;i<=end;i++)//按rgb的顺序写入0x03c9
	{
		io_out8(0x03c9,rgb[0]/4);
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);
		rgb+=3;
	}
	io_store_eflags(eflags);//恢复许可标志
	return;	
}

void boxfill8(unsigned char *vramBeginAddr, int xsize, unsigned char color, 
				int xBegin, int yBegin, int xEnd, int yEnd)
{
	int x,y;
	for(x=xBegin;x<=xEnd;x++)
	{
		for(y=yBegin;y<=yEnd;y++)
			vramBeginAddr[y*xsize+x]=color;
	}
}

void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x,	COL8_008484,	0,	0,			x-1,  y-19);
	boxfill8(vram, x,	COL8_C6C6C6,	0,	y - 18,	x -1, y - 1);

	boxfill8(vram, x, COL8_FFFFFF,  3,         y - 14, 40,         y - 14);
	boxfill8(vram, x, COL8_FFFFFF,  2,         y - 14,  2,         y -  4);
	boxfill8(vram, x, COL8_848484,  3,         y -  4, 40,         y -  4);
	boxfill8(vram, x, COL8_848484, 40,         y - 13, 40,         y -  5);
	boxfill8(vram, x, COL8_000000,  2,         y -  3, 40,         y -  3);
	boxfill8(vram, x, COL8_000000, 41,         y - 14, 41,         y -  3);

	return;
}

void putfont8(char *vram, int xsize, int x, int y, char color, char *font)
{
	int i;
	char *p, d ;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = color; }
		if ((d & 0x40) != 0) { p[1] = color; }
		if ((d & 0x20) != 0) { p[2] = color; }
		if ((d & 0x10) != 0) { p[3] = color; }
		if ((d & 0x08) != 0) { p[4] = color; }
		if ((d & 0x04) != 0) { p[5] = color; }
		if ((d & 0x02) != 0) { p[6] = color; }
		if ((d & 0x01) != 0) { p[7] = color; }
	}
	return;
}


void putString(char *vram,int xsize,int x,int y,char color,unsigned char *s)
{
	extern char hankaku[4096];
	for(;*s!=0x00;s++)
	{
		putfont8(vram,xsize,x,y,color,hankaku+*s*16);
		x+=8;
	}
}


void init_mouse(char *mouse, char backcolor)
/* 绘制鼠标 */
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = backcolor;
			}
		}
	}
	return;
}

//显示背景色的函数
void putblock8_8(char *vram,int vxsize,int pxsize,int pysize,int px0,int py0,char *buf,int bxsize)
{
	int x,y;
	for(y=0;y<pysize;y++)
		for(x=0;x<pxsize;x++)
		{
			vram[(py0+y)*vxsize+(px0+x)]=buf[y*bxsize+x];
		}
}
