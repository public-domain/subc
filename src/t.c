
#include <stdio.h>

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
	char c1;
	int i;
	s st;
	u un;
} s1;

int main()
{
	char *p = "Tera miy\n";
	char *p2 = "jhnkjhjk\n";
	printf("char %d\n%s", sizeof(s), p, p2);
	printf("char + int %d\n", sizeof(s1));
	return 0;
}
