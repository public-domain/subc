
//#include <stdio.h>
#include "include/stdio.h"

int main()
{
	static int buf1 = 9;
	char buf[] = "jml";
	printf("%d %s\n", buf1, buf);
	return 0;
}
