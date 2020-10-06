#include <stdio.h>

void SWDLCbf(unsigned char* payload)
{
	printf("%d\n",payload[1]);
}

int main()
{
	struct temp
	{
		unsigned char a;
		unsigned char b;
	}t;
	t.a = 2;
	t.b = 4;
	unsigned char *p = (unsigned char*)&t;
	SWDLCbf(p);
	return 0;
}
