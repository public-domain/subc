/*
 *	NMH's Simple C Compiler, 2011--2021
 *	Symbol table management
 */

#include "defs.h"
#include "data.h"
#include "decl.h"
#include "cgen.h"

int findglob(char *s) {
	int	i;

	for (i=0; i<Globs; i++) {
		if (	Types[i] != TMACRO && Stcls[i] != CMEMBER &&
			*s == *Names[i] && !strcmp(s, Names[i])
		)
			return i;
	}
	return 0;
}

int findloc(char *s) {
	int	i;

	for (i=Locs; i<NSYMBOLS; i++) {
		if (	Stcls[i] != CMEMBER &&
			*s == *Names[i] && !strcmp(s, Names[i])
		)
			return i;
	}
	return 0;
}

int findsym(char *s) {
	int	y;

	if ((y = findloc(s)) != 0) return y;
	return findglob(s);
}

int findmac(char *s) {
	int	i;

	for (i=0; i<Globs; i++)
		if (	NULL != Mtext[i] &&
			*s == *Names[i] && !strcmp(s, Names[i])
		) {
			/* dont't expand macro twice */
			if (Mp > 0 && Mact[Mp-1] == i) return 0;
			return i;
		}
	return 0;
}

int findstruct(char *s) {
	int	i;

	for (i=Locs; i<NSYMBOLS; i++)
		if (	TSTRUCT == Types[i] &&
			*s == *Names[i] && !strcmp(s, Names[i])
		)
			return i;
	for (i=0; i<Globs; i++)
		if (	TSTRUCT == Types[i] &&
			*s == *Names[i] && !strcmp(s, Names[i])
		)
			return i;
	return 0;
}

int findmem(int y, char *s) {
	y++;
	while (	(y < Globs ||
		 (y >= Locs && y < NSYMBOLS)) &&
		CMEMBER ==  Stcls[y]
	) {
		if (*s == *Names[y] && !strcmp(s, Names[y]))
			return y;
		y++;
	}
	return 0;
}

int newglob(void) {
	int	p;

	if ((p = Globs++) >= Locs)
		fatal("too many global symbols");
	return p;
}

int newloc(void) {
	int	p;

	if ((p = --Locs) <= Globs)
		fatal("too many local symbols");
	return p;
}

#ifdef __SUBC__
 #define PTR_INT_CAST	(int)
#else
 #define PTR_INT_CAST	(int) (long)
#endif

char *galloc(int k, int align) {
	int	p, mask;

	k += align * sizeof(int);
	if (Nbot + k >= Ntop)
		fatal("out of space for symbol names");
	p = Nbot;
	Nbot += k;
	mask = sizeof(int)-1;
	if (align)
		while (PTR_INT_CAST &Nlist[p] & mask)
			p++;
	return &Nlist[p];
}

char *globname(char *s) {
	char	*p;
	
	p = galloc(strlen(s)+1, 0);
	strcpy(p, s);
	return p;
}

char *locname(char *s) {
	int	p, k;

	k = strlen(s) + 1;
	if (Nbot + k >= Ntop)
		fatal("out of space for symbol names");
	Ntop -= k;
	p = Ntop;
	strcpy(&Nlist[p], s);
	return &Nlist[p];
}

static void defglob(char *name, int prim, int type, int size, int val,
			int scls, int init, int off)
{
	int	st;

	if (TCONSTANT == type || TFUNCTION == type) return;
	gendata();
	st = scls == CSTATIC;
	if (CPUBLIC == scls) genpublic(name);
	if (init && TARRAY == type)
		return;
	if (TARRAY != type && !(prim & STCMASK)) genname(name);
	if (prim & STCMASK) {
		if (TARRAY == type)
			genbss(gsym(name), objsize(prim, TARRAY, size), st);
		else if (OP_ADDR == init && val) 
			gendefl(gsym(name), val, 0);	
		else if (OP_ADD == init && val) 
			gendefl(gsym(name), val, off);	
		else
			genbss(gsym(name), objsize(prim, TVARIABLE, size), st);
	}
	else if (PUCHAR == prim) {
		if (TARRAY == type)
			genbss(gsym(name), size, st);
		else {
			gendefb(val);
			genalign(1);
		}
	}
	else if (PINT == prim) {
		if (TARRAY == type)
			genbss(gsym(name), size*INTSIZE, st);
		else
			gendefw(val);
	}
	else {
		if (TARRAY == type)
			genbss(gsym(name), size*PTRSIZE, st);
		else
			gendefp(val);
	}
}

int redeclare(char *name, int oldcls, int newcls) {
	switch (oldcls) {
	case CEXTERN:
		if (newcls == 0) return 0;
		if (newcls != CPUBLIC && newcls != CEXTERN)
			error("#ERR005 extern symbol redeclared static: %s", name);
		return newcls;
	case CPUBLIC:
		if (newcls == 0) return 0;
		if (CEXTERN == newcls)
			return CPUBLIC;
		if (newcls != CPUBLIC) {
			error("#ERR006 extern symbol redeclared static: %s", name);
			return CPUBLIC;
		}
		break;
	case CSPROTO:
		if (newcls != CSTATIC && newcls != CSPROTO)
			error("#ERR007 static symbol redeclared extern: %s", name);
		return newcls;
	case CSTATIC:
		if (newcls == 0) return 0;
		if (CSPROTO == newcls)
			return CSTATIC;
		if (newcls != CSTATIC) {
			error("#ERR008 static symbol redeclared extern: %s", name);
			return CSTATIC;
		}
		break;
	case CTYPE:
		error("#ERR009 redefinition of typedef name", Text);
		return CTYPE;
		break;
	}
	error("#ERR010 redefined symbol: %s", name);
	return newcls;
}

int addglob(char *name, int prim, int type, int scls, int size, int val,
		char *mtext, int init, int off)
{
	int	y;
	char 	*stext;
	stext = NULL;
	if (0 == *name)
		y = 0;
	else if ((y = findglob(name)) != 0) {
		scls = redeclare(name, Stcls[y], scls);
		if (CTYPE == scls) return y;
		if (TFUNCTION == Types[y])
			stext = Stext[y];
		if (!mtext)
			mtext = Mtext[y];
	}
	if (0 == y) {
 		y = newglob();
		Names[y] = globname(name);
	}
	else if (TFUNCTION == Types[y] || TMACRO == Types[y]) {
		if (prim == 0 && type == TMACRO && NULL == Mtext[y]) {
			Mtext[y] = (mtext);
			return y;
		}
		if (Prims[y] != prim || Types[y] != type)
			error("#ERR011 redefinition does not match prior type: %s",
				name);
	}
	if (CPUBLIC == scls || CSTATIC == scls)
		defglob(name, prim, type, size, val, scls, init, off);
	if (TTYPEDEF == type) {
		Prims[y] = y;
	} else {
		Prims[y] = prim;
	}
	Types[y] = type;
	Stcls[y] = scls;
	Sizes[y] = size;
	Vals[y] = val;
	Stext[y] = stext;
	Mtext[y] = mtext;
	return y;
}

static void defloc(int prim, int type, int size, int val, int init) {
	gendata();
	if (type != TARRAY && !(prim &STCMASK)) genlab(val);
	if (prim & STCMASK) {
		if (TARRAY == type)
			genbss(labname(val), objsize(prim, TARRAY, size), 1);
		else
			genbss(labname(val), objsize(prim, TVARIABLE, size),1);
	}
	/* FIXME other kind of prime */
	else if (PUCHAR == prim) {
		if (TARRAY == type) {
			if (size > 0) genbss(labname(val), size, 1);
		} else {
			gendefb(init);
			genalign(1);
		}
	}
	else if (PINT == prim) {
		if (TARRAY == type)
			genbss(labname(val), size*INTSIZE, 1);
		else
			gendefw(init);
	}
	else {
		if (TARRAY == type)
			genbss(labname(val), size*PTRSIZE, 1);
		else
			gendefp(init);
	}
}

int addloc(char *name, int prim, int type, int scls, int size, int val,
		int init)
{
	int	y;

	if (findloc(name))
		error("redefinition of: %s", name);
 	y = newloc();
	if (CLSTATC == scls) {
		defloc(prim, type, size, val, init);
	} else if (size < 0) {
		genloccopy(val, init, -size);
	}
	Names[y] = locname(name);
	Prims[y] = prim;
	Types[y] = type;
	Stcls[y] = scls;
	Sizes[y] = size > 0? size: -size;
	Vals[y] = val;
	return y;
}

void clrlocs(void) {
	Ntop = POOLSIZE;
	Locs = NSYMBOLS;
}

int objsize(int prim, int type, int size) {
	int	k = 0, sp;

	sp = prim & STCMASK;
	if (PINT == prim)
		k = INTSIZE;
	else if (PUINT == prim)
		k = UINTSIZE;
	else if (PCHAR == prim)
		k = CHARSIZE;
	else if (PUCHAR == prim)
		k = UCHARSIZE;
	else if (PSHORT == prim)
		k = SHORTSIZE;
	else if (PUSHORT == prim)
		k = USHORTSIZE;
	else if (PLONG == prim)
		k = LONGSIZE;
	else if (PULONG == prim)
		k = ULONGSIZE;
	else if (PDOUBLE == prim)
		k = DOUBLESIZE;
	else if (PFLOAT == prim)
		k = FLOATSIZE;
	else if (INTPTR == prim || UCHARPTR == prim || VOIDPTR == prim ||
			UINTPTR == prim || CHARPTR == prim ||
			SHORTPTR == prim || LONGPTR == prim ||
			USHORTPTR == prim || ULONGPTR == prim ||
			DOUBLEPTR == prim || FLOATPTR == prim)
		k = PTRSIZE;
	else if (INTPP == prim || UCHARPP == prim || VOIDPP == prim ||
			UINTPP == prim || CHARPP == prim ||
			SHORTPP == prim || LONGPP == prim ||
			USHORTPP == prim || ULONGPP == prim ||
			DOUBLEPP == prim || FLOATPP == prim)
		k = PTRSIZE;
	else if (STCPTR == sp || STCPP == sp)
		k = PTRSIZE;
	else if (UNIPTR == sp || UNIPP == sp)
		k = PTRSIZE;
	else if (TYPEPTR == sp || TYPEPP == sp)
		k = PTRSIZE;
	else if (PSTRUCT == sp || PUNION == sp || PTYPE == sp)
		k = Sizes[prim & ~STCMASK];
	else if (FUNPTR == prim)
		k = PTRSIZE;
	if (TFUNCTION == type || TCONSTANT == type || TMACRO == type)
		return -1;
	if (TARRAY == type)
		k *= size;
	return k;
}

static char *typename(int p) {
	switch (p & STCMASK) {
	case PSTRUCT:	return "STRUCT";
	case STCPTR:	return "STCT*";
	case STCPP:	return "STCT**";
	case PUNION:	return "UNION";
	case UNIPTR:	return "UNIO*";
	case UNIPP:	return "UNIO**";
	case PTYPE:	return "unknown";
	case TYPEPTR:	return "TYPE*";
	case TYPEPP:	return "TYPE**";
	}
	return	PINT    == p? "INT":
		PUCHAR  == p? "UCHAR":
		PCHAR   == p? "CHAR":
		PUINT   == p? "UINT":
		PSHORT == p? "SHORT":
		PUSHORT == p? "USHORT":
		PLONG == p? "LONG":
		PULONG == p? "ULONG":
		PDOUBLE == p? "DOUBLE":
		PFLOAT == p? "FLOAT":
		INTPTR  == p? "INT*":
		UCHARPTR == p? "UCHAR*":
		CHARPTR   == p? "CHAR":
		UINTPTR   == p? "UINT*":
		SHORTPTR == p? "SHORT*":
		USHORTPTR == p? "USHORT*":
		LONGPTR == p? "LONG*":
		ULONGPTR == p? "ULONG*":
		DOUBLEPTR == p? "DOUBLE*":
		FLOATPTR == p? "FLOAT*":
		VOIDPTR == p? "VOID*":
		FUNPTR  == p? "FUN*":
		INTPP   == p? "INT**":
		UCHARPP  == p? "UCHAR**":
		CHARPP   == p? "CHAR**":
		UINTPP   == p? "UINT**":
		SHORTPP == p? "SHORT**":
		USHORTPP == p? "USHORT**":
		LONGPP == p? "LONG**":
		ULONGPP == p? "ULONG*":
		DOUBLEPP == p? "DOUBLE**":
		FLOATPP == p? "FLOAT**":
		VOIDPP  == p? "VOID**":
		PVOID   == p? "VOID": "n/a";
}

void dumpsyms(char *title, char *sub, int from, int to) {
	int	i;
	int	*p;

	printf("\n===== %s%s =====\n", title, sub);
	printf(	"PRIM    TYPE  STCLS   SIZE  VALUE  NAME [MVAL]/(SIG)\n"
		"------  ----  -----  -----  -----  -----------------\n");
	for (i = from; i < to; i++) {
		printf("%-6s  %s  %s  %5d  %5d  %s",
			typename(Prims[i]),
			TVARIABLE == Types[i]? "VAR ":
				TARRAY == Types[i]? "ARRY":
				TFUNCTION == Types[i]? "FUN ":
				TCONSTANT == Types[i]? "CNST":
				NULL != Mtext[i]? "MAC ":
				TSTRUCT == Types[i]? "STCT": "n/a",
			CPUBLIC == Stcls[i]? "PUBLC":
				CEXTERN == Stcls[i]? "EXTRN":
				CSTATIC == Stcls[i]? "STATC":
				CSPROTO == Stcls[i]? "STATP":
				CLSTATC == Stcls[i]? "LSTAT":
				CAUTO   == Stcls[i]? "AUTO ":
				CMEMBER == Stcls[i]? "MEMBR":
				CTYPE   == Stcls[i]? "TYPE ": "n/a  ",
			Sizes[i],
			Vals[i],
			Names[i]);
		if (NULL != Mtext[i])
			printf(" [\"%s\"]", Mtext[i]);
		if (TFUNCTION == Types[i]) {
			printf(" (");
			for (p = (int *) Stext[i]; *p; p++) {
				printf("%s", typename(*p));
				if (p[1]) printf(", ");
			}
			putchar(')');
		}
		putchar('\n');
	}
}
