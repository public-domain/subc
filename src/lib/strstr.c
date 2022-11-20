/*
 *	Simple C Compiler, 2022 
 *	strchr()
 */

#include <stdio.h>
#include <string.h>

char *strstr(char *s, char *c) {
	char *p;
	char *r;
	p = c;
	r = s;
	while (*s && *p) {
		if (*s == *p) {
			p++;
		} else {
			r = s + 1;
			p = c;
		}
		s++;
	}
	if (*p == '\0') {
	       return r;
        }	       
	return NULL;
}
