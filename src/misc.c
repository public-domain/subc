/*
 *	NMH's Simple C Compiler, 2011,2014
 *	Miscellanea
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

void init(void) {
	char *defines;

	Line = 1;
	Putback = '\n';
	Rejected = -1;
	Errors = 0;
	Mp = 0;
	Expandmac = 1;
	Syntoken = 0;
	Isp = 0;
	Inclev = 0;
	Globs = 0;
	Locs = NSYMBOLS;
	Nbot = 0;
	Ntop = POOLSIZE;
	Ndmax = 0;
	Bsp = 0;
	Csp = 0;
	Q_type = empty;
	Q_cmp = cnone;
	Q_bool = bnone;
	addglob("", 0, 0, 0, 0, 0, NULL, 0);
	defines = DEFINES;
	while (defines[0]) {
		addglob(defines, 0, TMACRO, 0, 0, 0, globname(""), 0);
		defines += strlen(defines) + 1;
	}
	Infile = stdin;
	File = "(stdin)";
	Basefile = NULL;
	Outfile = stdout;
	opt_init();
}

int chrpos(char *s, int c) {
	char	*p;

	p = strchr(s, c);
	return p? p-s: -1;
}

void copyname(char *name, char *s) {
	strncpy(name, s, NAMELEN);
	name[NAMELEN] = 0;
}

void match(int t, char *what) {
	if (Token == t) {
		Token = scan();
	}
	else {
		error("%s expected", what);
	}
}

void lparen(void) {
	match(LPAREN, "'('");
}

void rparen(void) {
	match(RPAREN, "')'");
}

void lbrace(void) {
	match(LBRACE, "'{'");
}

void rbrace(void) {
	match(RBRACE, "'}'");
}

void rbrack(void) {
	match(RBRACK, "']'");
}

void semi(void) {
	match(SEMI, "';'");
}

void colon(void) {
	match(COLON, "':'");
}

void ident(void) {
	match(IDENT, "identifier");
}

int eofcheck(void) {
	if (XEOF == Token) {
		error("missing '}'", NULL);
		return 1;
	}
	return 0;
}

int inttype(int p) {
	return PINT == p || PUCHAR == p || PUINT == p ||
		PCHAR == p || PSHORT == p || PUSHORT == p ||
		PULONG == p || PLONG == p;
}

int ptrtype1(int p)
{
	return UCHARPTR == p ||
               CHARPTR == p ||
               INTPTR == p ||
               UINTPTR == p ||
               LONGPTR == p ||
               ULONGPTR == p ||
               SHORTPTR == p ||
               USHORTPTR == p;
}

int unsigtype(int p) {
	if (!inttype(p)) {
		return 1;
	}
	return PUINT == p || PUCHAR == p || PULONG == p || PUSHORT == p;
}

int comptype(int p) {
	p &= STCMASK;
	return p == PSTRUCT || p == PUNION;
}

void notvoid(int p) {
	if (PVOID == p)
		error("void value in expression", NULL);
}
