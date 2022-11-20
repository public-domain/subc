
#define S_IRWXU (0x100|0x80|0x40) 
#define S_IRWXG ((0x100|0x80|0x40)>>3)
#define S_IRWXO ((0x100|0x80|0x40)>>6)
#define S_IRUSR 0x100
#define S_IWUSR 0x80
#define S_IXUSR 0x40
#define S_IRGRP (0x100 >> 3)
#define S_IWGRP (0x80 >> 3)
#define S_IXGRP (0x40 >>3)
#define S_IROTH (0x100 >> 6)
#define S_IWOTH (0x80 >> 6)
#define S_IXOTH (0x40 >> 6)
#define S_IFDIR 0x4000

struct stat {
	char garbage[144];
	int st_mode;
	int st_size;
};

int mkdir(char *pathname, int mode);
int stat(char *pathname, struct stat *statbuf);

