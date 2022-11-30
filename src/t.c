
//#include <stdio.h>
#include "include/stdio.h"


static char b1[] = "HELLO WORLD";
void trr(int k)
{
	//char *buf = "jml";
	static char buf[] = "hello world";
	printf("%s %s\n", buf, b1);
}

int main()
{
	static int buf1 = 9;
	static char buf[100];
	//static char *buf = "mix";
	printf("%d %s", buf1, buf);
	buf1 = 11;
	trr(2);
	return 0;
}
