
//#include <stdio.h>
#include "include/stdio.h"

typedef void* vp;
typedef vp* vpp;

#if 0

x'15'

#endif

int main()
{
	static int buf1 = 9;
	static char buf[] = "jml";
	printf("%d %s\n", buf1, &(*buf));
	return 0;
}
