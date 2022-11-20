/*
 *	NMH's Simple C Compiler, 2011--2021
 *	malloc()
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define THRESHOLD	1024
#define OVERALLOC	32

int		*_arena = 0;
int		_asize;
static int	*freep;

static void defrag(void) {
	int	*p, *q, *end;

	end = _arena + _asize;
	for (p = _arena; p < end; p += ((*p < 0) ? -*p : *p)) {
		if (*p > 0) {
			for (q = p + *p; q < end && *q > 0; q += *q)
				;
			*p = q - p;
		}
	}
}

void *malloc(int size) {
	int	*p, *end;
	int	k, n, tries;

	size = (size + sizeof(int) - 1) / sizeof(int);
	if (NULL == _arena) {
		if (size >= THRESHOLD)
			_asize = size + 1;
		else
			_asize = size * OVERALLOC;
		_arena = _sbrk(_asize * sizeof(int));
		if ((int *) -1 == _arena) {
			errno = ENOMEM;
			return NULL;
		}
		_arena[0] = _asize;
		freep = _arena;
	}
	for (tries = 0; tries < 3; tries++) {
		end = _arena + _asize;
		p = freep;
		do {
			if (*p > size) {
				if (size + 1 == *p) {
					*p = -*p;
				}
				else {
					k = *p;
					*p = -(size+1);
					p[size+1] = k - size - 1;
				}
				freep = p;
				return p+1;
			}
			if (*p < 0) {
				p -= *p;
			} else {
				p += *p;
			}
			if (p == end) p = _arena;
			if (p < _arena || p >= end || 0 == *p) {
				_write(2, "malloc(): corrupt arena\n", 24);
				abort();
			}
		} while (p != freep);
		if (0 == tries) {
			defrag();
		}
		else {
			n = _asize; /* double the amout of memory */
			while (n <= size) {
				n += _asize;
			}
			if (_sbrk(n * sizeof(int)) == (void *)-1) {
				errno = ENOMEM;
				_write(2, "malloc(): _sbrk failed\n", 23);
				return NULL;
			}
			k = _asize;
			_asize += n;
			*end = _asize - k;
		}
	}
	errno = ENOMEM;
	_write(2, "malloc(): no memory\n", 20);
	return NULL;
}
