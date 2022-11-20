
#include <sys/stat.h>

int mkdir(char *pathname, int mode)
{
	return _mkdir(pathname, mode);
}
