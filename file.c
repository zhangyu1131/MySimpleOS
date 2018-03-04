#include"bootpack.h"


//������ӳ���е�fat��ѹ��
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
		if (size <= 512) {//�ļ�С��512�ֽڣ�������һ������
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		//����512�ֽ�ʱ��ѭ������һ������
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];//�ҵ���һ��
	}
	return;
}


struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	int findflag;
	char s[12];
	//��ʼ������
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	
	//���ɷ��Ϲ淶��ҪѰ�ҵ��ļ���
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) 
		{
			return 0;//û�ҵ�
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				//Сдת���ɴ�д
				s[j] -= 0x20;
			} 
			j++;
		}
	}
	
	//Ѱ��
	for (i = 0; i < max;i++ ) {
		if (finfo[i].name[0] == 0x00) {//���ö��Ƿ�����ļ���Ϣ
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {//����ļ�����
			findflag=1;
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					findflag=0;
					break;
				}
			}
			if(findflag!=0)
				return finfo + i; //�ҵ�
		}
	}
	return 0; //û�ҵ�
}