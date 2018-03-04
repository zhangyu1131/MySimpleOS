#include"bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *)ADR_GDT;//�����ڴ�0x270000~0x27ffff
	struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR *)ADR_IDT;//0x26f800~0x26ffff
	int i;
	
	/*GDT��ʼ��*/
	for(i=0;i<=LIMIT_GDT / 8;i++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	//�Զκ�Ϊ1�Ķν����趨������4g����ַ0������Ϊ4092������ϵͳ���ݶ�
	set_segmdesc(gdt+1,0xffffffff,0x00000000,AR_DATA32_RW);
	set_segmdesc(gdt+2,LIMIT_BOTPAK,ADR_BOTPAK,AR_CODE32_ER);//�Զκ�Ϊ2�Ķν����趨������512KB,��ַ280000������409a
															//Ϊbootpack.hrb׼���ģ�����ϵͳ�ô����
	
	load_gdtr(LIMIT_GDT,ADR_GDT);//��gdt��ֵ��Ҫ�û��
	
	/*IDT��ʼ��*/
	for(i=0;i <= LIMIT_IDT / 8;i++)
	{
		set_gatedesc(idt+i,0,0,0);
	}
	load_idtr(LIMIT_IDT,ADR_IDT);
	
	/* ����IDT*/
	set_gatedesc(idt + 0x0c, (int) asm_inthandler0c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x20, (int) asm_inthandler20,  2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x40, (int) asm_hrb_api	,  2 * 8, AR_INTGATE32+0x60);//+0x60��ʾ���жϿɹ�Ӧ�ó�����Ϊapi�����ã������޷����ø��ж�
	
	return;
}


//����cpu�Ĺ��Ҫ�󣬽�����Ϣ����8���ֽ�д���ڴ档
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if(limit>0xfffff)//���޴���1mb����Ҫ�ı�gλ������Ҫ��������
	{
		ar|=0x8000; //G_bit=1
		limit/=0x1000;
	}
	sd->limit_low=limit&0xffff;
	sd->base_low=base&0xffff;
	sd->base_mid=(base>>16)&0xff;
	sd->access_right=ar&0xff;
	sd->limit_high=((limit>>16)&0x0f)|((ar>>8)&0xf0);
	sd->base_high=(base>>24)&0xff;
	return;
	
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd,int offset,int selector,int ar)
{
	gd->offset_low=offset&0xffff;
	gd->selector=selector;
	gd->dw_count=(ar>>8)&0xff;
	gd->access_right=ar&0xff;
	gd->offset_high=(offset>>16)&0xffff;
	return;
}