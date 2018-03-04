//无限循环
void api_putchar(int c);
void HariMain()
{
	for(;;){
		api_putchar('a');
		api_putchar('b');
		api_putchar('c');
		api_putchar('d');
	}
}