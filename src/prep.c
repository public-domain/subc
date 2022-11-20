/*
 *	NMH's Simple C Compiler, 2011,2012,2014
 *	Preprocessor
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

struct MacroBuf {
	char params[NAMELEN+1];
	char args[TEXTLEN+1];
};

static struct MacroBuf Macexp[MAXFNARGS];

static int append(int k, char *buf, char *s, int maxlen, int quote) {
	int n;
	char *p;
	p = buf + k;
	n = 0;
	if (quote && n + k < maxlen) {
		p[n] = '"'; n++;
	}

	while (n + k < maxlen - 1 && *s != '\0') {
		if (quote) {
			if (*s == '\\') {
				p[n] = '\\'; n++;
				p[n] = '\\'; n++;
			} else if (*s == '"') {
				p[n] = '\\'; n++;
				p[n] = '"'; n++;
			} else {
				p[n] = *s;
				n++;
			}
		} else {	
				p[n] = *s;
				n++;
		}
		s++;
	}
	if (quote && n + k < maxlen) {
		p[n] = '"'; n++;
	}
	p[n] = '\0';
	if (k + n >= maxlen) 
		fatal("buffer overflow in macro expansion");
	return k + n;
}

static int replace(int na, int k, char *buf, int maxlen) {
	int i;
	int q = 0;
	char *tok;
	tok = buf + k;
	if (*tok == '#') {
		tok++;
		q = 1;
	}
	for (i = 0; i < na; i++) {
		if (!strcmp(tok, Macexp[i].params)) {
			buf[k] = '\0';
			return append(k, buf, Macexp[i].args, maxlen, q);
		}
	}
	return strlen(buf);
}

static char *expandbody(char *s, int na) {
	static char buf[TEXTLEN+1];
	int c, ni, i;

	buf[0] = '\0';
	ni = 0;
	i = 0;
	while (*s && i < TEXTLEN) {
		if (*s == '"' || *s == '\'') {
			c = *s;
			if (ni < i) {
				buf[i] = '\0';
				i = replace(na, ni, buf, TEXTLEN);
			}
			ni = i + 1;	
			buf[i] = *s;
			s++; i++;
			while (*s && *s != c && i < TEXTLEN) {
				if (*s == '\\') {
					buf[i] = *s;
					s++; i++;
				}
				buf[i] = *s;
				s++; i++;
			}
			if (*s == c && i < TEXTLEN) {
				buf[i] = *s;
				s++; i++;
			}
			ni = i;
			continue;
		}
		if (s[0] == '#' && s[1] == '#') {
			buf[i] = '\0';
			i = replace(na, ni, buf, TEXTLEN);
			ni = i;
			s += 2;
			continue;
		}
		if (!(isalpha(*s) || isdigit(*s) || '_' == *s)) {
			buf[i] = '\0';
			if (ni < i) {
				i = replace(na, ni, buf, TEXTLEN);
			}
			if (*s == '#') 
				ni = i;
			else 	
				ni = i + 1;
		}
		buf[i] = *s;
		i++; s++;
	}
	buf[i] = '\0';
	if (ni < i) {
		replace(na, ni, buf, TEXTLEN);
	}
	/* printf("XPND: %s\n", buf); */
	return buf;
}

static int getargs(void) {
	int l, na, k;
	Token = scanraw();
	if (Token != LPAREN) fatal("missing '(' after macro name");
	l = 1;
	na = 0;
	k = 0;
	Macexp[na].args[k] = '\0';
	while (l > 0  && Token != XEOF && na < MAXFNARGS - 1) {
		Token = scanraw();
		if (Token == RPAREN) l--;
		if (Token == LPAREN) l++;
		if (Token == COMMA && l == 1) {
			na++;
			k = 0;
			Macexp[na].args[k] = '\0';
		} else if (l > 0) {
			k = append(k, Macexp[na].args, Text, TEXTLEN, 0);
		} else {
			if (k != 0) na++;
		}
	}
	Macexp[na].args[k] = '\0';
	if (l != 0) fatal("Missing ')' after macro call");
	return na;
}

static int getparams(char **ps) {
	int np, k;
	char *s;
	s = *ps;
	np = 0;
	k = 0;
	Macexp[np].params[k] = '\0';
	s++;
	while ('\0' != *s && np < MAXFNARGS - 1) {
		if (')' == *s) {
			s++;
			break;
		}
		if (',' == *s) {
			np++;
			k = 0;
			Macexp[np].params[k] = '\0';
		} else {
			if (k >= NAMELEN) 
				fatal("parameter name too long in macro");

			if (!isspace(*s)) {
				Macexp[np].params[k] = *s;
				k++;
				Macexp[np].params[k] = '\0';
			}
		}
		s++;
	}
	if (k != 0) np++;
	*ps = s;
	return np;
}

static char *expandmac(char *s) {
	int na, np;
	na = getargs();
	np = getparams(&s);
	if (na != np) fatal("wrong number of arguments to macro");
	return expandbody(s, na);
}

void playmac(char *s) {
	if (Mp >= MAXNMAC) fatal("too many nested macros");
	if ('(' == s[0]) { 
		if (Mp > 0) fatal("too many nested function like macros");
		s = expandmac(s);
	}
	Macc[Mp] = next();
	Macp[Mp++] = s;
}

int getln(char *buf, int max) {
	int	k;

	if (fgets(buf, max, Infile) == NULL) return 0;
	k = strlen(buf);
	if (k) buf[--k] = 0;
	if (k && '\r' == buf[k-1]) buf[--k] = 0;
	return k;
}

static void defmac(void) {
	char	name[NAMELEN+1];
	char	buf[TEXTLEN+1], *p;
	int	y;
	int 	l;

	Token = scanraw();
	if (Token != IDENT)
		error("identifier expected after '#define': %s", Text);
	copyname(name, Text);
	buf[0] = ' ';
	buf[1] = 0;
	if ('(' == Putback) {
		putback(' ');
		buf[0] = '(';
	}
	l = 0;
	if ('\n' == Putback)
		buf[0] = 0;
	else 
		l = getln(buf+1, TEXTLEN-2)+1;

	while (l > 0 && l < TEXTLEN-2 && buf[l-1] == '\\') {
		l--;
		buf[l] = 0;
		Line++;
		l += getln(buf+l, TEXTLEN-1-l);
	}
	p = buf;
	if (' ' == *p) {
		for (; isspace(*(p+1)); p++)
			;
	}
		
	if ((y = findmac(name)) != 0) {
		if (strcmp(Mtext[y], buf)) /* FIXME: should be p not buf??? */
			error("macro redefinition: %s", name);
	}
	else {
		addglob(name, 0, TMACRO, 0, 0, 0, globname(p), 0);
	}
	Line++;
}

static void undef(void) {
	char	name[NAMELEN+1];
	int	y;

	Token = scanraw();
	copyname(name, Text);
	if (IDENT != Token)
		error("identifier expected after '#undef': %s", Text);
	if ((y = findmac(name)) != 0)
		Names[y] = "#undef'd";
}

static void include(void) {
	char	file[TEXTLEN+1], path[TEXTLEN+1];
	int	c, k;
	FILE	*inc, *oinfile;
	char	*ofile;
	int	oc, oline;
	char	*p;

	if ((c = skip()) == '<')
		c = '>';
	k = getln(file, TEXTLEN-strlen(SCCDIR)-9);
	Line++;
	if (!k || file[k-1] != c)
		error("missing delimiter in '#include'", NULL);
	if (k) file[k-1] = 0;
	if (c == '"') {
		strcpy(path, file);
		if ((inc = fopen(path, "r")) == NULL) {
			strcpy(path, File);
			p = path + strlen(path);
			while (p > path) {
				p--;
				if (*p == '/') {
					p[1] = '\0';
					break;
				}
			}
			strcat(path, file);
		} else 
			fclose(inc);
	} else {
		strcpy(path, SCCDIR);
		strcat(path, "/include/");
		strcat(path, file);
	}
	if ((inc = fopen(path, "r")) == NULL)
		error("cannot open include file: %s", path);
	else {
		Inclev++;
		oc = next();
		oline = Line;
		ofile = File;
		oinfile = Infile;
		Line = 1;
		putback('\n');
		File = path;
		Infile = inc;
		Token = scan();
		while (XEOF != Token)
			top();
		Line = oline;
		File = ofile;
		Infile = oinfile;
		fclose(inc);
		putback(oc);
		Inclev--;
	}
}

static void ifdef(int expect) {
	char	name[NAMELEN+1];

	if (Isp >= MAXIFDEF)
		fatal("too many nested '#ifdef's");
	Token = scanraw();
	copyname(name, Text);
	if (IDENT != Token)
		error("identifier expected in '#ifdef'", NULL);
	if (frozen(1))
		Ifdefstk[Isp++] = P_IFNDEF;
	else if ((findmac(name) != 0) == expect)
		Ifdefstk[Isp++] = P_IFDEF;
	else
		Ifdefstk[Isp++] = P_IFNDEF;
}

static void p_else(void) {
	if (!Isp)
		error("'#else' without matching '#ifdef'", NULL);
	else if (frozen(2))
		;
	else if (P_IFDEF == Ifdefstk[Isp-1])
		Ifdefstk[Isp-1] = P_ELSENOT;
	else if (P_IFNDEF == Ifdefstk[Isp-1])
		Ifdefstk[Isp-1] = P_ELSE;
	else
		error("'#else' without matching '#ifdef'", NULL);
		
}

static void endif(void) {
	if (!Isp)
		error("'#endif' without matching '#ifdef'", NULL);
	else
		Isp--;
}

static void pperror(void) {
	char	buf[TEXTLEN+1];

	if ('\n' == Putback)
		buf[0] = 0;
	else
		getln(buf, TEXTLEN-1);
	error("#error: %s", buf);
	exit(1);
}

static char FNbuf[TEXTLEN];

static void setline(void) {
	char	buf[TEXTLEN+1], *p, *q;

	if ('\n' == Putback)
		buf[0] = 0;
	else
		getln(buf, TEXTLEN-1);
	Line = atoi(buf) - 1;
	if ((p = strchr(buf, '"')) != NULL) {
		p++;
		if ((q = strchr(p, '"')) != NULL) {
			*q = 0;
			File = strcpy(FNbuf, p);
		}
	}
}

static void junkln(void) {
	while (!feof(Infile) && fgetc(Infile) != '\n')
		;
	Line++;
}

int frozen(int depth) {
	return Isp >= depth &&
		(P_IFNDEF == Ifdefstk[Isp-depth] ||
		P_ELSENOT == Ifdefstk[Isp-depth]);
}

void preproc(void) {
	putback('#');
	Token = scanraw();
	if (	frozen(1) &&
		P_IFDEF != Token && P_IFNDEF != Token &&
		P_ELSE != Token && P_ENDIF != Token
	) {
		junkln();
		return;
	}
	switch (Token) {
	case P_DEFINE:	defmac(); break;
	case P_UNDEF:	undef(); break;
	case P_INCLUDE:	include(); break;
	case P_IFDEF:	ifdef(1); break;
	case P_IFNDEF:	ifdef(0); break;
	case P_ELSE:	p_else(); break;
	case P_ENDIF:	endif(); break;
	case P_ERROR:	pperror(); break;
	case P_LINE:	setline(); break;
	case P_PRAGMA:	junkln(); break;
	default:	junkln(); break;
			break;
	}
}
