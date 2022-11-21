
typedef struct s {
	char s;
} s;

typedef union u {
	char a;
	char b[1];
} u;

typedef struct s1 {
	char c;
	char c1;
	int i;
	s st;
	u un;
} s1;

int main()
{
	printf("char %d\n", sizeof(s));
	printf("char + int %d\n", sizeof(s1));
	return 0;
}
