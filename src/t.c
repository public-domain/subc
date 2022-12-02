
//#include <stdio.h>
#include "include/stdio.h"

typedef void* vp;
typedef vp* vpp;

int main()
{
	static int buf1 = 9;
	char buf[] = "jml";
	printf("%d %s\n", buf1, (vpp*)buf);
	return 0;
}
