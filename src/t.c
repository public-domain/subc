
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

typedef unsigned int size_tt;

int mix(size_tt *ii, char *q, char *f, ...)
{
	/*register*/ char *u = q;
	char *p;
	char *p1;
	printf("MIXXXX\n");
	p = p1 + *ii;
	return 0;
}

int mix2() {
	printf("MIX11111111\n");
	return 1;
}

#define mix3(q) mix2a(q); mix2b(q); mix2c(q)
#define mix1(a) mix3("bos"); mix3(a) ; mix3("yo")
#define mix(a,b) mix1(a)
#define mit(a) a

void sdmi(char *a, char *b, char *c)
{
	char *d = a;
	char *e = b;
	char *f = c;
	printf("SDMI %s %s %s\n", d, e, f);
}

int main()
{
	int i;
	char *p = "Tera miy\n";
	char *p2 = "jhnkjhjk\n";
	mit(const char *) p3 = "jml";
	sdmi("one", "two", "three");
	printf("char %d\n%s", sizeof(s), p, p2);
	printf("char + int %d\n", sizeof(s1));
	printf("de %d %d\n", &de[1] , ptr);
	//mix(0, 1);
	//mix1(0);
	*(&(p2)) = 1, mix2();
	i = (size_tt)( (int*)p - 0) + 1;
	if (p > de || 1){
	 	int z = 448;
	  	printf("z %d\n", z);	
	}
	if (1) {
		int y = 987;
	  	printf("y %d\n", y);	
	}
	//printf("%s\n", p2);
	//
	return 0;
}
