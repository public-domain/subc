/*
 *	NMH's Simple C Compiler, 2011--2021
 *	Lexical analysis (scanner)
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

int next(void) {
	int	c;

	if (Putback) {
		c = Putback;
		Putback = 0;
		return c;
	}
	if (Mp) {
		if ('\0' == *Macp[Mp-1]) {
			Macp[Mp-1] = NULL;
			return Macc[--Mp];
		}
		else {
			return *Macp[Mp-1]++;
		}
	}
	c = fgetc(Infile);
	if ('\n' == c) Line++;
	return c;
}

void putback(int c) {
	Putback = c;
}

static int hexchar(void) {
	int	c, h, n = 0, f = 0;

	while (isxdigit(c = next())) {
		h = chrpos("0123456789abcdef", tolower(c));
		n = n * 16 + h;
		f = 1;
	}
	if (!f)
		error("missing digits after '\\x'", NULL);
	if (n > 255)
		error("value out of range after '\\x'", NULL);
	putback(c);
	return n;
}

static int scanch(void) {
	int	i, c, c2;

	c = next();
	if ('\\' == c) {
		switch (c = next()) {
		case 'a': return '\a';
		case 'b': return '\b';
		case 'f': return '\f';
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'v': return '\v';
		case '\\': return '\\';
		case '"': return '"' | 256;
		case '\'': return '\'';
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7':
			for (i = c2 = 0; isdigit(c) && c < '8'; c = next()) {
				if (++i > 3) break;
				c2 = c2 * 8 + (c - '0');
			}
			putback(c);
			return c2;
		case 'x':
			return hexchar();
		default:
			scnerror("unknown escape sequence: %s", c);
			return ' ';
		}
	}
	else {
		return c;
	}
}

/* FIXME parse float */
static int scanflt(int i, int c, int val) {
	int k, point, n;
	while ((k = chrpos("0123456789", c)) >= 0) {
		Text[i++] = c;
		val = val * 10 + k;
		c = next();
	}
	n = 1;
	point = 0;
	if ('.' == c) {
		Text[i++] = c;
		c = next();
	}
	while ((k = chrpos("0123456789", c)) >= 0) {
		Text[i++] = c;
		point = point * 10 + k;
		n *= 10;
		c = next();
	}
	putback(c);
	Text[i] = 0;
	return val;
}

static int scanint(int c, int *r) {
	int	val, radix, k, i = 0;

	*r = INTLIT;
	val = 0;
	radix = 10;
	if ('0' == c) {
		Text[i++] = '0';
		if ((c = next()) == 'x') {
			radix = 16;
			Text[i++] = c;
			c = next();
		}
		else {
			radix = 8;
		}
	}
	if ('.' == c) {
		*r = FLOATLIT;
		return scanflt(i, c, 0);
	}
	while ((k = chrpos("0123456789abcdef", tolower(c))) >= 0) {
		Text[i++] = c;
		if (k >= radix)
			scnerror("invalid digit in integer literal: %s", c);
		val = val * radix + k;
		c = next();
	}
	if ('.' == c) {
		*r = FLOATLIT;
		return scanflt(i, c, val);
	}
	if (c != 'L') {
		putback(c);
	}
	Text[i] = 0;
	return val;
}

static int scanstr(char *buf) {
	int	i, c;

	buf[0] = '"';
	for (i=1; i<TEXTLEN-2; i++) {
		if ((c = scanch()) == '"') {
			buf[i++] = '"';
			buf[i] = 0;
			return Value = i;
		}
		buf[i] = c;
	}
	fatal("string literal too long");
	return 0;
}

static int scanident(int c, char *buf, int lim) {
	int	i = 0;

	while (isalpha(c) || isdigit(c) || '_' == c) {
		if (lim-1 == i) {
			error("identifier too long", NULL);
			i++;
		}
		else if (i < lim-1) {
			buf[i++] = c;
		}
		c = next();
	}
	putback(c);
	buf[i] = 0;
	return i;
}

static int skipundef(int c)
{
	if (frozen(1) && Expandmac) {
		while (EOF != c && '\n' != c && '#' != c && '\r' != c
				&& '*' != c && '/' != c && '\\' != c) 
		{
			c = next();
		}
	}
	return c;
}

static int skipspace(int *cl, int *nl, int *skipnl)
{
	int c, r;
	c = *cl;
	r = 0;
	for (;;) {
		//fprintf(stderr, "JML %c ...\n", c);
		while (' ' == c || '\t' == c || '\n' == c ||
			'\r' == c || '\f' == c
		) {
			if ('\n' == c) {
				if (*skipnl) {
					*skipnl = 0;
				} else {
					if (!Expandmac) {
						*cl = ' ';
						return  1;	
					}
				}
				*nl = 1;
			}
			c = next();
			r = 1;
		}
		// JML
		if ('\\' == c) {
			c = next();
			if (c == '\n' || c == '\r') { 
				*skipnl = 1;
				continue;
			} else {
				putback(c);
				c = '\\';
			}
		}
		break;
	}
	*cl = c;
	return r;
}

static int skipcom(int *cl, int *nl)
{
	int p, c, r;
	c = *cl;
	r = 0;
	while (' ' == c || '\t' == c || '\n' == c ||
			'\r' == c || '\f' == c
		) 
	{
		if ('\n' == c) {
			*nl = 1;
			if (!Expandmac) {
				//fprintf(stderr, "JML EOOL \n");
				*cl = ' ';
				return  1;	
			}
		}
		c = next();
		r = 1;
	}
	
	while (c == '/') {
		c = next();
		if (c != '*' && c != '/') {
			putback(c);
			c = '/';
			break;
		}
		r = 1;
		if (c == '/') {
			while ((c = next()) != EOF) {
				if (c == '\n') {
					*nl = 1;
					c = next();
					break;
				}
			}
                }
                else {
			p = 0;
			while ((c = next()) != EOF) {
				if ('/' == c && '*' == p) {
					c = next();
					break;
				}
				p = c;
			}
		}
	}
	*cl = c;
	return r;
}


int skip(void) {
	int	c, p, skipnl;

	c = next();
	skipnl = 0;
	for (;;) {
		if (EOF == c) {
			strcpy(Text, "<EOF>");
			return EOF;
		}
		//fprintf(stderr, "JML '%c'\n", c);
	
		while (skipcom(&c, &Newl)) {
			if (' ' == c && Newl && !Expandmac) {
				Newl = 0;
				return c;
			}
			if ('\\' == c) {
				c = next();
				if ('\r' == c) {
					c = next();
				}
				if (c == '\n') {
					c = next();	
				} else {
					putback(c);
					c = '\\';
				}
			}
		}
		//fprintf(stderr, "JML PRiiE '%c' %d FR %d\n", c, nl, frozen(1));
		if (Newl && c == '#') {
			preproc();
			Newl = 1;
			c = next();
		//fprintf(stderr, "JML PRE '%c' FR %d\n", c, frozen(1));
			continue;
		} else if (Newl && c == ' ') {
			return c;
		} else {
			Newl = 0;
		}
		if (frozen(1) && Expandmac) {
			c = next();
			continue;
		}
		break;
	};
	return c;
}

static int keyword(char *s) {
	switch (*s) {
	case '#':
		switch (s[1]) {
		case 'd':
			if (!strcmp(s, "#define")) return P_DEFINE;
			break;
		case 'e':
			if (!strcmp(s, "#else")) return P_ELSE;
			if (!strcmp(s, "#elif")) return P_ELIF;
			if (!strcmp(s, "#endif")) return P_ENDIF;
			if (!strcmp(s, "#error")) return P_ERROR;
			break;
		case 'i':
			if (!strcmp(s, "#ifdef")) return P_IFDEF;
			if (!strcmp(s, "#if")) return P_IF;
			if (!strcmp(s, "#ifndef")) return P_IFNDEF;
			if (!strcmp(s, "#include")) return P_INCLUDE;
			break;
		case 'l':
			if (!strcmp(s, "#line")) return P_LINE;
			break;
		case 'p':
			if (!strcmp(s, "#pragma")) return P_PRAGMA;
			break;
		case 'u':
			if (!strcmp(s, "#undef")) return P_UNDEF;
			break;
		}
		break;
	case 'a':
		if (!strcmp(s, "auto")) return AUTO;
		break;
	case 'b':
		if (!strcmp(s, "break")) return BREAK;
		break;
	case 'c':
		if (!strcmp(s, "case")) return CASE;
		if (!strcmp(s, "char")) return CHAR;
		if (!strcmp(s, "const")) return CONST;
		if (!strcmp(s, "continue")) return CONTINUE;
		break;
	case 'd':
		if (!strcmp(s, "default")) return DEFAULT;
		if (!strcmp(s, "do")) return DO;
		if (!strcmp(s, "double")) return DOUBLE;
		break;
	case 'e':
		if (!strcmp(s, "else")) return ELSE;
		if (!strcmp(s, "enum")) return ENUM;
		if (!strcmp(s, "extern")) return EXTERN;
		break;
	case 'f':
		if (!strcmp(s, "float")) return FLOAT;
		if (!strcmp(s, "for")) return FOR;
		break;
	case 'i':
		if (!strcmp(s, "if")) return IF;
		if (!strcmp(s, "int")) return INT;
		break;
	case 'l':
		if (!strcmp(s, "long")) return LONG;
		break;
	case 'r':
		if (!strcmp(s, "register")) return REGISTER;
		if (!strcmp(s, "return")) return RETURN;
		break;
	case 's':
		if (!strcmp(s, "short")) return SHORT;
		if (!strcmp(s, "signed")) return SIGNED;
		if (!strcmp(s, "sizeof")) return SIZEOF;
		if (!strcmp(s, "static")) return STATIC;
		if (!strcmp(s, "struct")) return STRUCT;
		if (!strcmp(s, "switch")) return SWITCH;
		break;
	case 't':
		if (!strcmp(s, "typedef")) return TYPEDEF;
		break;
	case 'u':
		if (!strcmp(s, "union")) return UNION;
		if (!strcmp(s, "unsigned")) return UNSIGNED;
		break;
	case 'v':
		if (!strcmp(s, "void")) return VOID;
		if (!strcmp(s, "volatile")) return VOLATILE;
		break;
	case 'w':
		if (!strcmp(s, "while")) return WHILE;
		break;
	}
	return 0;
}

static int macro(char *name) {
	int	y;

	y = findmac(name);
	if (!y || Mtext[y] == NULL)
		return 0;
	playmac(y);
	return 1;
}

int scanpproc(void)
{
	int c, t, x;
	Value = 0;
	c = next();
	memset(Text, 0, 4);
	Text[0] = c;
	switch (c) {
	case '#':
		Text[0] = '#';
		x = Expandmac;
		Expandmac = 0;
		scanident(next(), &Text[1], TEXTLEN-1);
		Expandmac = x;
		if ((t = keyword(Text)) != 0)
			return t;
		error("unknown preprocessor command: %s", Text);
		return IDENT;
	default:
		error("junk in preprocessor command: %s", Text);
		return IDENT;
	}
}

static int scanpp(void) {
	int	c, t, y, r;

	if (Rejected != -1) {
		t = Rejected;
		Rejected = -1;
		strcpy(Text, Rejtext);
		Value = Rejval;
		return t;
	}
	for (;;) {
		Value = 0;
		c = skip();
		memset(Text, 0, 4);
		Text[0] = c;
		switch (c) {
		case '!':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return NOTEQ;
			}
			else {
				putback(c);
				return XMARK;
			}
		case '%':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return ASMOD;
			}
			else {
				putback(c);
				return MOD;
			}
		case '&':
			if ((c = next()) == '&') {
				Text[1] = '&';
				return LOGAND;
			}
			else if ('=' == c) {
				Text[1] = '=';
				return ASAND;
			}
			else {
				putback(c);
				return AMPER;
			}
		case '(':
			return LPAREN;
		case ')':
			return RPAREN;
		case '*':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return ASMUL;
			}
			else {
				putback(c);
				return STAR;
			}
		case '+':
			if ((c = next()) == '+') {
				Text[1] = '+';
				return INCR;
			}
			else if ('=' == c) {
				Text[1] = '=';
				return ASPLUS;
			}
			else {
				putback(c);
				return PLUS;
			}
		case ',':
			return COMMA;
		case '-':
			if ((c = next()) == '-') {
				Text[1] = '-';
				return DECR;
			}
			else if ('=' == c) {
				Text[1] = '=';
				return ASMINUS;
			}
			else if ('>' == c) {
				Text[1] = '>';
				return ARROW;
			}
			else {
				putback(c);
				return MINUS;
			}
		case '/':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return ASDIV;
			}
			else {
				putback(c);
				return SLASH;
			}
		case ':':
			return COLON;
		case ';':
			return SEMI;
		case '<':
			if ((c = next()) == '<') {
				Text[1] = '<';
				if ((c = next()) == '=') {
					Text[2] = '=';
					return ASLSHIFT;
				}
				else {
					putback(c);
					return LSHIFT;
				}
			}
			else if ('=' == c) {
				Text[1] = '=';
				return LTEQ;
			}
			else {
				putback(c);
				return LESS;
			}
		case '=':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return EQUAL;
			}
			else {
				putback(c);
				return ASSIGN;
			}
		case '>':
			if ((c = next()) == '>') {
				Text[1] = '>';
				if ((c = next()) == '=') {
					Text[1] = '=';
					return ASRSHIFT;
				}
				else {
					putback(c);
					return RSHIFT;
				}
			}
			else if ('=' == c) {
				Text[1] = '=';
				return GTEQ;
			}
			else {
				putback(c);
				return GREATER;
			}
		case '?':
			return QMARK;
		case '[':
			return LBRACK;
		case ']':
			return RBRACK;
		case '^':
			if ((c = next()) == '=') {
				Text[1] = '=';
				return ASXOR;
			}
			else {
				putback(c);
				return CARET;
			}
		case '{':
			return LBRACE;
		case '|':
			if ((c = next()) == '|') {
				Text[1] = '|';
				return LOGOR;
			}
			else if ('=' == c) {
				Text[1] = '=';
				return ASOR;
			}
			else {
				putback(c);
				return PIPE;
			}
		case '}':
			return RBRACE;
		case '~':
			return TILDE;
		case EOF:
			strcpy(Text, "<EOF>");
			return XEOF;
		case '\'':
			Text[1] = Value = scanch();
			if ((c = next()) != '\'') {
				error(
				 "expected '\\'' at end of char literal",
					NULL);
			}
			Text[2] = '\'';
			return INTLIT;
		case '"':
			Value = scanstr(Text);
			return STRLIT;
		case '#':
			Text[0] = '#';
			scanident(next(), &Text[1], TEXTLEN-1);
			if ((t = keyword(Text)) != 0)
				return t;
			error("unknown preprocessor command: %s", Text);
			return IDENT;
		case '.':
			c = next();
			if (c == '.') {
				Text[1] = Text[2] = '.';
				Text[3] = 0;
				if ((c = next()) == '.')
					return ELLIPSIS;
				putback(c);
				error("incomplete '...'", NULL);
				return ELLIPSIS;
			} else if (isdigit(c)) {
				putback(c);
				Value = scanflt(0, '.', 0);
				return FLOATLIT;
			}
			putback(c);
			return DOT;
		case ' ':
			return XEOL;
		default:
			if (isdigit(c)) {
				Value = scanint(c, &r);
				return r;
			}
			else if (isalpha(c) || '_' == c) {
				Value = scanident(c, Text, TEXTLEN);
				if (Expandmac && macro(Text))
					break;
				if ((t = keyword(Text)) != 0)
					return t;
				return IDENT;
			}
			else {
				scnerror("#ERR024 funny input character: %s", c);
				break;
			}
		}
	}
}

int scan(void) {
	int	t;

	do {
		t = scanpp();
	//fprintf(stderr, "JML TO '%s' %d %d %d\n", Text, Inclev, Isp, t);
		if (!Inclev && Isp && XEOF == t) 
			fatal("missing '#endif'");
	} while (frozen(1) && Expandmac);
	if (t == Syntoken)
		Syntoken = 0;
	return t;
}

int scanraw(void) {
	int	t, oisp;
	int 	xpd;
	oisp = Isp;
	//Isp = 0;
	xpd = Expandmac;
	Expandmac = 0;
	t = scan();
	Expandmac = xpd;
	//Isp = oisp;
	return t;
}

void reject(void) {
	Rejected = Token;
	Rejval = Value;
	strcpy(Rejtext, Text);
}
