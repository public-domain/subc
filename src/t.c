#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>

int main()
{
	printf("%d\n", sizeof(struct stat));
	printf("%d\n", offsetof(struct stat, st_size));
	printf("%d %d\n", offsetof(struct stat, st_mode), sizeof(mode_t));
	return 0;
}
