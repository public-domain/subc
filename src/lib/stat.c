
#include <sys/stat.h>

int stat(char *path, struct stat *buf)
{
	int r;
	r =  _stat(path, buf);
	if (r == 0) {
		/* FIXME */
		if (sizeof(int) == 4) {
			buf->st_size = buf->garbage[20] +
				(buf->garbage[21] << 8) +
				(buf->garbage[22] << 16) +
				(buf->garbage[23] << 24);
			buf->st_mode = buf->garbage[8] +
				(buf->garbage[9] << 8);
		} else {
			buf->st_size = buf->garbage[48] +
				(buf->garbage[49] << 8) +
				(buf->garbage[50] << 16) +
				(buf->garbage[51] << 24);
			buf->st_mode = buf->garbage[24] +
				(buf->garbage[25] << 8) +
				(buf->garbage[26] << 16) +
				(buf->garbage[27] << 24);
		}
	}
	return r;
}
