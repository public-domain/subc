/*
 *	NMH's Simple C Compiler, 2011,2012
 *	stdio.h
 */

#define NULL	(void *)0
#define EOF	(-1)

#define FOPEN_MAX	20
#define BUFSIZ		512
#define FILENAME_MAX	128
#define TMP_MAX		1
#define L_tmpnam	8

#define _IONBF	0
#define _IOLBF	1
#define _IOFBF	2
#define _IOACC	3
#define _IOUSR	4

#define _FCLOSED	0
#define _FREAD		1
#define _FWRITE		2
#define _FERROR		4

#define FILE	int

extern int	 _file_fd[];
extern char	 _file_iom[];
extern char	 _file_last[];
extern char	 _file_mode[];
extern int	 _file_ptr[];
extern int	 _file_end[];
extern int	 _file_size[];
extern int	 _file_ch[];
extern char	*_file_buf[];

extern FILE	*stdin, *stdout, *stderr;

#define getc	fgetc
#define putc	fputc

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

void	clearerr(FILE *f);
int	fclose(FILE *f);
FILE	*fdopen(int fd, char *mode);
int	feof(FILE *f);
int	ferror(FILE *f);
int	fflush(FILE *f);
int	fgetc(FILE *f);
int	fgetpos(FILE *f, int *ppos);
char	*fgets(char *buf, int len, FILE *f);
int	fileno(FILE *f);
FILE	*fopen(char *path, char *mode);
int	fprintf(FILE *f, char *fmt, ...);
int	fputc(int c, FILE *f);
int	fputs(char *s, FILE *f);
int	fread(void *buf, int size, int count, FILE *f);
FILE	*freopen(char *path, char *mode, FILE *f);
int	fscanf(FILE *f, char *fmt, ...);
int	fseek(FILE *f, int off, int how);
int	fsetpos(FILE *f, int *ppos);
int	ftell(FILE *f);
int	fwrite(void *buf, int size, int count, FILE *f);
int	getc(FILE *f);
int	getchar(void);
char	*gets(char *buf);
void	perror(char *s);
int	printf(char *fmt, ...);
int	putc(int c, FILE *f);
int	putchar(int c);
int	puts(char *s);
int	remove(char *path);
int	rename(char *from, char *to);
void	rewind(FILE *f);
int	scanf(char *fmt, ...);
void	setbuf(FILE *f, char *buf);
int	setvbuf(FILE *f, char *buf, int mode, int size);
int	sscanf(char *s, char *fmt, ...);
int	sprintf(char *buf, char *fmt, ...);
char	*tmpnam(char *buf);
FILE	*tmpfile(void);
int	ungetc(int c, FILE *f);
