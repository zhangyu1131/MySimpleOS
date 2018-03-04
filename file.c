#include"bootpack.h"


//将磁盘映像中的fat解压缩
/*
ab cd ef  =>
dab efc
*/
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {//文件小于512字节，则存放在一个簇里
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		//大于512字节时，循环读入一簇数据
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];//找到下一簇
	}
	return;
}


struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	int findflag;
	char s[12];
	//初始化数组
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	
	//生成符合规范的要寻找的文件名
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) 
		{
			return 0;//没找到
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				//小写转换成大写
				s[j] -= 0x20;
			} 
			j++;
		}
	}
	
	//寻找
	for (i = 0; i < max;i++ ) {
		if (finfo[i].name[0] == 0x00) {//检查该段是否包含文件信息
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {//检查文件类型
			findflag=1;
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					findflag=0;
					break;
				}
			}
			if(findflag!=0)
				return finfo + i; //找到
		}
	}
	return 0; //没找到
}