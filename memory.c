#include"bootpack.h"

unsigned int memtest(unsigned int start,unsigned int end)
{
	char flg486=0;
	unsigned int eflag,cr0,i;
	/*确认CPU是386还是486*/
	eflag=io_load_eflags();
	eflag|=EFLAGS_AC_BIT;//如果是486以上，则EFLAGS第18位是AC标志位，若是386，则无论如何设置一直是0.
	io_store_eflags(eflag);
	io_load_eflags();
	if((eflag & EFLAGS_AC_BIT)!=0)//如果是386，即使上述步骤把AC置为1，AC的值还是会自动回到0
	{
		flg486=1;
	}
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	
	if(flg486!=0)
	{
		cr0=load_cr0();
		cr0|=CR0_CATCH_DISABLE;//关闭缓存
		store_cr0(cr0);
	}
	
	i=memtest_sub(start,end);
	
	if(flg486!=0)
	{
		cr0=load_cr0();
		cr0 &= ~CR0_CATCH_DISABLE;//开启缓存
		store_cr0(cr0);
	}
	
	return i;
}


void memman_init(struct MEMMAN *man)
{
	man->frees=0;//可用信息数目
	man->maxfrees=0;//用于观察可用状况，frees的最大值
	man->lostsize=0;//释放失败的内存大小总和
	man->lostcount=0;//释放失败次数
}

//计算空闲内存大小
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i,t=0;
	for(i=0;i<man->frees;i++)
	{
		t+=man->free[i].size;
	}
	return t;
}

//分配内存
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size)
{
	unsigned int i,a;
	for(i=0;i<man->frees;i++)
	{
		//找到足够内存
		if(man->free[i].size>=size)
		{
			a=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			//如果该内存块分配后容量为0
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
	return 0;//无可用空间
}

//释放内存
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i,j;
	
	//找到要释放的内存块位于的位置
	for(i=0;i<man->frees;i++)
	{
		if(man->free[i].addr>addr)
			break;
	}
	
	if(i>0)
	{
		//可以和前面的空闲内存合并
		if(man->free[i-1].addr+man->free[i-1].size==addr)
		{
			man->free[i-1].size+=size;
			if(i<man->frees)
			{
				//可以和后面的合并
				if(addr+size==man->free[i].addr)
				{
					man->free[i-1].size+=man->free[i].size;
					man->frees--;//空闲块少1
					for(;i<man->frees;i++)
					{
						man->free[i]=man->free[i+1];
					}
					
				}
			}
			return 0;
		}
		
	}
	//不能与前面的合并
	if(i<man->frees)
	{
		//可以和后面的合并
		if(addr+size==man->free[i].addr)
		{
			man->free[i].addr=addr;
			man->free[i].size+=size;
			return 0;
		}
	}
	
	//都不能合并
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
	
	//失败
	man->lostcount++;
	man->lostsize+=size;
	return -1;
	
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size=(size+0xfff)&0xfffff000;//以0x1000为单位向下舍入
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