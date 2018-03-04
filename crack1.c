//应用程序试图修改操作系统的内存
void api_end();

void HariMain()
{
	*((char *)0x00102600)=0;
	api_end();
}