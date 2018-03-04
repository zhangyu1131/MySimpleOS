#include"bootpack.h"

unsigned int memtest(unsigned int start,unsigned int end)
{
	char flg486=0;
	unsigned int eflag,cr0,i;
	/*ȷ��CPU��386����486*/
	eflag=io_load_eflags();
	eflag|=EFLAGS_AC_BIT;//�����486���ϣ���EFLAGS��18λ��AC��־λ������386���������������һֱ��0.
	io_store_eflags(eflag);
	io_load_eflags();
	if((eflag & EFLAGS_AC_BIT)!=0)//�����386����ʹ���������AC��Ϊ1��AC��ֵ���ǻ��Զ��ص�0
	{
		flg486=1;
	}
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	
	if(flg486!=0)
	{
		cr0=load_cr0();
		cr0|=CR0_CATCH_DISABLE;//�رջ���
		store_cr0(cr0);
	}
	
	i=memtest_sub(start,end);
	
	if(flg486!=0)
	{
		cr0=load_cr0();
		cr0 &= ~CR0_CATCH_DISABLE;//��������
		store_cr0(cr0);
	}
	
	return i;
}


void memman_init(struct MEMMAN *man)
{
	man->frees=0;//������Ϣ��Ŀ
	man->maxfrees=0;//���ڹ۲����״����frees�����ֵ
	man->lostsize=0;//�ͷ�ʧ�ܵ��ڴ��С�ܺ�
	man->lostcount=0;//�ͷ�ʧ�ܴ���
}

//��������ڴ��С
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i,t=0;
	for(i=0;i<man->frees;i++)
	{
		t+=man->free[i].size;
	}
	return t;
}

//�����ڴ�
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size)
{
	unsigned int i,a;
	for(i=0;i<man->frees;i++)
	{
		//�ҵ��㹻�ڴ�
		if(man->free[i].size>=size)
		{
			a=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			//������ڴ����������Ϊ0
			if(man->free[i].size==0)
			{
				man->frees--;
				for(;i<man->frees;i++)
				{
					man->free[i]=man->free[i+1];
				}
			}
			return a;
		}
	}
	return 0;//�޿��ÿռ�
}

//�ͷ��ڴ�
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i,j;
	
	//�ҵ�Ҫ�ͷŵ��ڴ��λ�ڵ�λ��
	for(i=0;i<man->frees;i++)
	{
		if(man->free[i].addr>addr)
			break;
	}
	
	if(i>0)
	{
		//���Ժ�ǰ��Ŀ����ڴ�ϲ�
		if(man->free[i-1].addr+man->free[i-1].size==addr)
		{
			man->free[i-1].size+=size;
			if(i<man->frees)
			{
				//���Ժͺ���ĺϲ�
				if(addr+size==man->free[i].addr)
				{
					man->free[i-1].size+=man->free[i].size;
					man->frees--;//���п���1
					for(;i<man->frees;i++)
					{
						man->free[i]=man->free[i+1];
					}
					
				}
			}
			return 0;
		}
		
	}
	//������ǰ��ĺϲ�
	if(i<man->frees)
	{
		//���Ժͺ���ĺϲ�
		if(addr+size==man->free[i].addr)
		{
			man->free[i].addr=addr;
			man->free[i].size+=size;
			return 0;
		}
	}
	
	//�����ܺϲ�
	if(man->frees<MEMMAN_FREES)
	{
		for(j=man->frees;j>i;j--)
		{
			man->free[j]=man->free[j-1];
		}
		man->frees++;
		if(man->maxfrees<man->frees)
			man->maxfrees=man->frees;
		man->free[i].addr=addr;
		man->free[i].size=size;
		return 0;
	}
	
	//ʧ��
	man->lostcount++;
	man->lostsize+=size;
	return -1;
	
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size=(size+0xfff)&0xfffff000;//��0x1000Ϊ��λ��������
	a=memman_alloc(man,size);
	return a;
}

int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size)
{
	int i;
	size=(size+0xfff)&0xfffff000;
	i=memman_free(man,addr,size);
	return i;
}