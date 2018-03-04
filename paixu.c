void api_putstr(char *s);
void api_end();

void HariMain()
{	
	char array[6]={'3','1','2','a','c','b'};
	char temp;
	int i,j;
	for(i=0;i<6;i++)
	{
		for(j=0;j<5-i;j++)
		{
			if(array[j]>array[j+1])
			{
				temp=array[j];
				array[j]=array[j+1];
				array[j+1]=temp;
			}
		}
	}
	api_putstr(array);
	api_end();
}