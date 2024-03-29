
#define DT_DIR 4
#define OPENDIR_MAX 20

/* JML FIXME 32 / 64 bit */
struct dirent {
	int d_ino;
	int d_off;
	char d_type;
	char pad1[3];
	char d_name[256]; 
};

struct DIR_ {
	int fd;
	char buf[1024];
	int count;
	int bpos;
	int nread;
};

#define DIR  struct DIR_

DIR *opendir(char *name);
int closedir(DIR *dp);
struct dirent *readdir(DIR *dp);

