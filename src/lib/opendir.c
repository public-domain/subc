
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

extern DIR     *_dirs[]; 

DIR *opendir(char *name) {
	DIR *p;
	int i;
	for (i = 0; i < OPENDIR_MAX; i++) {
		if (_dirs[i] == NULL) break;
	}
	if (i >= OPENDIR_MAX) {
		errno = ENFILE;
		return NULL;
	}
	p = malloc(sizeof(DIR));
	if (p == NULL) return NULL;
	_dirs[i] = p;
	p->fd = _open(name, O_RDONLY | O_DIRECTORY);
	if (p->fd < 0) {
		free(p);
		return NULL;
	}
	p->count = 1024;
	p->bpos = 0;
	p->nread = 0;
	return p;
}

int closedir(DIR *dirp) {
	int i;
	for (i = 0; i < OPENDIR_MAX; i++) {
		if (_dirs[i] == dirp) break;
	}
	if (i >= OPENDIR_MAX) {
		return -1;
	}
	free(dirp);
	_dirs[i] = NULL;
	return 0;
}

/* FIXME find a solution for unaligned struct members... */
static fix_name(char *n)
{
	char *d;
       	d = n - sizeof(int) + 2;
	memmove(n, d, strlen(d) + 1);
}

struct dirent *readdir(DIR *dirp) {

	int r;
	struct dirent *d;
	char *p;
	char *reclen;
	int rl;
	if (dirp->bpos < dirp->nread) {
		p = dirp->buf + dirp->bpos;
		d = (struct dirent *) p;
		reclen = &(d->d_type);
		rl = reclen[0] + (reclen[1] << 8);
		dirp->bpos += rl;
		d->d_type = p[rl-1];
		fix_name(d->d_name);
		return d;
	}
	r = _getdents(dirp->fd, dirp->buf, dirp->count);
	if (r > 0) {
		dirp->nread = r;
		p = dirp->buf;
		d = (struct dirent *) p;
		reclen = &(d->d_type);
		rl = reclen[0] + (reclen[1] << 8);
		dirp->bpos += rl;
		d->d_type = p[rl-1];
		fix_name(d->d_name);
		return d;
	} else if (r < 0) {
		errno = EINVAL;
	}
	
	return NULL;
}

