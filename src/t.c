
//#include <stdio.h>
#include "include/stdio.h"

//#if defined __SCC__ \
//	&& defined(__gnu_linux__)
//#define YO
#if 5 == 5 || 7 > 8
static int ioii = 9;
#endif


typedef struct s {
	char s;
} s;

typedef union u {
	char a;
	char b[1];
} u;

typedef struct s1 {
	char c;
	unsigned long l;
	char c1;
	int i;
	s st;
	u un;
} s1;

extern s1 *a, *b, *c;

s1 de1;
s1 de[3];
s1 *ptr = &de[1];

int mix(int *ii, long f, ...)
{
	printf("MIXXXX\n");
	return 0;
}

int mix1() {
	printf("MIX11111111\n");
	return 1;
}

#define mix(a,b) mix1()


int main()
{
	char *p = "Tera miy\n";
	char *p2 = "jhnkjhjk\n";
	printf("char %d\n%s", sizeof(s), p, p2);
	printf("char + int %d\n", sizeof(s1));
	printf("de %d %d\n", &de[1] , ptr);
//	mix(NULL, 1);
	return 0;
}
