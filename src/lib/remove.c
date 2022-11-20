/*
 *	NMH's Simple C Compiler, 2011,2012
 *	remove()
 */

#include <unistd.h>

int remove(char *path) {
	int r;
	r = _unlink(path);
	if (r != 0) {
		r = _rmdir(path);
	}
	return r;
}

int rmdir(char *path) {
	return _rmdir(path);
}
