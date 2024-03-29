/*
 *	NMH's Simple C Compiler, 2011--2022
 *	Declaration parser
 */

#include "defs.h"
#include "data.h"
#include "decl.h"

static int declarator(int arg, int scls, char *name, int *pprim, int *psize,
			int *pval, int *pinit, int *off, int lab);

/*
 * enumdecl := { enumlist } ;
 *
 * enumlist :=
 *	  enumerator
 *	| enumerator , enumlist
 *
 * enumerator :=
 *	  IDENT
 *	| IDENT = constexpr
 */

static void enumdecl(int glob) {
	int	v = 0;
	char	name[NAMELEN+1];

	Token = scan();
	if (IDENT == Token)
		Token = scan();
	lbrace();
	while (RBRACE != Token) {
		copyname(name, Text);
		ident();
		if (ASSIGN == Token) {
			Token = scan();
			v = constexpr();
		}
		if (glob)
			addglob(name, PINT, TCONSTANT, 0, 0, v++, NULL, 0, 0);
		else
			addloc(name, PINT, TCONSTANT, 0, 0, v++, 0);
		if (Token != COMMA)
			break;
		Token = scan();
		if (eofcheck()) return;
	}
	rbrace();
	semi();
}

/*
 * initlist :=
 *	  { const_list }
 *	| STRLIT
 *
 * const_list :=
 *	  constexpr
 *	| constexpr , const_list
 */

static void locstatname(char *nbuf, char *name) {
	int	l;
	strncpy(nbuf, Names[Thisfn], NAMELEN);
	nbuf[NAMELEN] = 0;
	l = strlen(nbuf);
	strncpy(nbuf + l, "__", NAMELEN -l);
	nbuf[NAMELEN] = 0;
	l = strlen(nbuf);
	strncpy(nbuf + l, name, NAMELEN -l);
	nbuf[NAMELEN] = 0;
}

static int initlist(char *name, int prim, int scls, int lab) {
	int	n = 0, v;
	char	buf[30];
	char    nbuf[NAMELEN+1];
	int 	val;
	gendata();
	if (CAUTO == scls) {
		locstatname(nbuf, name);
		genloclabel(lab);
	} else {
		genname(name);
	}
	if (STRLIT == Token) {
		if (PUCHAR != prim)
			error("initializer type mismatch: %s", name);
		gendefs(Text, Value);
		gendefb(0);
		genalign(Value-1);
		val = Value;
		Token = scan();
		return -val;
	}
	lbrace();
	while (Token != RBRACE) {
		v = constexpr();
		if (PUCHAR == prim) {
			if (v < 0 || v > 255) {
				sprintf(buf, "%d", v);
				error("initializer out of range: %s", buf);
			}
			gendefb(v);
		}
		else {
			gendefw(v);
		}
		n++;
		if (COMMA == Token)
			Token = scan();
		else
			break;
		if (eofcheck()) return 0;
	}
	if (PUCHAR == prim) genalign(n);
	Token = scan();
	if (!n) error("too few initializers", NULL);
	return n;
}

int getunsig(int t) {
	return t == CHAR? PUCHAR:
		t == SHORT? PUSHORT:
		t == INT? PUINT:
		t == LONG? PULONG:
		t;
}

int getsig(int t) {
	return t == CHAR? PCHAR:
		t == SHORT? PSHORT:
		t == INT? PINT:
		t == LONG? PLONG:
		t;
}


int primtype(int t, char *s) {
	int	p, y;
	char	sname[NAMELEN+1];

	p = t == CHAR? PUCHAR:
		t == INT? PINT:
		t == LONG? PLONG:
		t == SHORT? PSHORT:
		t == UNSIGNED? PUINT:
		t == SIGNED? PINT:
		t == STRUCT? PSTRUCT:
		t == UNION? PUNION:
		t == DOUBLE? PDOUBLE:
		t == FLOAT? PFLOAT:
		PVOID;
	if (PUNION == p || PSTRUCT == p) {
		if (!s) {
			Token = scan();
			copyname(sname, Text);
			s = sname;
			if (IDENT != Token) {
				error("struct/union name expected: %s", Text);
				return p;
			}
		}
		if ((y = findstruct(s)) == 0 || Prims[y] != p)
			error("no such struct/union: %s", s);
		p |= y;
	}
	return p;
}

int usertype(char *s) {
	int	y;

	if ((y = findsym(s)) == 0) return 0;
	return CTYPE == Stcls[y]? (y | PTYPE): 0;
}

/*
 * pmtrdecl :=
 *	  ( )
 *	| ( pmtrlist )
 *	| ( pmtrlist , ... )
 *
 * pmtrlist :=
 *	  primtype declarator
 *	| primtype declarator , pmtrlist
 *	| usertype declarator
 *	| usertype declarator , pmtrlist
 *
 * usertype :=
 *	  TYPEDEF_NAME
 */

static int pmtrdecls(void) {
	char	name[NAMELEN+1];
	int	utype, prim, type, size, na, addr;
	int	dummy;

	if (RPAREN == Token)
		return 0;
	na = 0;
	addr = 2*BPW;
	for (;;) {
		utype = 0;
		if (CONST == Token) {
			Token = scan();
		}
		if (na > 0 && ELLIPSIS == Token) {
			Token = scan();
			na = -(na + 1);
			break;
		}
		else if (IDENT == Token &&
			 (utype = usertype(Text)) == 0)
		{
			prim = PINT;
		}
		else {
			if (	CHAR == Token || INT == Token ||
				VOID == Token || UNSIGNED == Token ||
				SIGNED == Token || LONG == Token ||
				DOUBLE == Token || FLOAT == Token ||
				SHORT == Token || 
				STRUCT == Token || UNION == Token ||
				(IDENT == Token && utype != 0)
			) {
				name[0] = 0;
				if (UNSIGNED == Token) {
					Token = scan();
					if (CHAR == Token || SHORT == Token ||
						INT == Token || LONG == Token)
					{
						prim = getunsig(Token);
						Token = scan();
					} else {
						prim = PUINT;
					}
				} else if (SIGNED == Token) {
					Token = scan();
					if (CHAR == Token || SHORT == Token ||
						INT == Token || LONG == Token)
					{
						prim = getsig(Token);
						Token = scan();
					} else {
						prim = PINT;
					}
				} else if (SHORT == Token) {
					prim = PSHORT;
					Token = scan();
					if (INT == Token) {
						Token = scan();
					}
				} else if (LONG == Token) {
					prim = PLONG;
					Token = scan();
					if (INT == Token) {
						Token = scan();
					}
				} else {
					prim = utype? Prims[utype & ~STCMASK]:
						primtype(Token, NULL);
					Token = scan();
				}
				if (RPAREN == Token && prim == PVOID && !na)
					return 0;
			}
			else {
				error("#ERR002 type specifier expected at: %s", 
						Text);
				Token = synch(RPAREN);
				return na;
			}
		}
		size = 1;
		type = declarator(1, CAUTO, name, &prim, &size, &dummy,
				&dummy, &dummy, 0);
		if ((utype && TARRAY == Types[utype & ~STCMASK]) || TARRAY == type) {
			prim = pointerto(prim);
			type = TVARIABLE;
		}
		addloc(name, prim, type, CAUTO, size, addr, 0);
		addr += BPW;
		na++;
		if (COMMA == Token)
			Token = scan();
		else
			break;
	}
	return na;
}

int pointerto(int prim) {
	int	y;
	char	name[NAMELEN + 1];

	if (U8PP == prim || I8PP == prim || 
	    U16PP == prim || I16PP == prim ||
	    U32PP == prim || I32PP == prim ||
	    U64PP == prim || I64PP == prim ||
	    F32PP == prim || F64PP == prim ||
	    VOIDPP == prim ||
	    FUNPTR == prim ||
	    (prim & STCMASK) == STCPP || (prim & STCMASK) == UNIPP ||
	    (prim & STCMASK) == TYPEPP
	) 
	{
		sprintf(name, "*%d", prim);
		y = findglob(name);
		if (y > 0) return y;
		y = addglob(name, 0, TTYPEDEF, CTYPE, PTRSIZE, prim, NULL, 0, 0);
		return y | TYPEPTR;
	}
	y = prim & ~STCMASK;
	switch (prim & STCMASK) {
	case PSTRUCT:	return STCPTR | y;
	case STCPTR:	return STCPP | y;
	case PUNION:	return UNIPTR | y;
	case UNIPTR:	return UNIPP | y;
	case PTYPE:	return TYPEPTR | y;
	case TYPEPTR:	return TYPEPP | y;
	}
	return PI8 == prim? I8PTR:
		I8PTR == prim? I8PP:
		PI16 == prim? I16PTR:
		I16PTR == prim? I16PP:
		PI32 == prim? I32PTR:
		I32PTR == prim? I32PP:
		PI64 == prim? I64PTR:
		I64PTR == prim? I64PP:
		PU8 == prim? U8PTR:
		U8PTR == prim? U8PP:
		PU16 == prim? U16PTR:
		U16PTR == prim? U16PP:
		PU32 == prim? U32PTR:
		U32PTR == prim? U32PP:
		PU64 == prim? U64PTR:
		U64PTR == prim? U64PP:
		PF64 == prim? F64PTR:
		F64PTR == prim? F64PP:
		PF32 == prim? F32PTR:
		F32PTR == prim? F32PP:
		PVOID == prim? VOIDPTR: VOIDPP;
}

/*
 * declarator :=
 *	  IDENT
 *	| * IDENT
 *	| * * IDENT
 *	| * IDENT [ constexpr ]
 *	| IDENT [ constexpr ]
 *	| IDENT = ldlabexpr
 *	| IDENT [ ] = initlist
 *	| IDENT pmtrdecl
 *	| IDENT [ ]
 *	
 *	| ( * IDENT ) ( )
 */

static int declarator(int pmtr, int scls, char *name, int *pprim, int *psize,
			int *pval, int *pinit, int *poff, int lab)
{
	int	type = TVARIABLE;
	int	ptrptr = 0;
	char	*unsupp;

	unsupp = "unsupported typedef syntax";
	if (STAR == Token) {
		Token = scan();
		*pprim = pointerto(*pprim);
		while (STAR == Token) {
			Token = scan();
			*pprim = pointerto(*pprim);
			ptrptr = 1;
		}
	}
	else if (LPAREN == Token) {
		if (CTYPE == scls)
			error(unsupp, NULL);
		if (*pprim != PINT)
			error("function pointers are limited to type 'int'",
				NULL);
		Token = scan();
		*pprim = FUNPTR;
		match(STAR, "(*name)()");
	}
	if (IDENT != Token) {
		error("missing identifier at: %s", Text);
		name[0] = 0;
	}
	else {
		copyname(name, Text);
		Token = scan();
	}
	if (FUNPTR == *pprim) {
		rparen();
		lparen();
		rparen();
	}
	if (!pmtr && ASSIGN == Token) {
		if (CTYPE == scls)
			error(unsupp, NULL);
		Token = scan();
		*pval = ldlabexpr(pinit, poff);
	}
	else if (!pmtr && LPAREN == Token) {
		if (CTYPE == scls)
			error(unsupp, NULL);
		Token = scan();
		*psize = pmtrdecls();
		rparen();
		return TFUNCTION;
	}
	else if (LBRACK == Token) {
		if (ptrptr)
			error("too many levels of indirection: %s", name);
		Token = scan();
		if (RBRACK == Token) {
			if (CTYPE == scls)
				error(unsupp, NULL);
			Token = scan();
			if (pmtr) {
				*pprim = pointerto(*pprim);
			}
			else {
				type = TARRAY;
				*psize = 1;
				if (ASSIGN == Token) {
					Token = scan();
					if (!inttype(*pprim))
						error("initialization of"
							" pointer array not"
							" supported",
							NULL);
					*psize = initlist(name, *pprim, scls, lab);
					*pinit = OP_LIT;
					if (CAUTO == scls)
						*pinit = 0;
				}
				else if (CEXTERN != scls) {
					error("automatically-sized array"
						" lacking initialization: %s",
						name);
				}
			}
		}
		else {
			*psize = constexpr();
			if (*psize < 1) {
				error("invalid array size", NULL);
				*psize = 0;
			}
			type = TARRAY;
			rbrack();
		}
	}
	if (PVOID == *pprim)
		error("#ERR004 'void' is not a valid type: %s", name);
	return type;
}

int upgrade_array(int utype, int type, int *size) {
	if (utype && TARRAY == Types[utype & ~STCMASK]) {
		if (TARRAY == type)
			error("unsupported typedef (array of array)", NULL);
		*size = *size? *size * Sizes[utype & ~STCMASK]: 
				Sizes[utype & ~STCMASK];
		return TARRAY;
	}
	return type;
}

/*
 * localdecls :=
 *        ldecl
 *      | ldecl localdecls
 *
 * ldecl :=
 *	  primtype ldecl_list ;
 *	| usertype ldecl_list ;
 *	| lclass primtype ldecl_list ;
 *	| lclass ldecl_list ;
 *	| enum_decl
 *	| struct_decl
 *
 * lclass :=
 *	| AUTO
 *	| EXTERN
 *	| REGISTER
 *	| STATIC
 *	| VOLATILE
 *
 * ldecl_list :=
 *	  declarator
 *	| declarator , ldecl_list
 */

int localdecls(int addr) {
	char	name[NAMELEN+1];
	int	utype, prim, type, size, val, ini;
	int	stat, extn;
	int	pbase, rsize;
	int	sig, unsig, off;
	int	lab;

	utype = 0;
	while ( AUTO == Token || EXTERN == Token || REGISTER == Token ||
		STATIC == Token || VOLATILE == Token || CONST == Token ||
		INT == Token || UNSIGNED == Token || SIGNED == Token ||
		LONG == Token || SHORT == Token ||
		DOUBLE == Token || FLOAT == Token ||
		CHAR == Token || VOID == Token ||
		ENUM == Token ||
		STRUCT == Token || UNION == Token ||
		(IDENT == Token && (utype = usertype(Text)) != 0)
	) {
		if (CONST == Token) {
			Token = scan();
		}
		if (ENUM == Token) {
			enumdecl(0);
			continue;
		}
		sig = unsig = 0;
		extn = stat = 0;
		if (AUTO == Token || REGISTER == Token || STATIC == Token ||
			VOLATILE == Token || EXTERN == Token
		) {
			stat = STATIC == Token;
			extn = EXTERN == Token;
			Token = scan();
			if (	INT == Token || CHAR == Token ||
				VOID == Token ||
				DOUBLE == Token || FLOAT == Token ||
				STRUCT == Token || UNION == Token
			) {
				prim = primtype(Token, NULL);
				Token = scan();
			} else if (SHORT == Token) {
				prim = primtype(Token, NULL);
				Token = scan();
				if (INT == Token) {
					Token = scan();
				}
			} else if (LONG == Token) {
				prim = primtype(Token, NULL);
				Token = scan();
				if (INT == Token) {
					Token = scan();
				}
			} else if (UNSIGNED == Token) {
				unsig = 1;
			} else if (SIGNED == Token) {
				sig = 1;
			} else if (utype) {
				prim = Prims[utype & ~STCMASK];
			}
			else
				prim = PINT;
		} else if (UNSIGNED == Token) {
			unsig = 1;
		} else if (SIGNED == Token) {
			sig = 1;
		} else if (utype) {
			prim = Prims[utype & ~STCMASK];
			Token = scan();
		} else if (LONG == Token || SHORT == Token) {
			prim = primtype(Token, NULL);
			Token = scan();
			if (INT == Token) {
				Token = scan();
			}
		} else {
			prim = primtype(Token, NULL);
			Token = scan();
		}
		if (unsig) {
			Token = scan();
			if (CHAR == Token || SHORT == Token ||
				INT == Token || LONG == Token)
			{
				prim = getunsig(Token);
				Token = scan();
			} else {
				prim = PUINT;
			}
		} else if (sig) {
			Token = scan();
			if (CHAR == Token || SHORT == Token ||
				INT == Token || LONG == Token)
			{
				prim = getsig(Token);
				Token = scan();
			} else {
				prim = PINT;
			}
		}
		pbase = prim;
		for (;;) {
			prim = pbase;
			if (eofcheck()) return 0;
			size = 1;
			lab = ini = val = 0;
			if (stat) {
				lab = label();
			}
			type = declarator(0, CAUTO, name, &prim, &size,
					&val, &ini, &off, lab);
			type = upgrade_array(utype, type, &size);
			rsize = objsize(prim, type, size);
			if (rsize < 0) rsize = -rsize;
			rsize = (rsize + INTSIZE-1) / INTSIZE * INTSIZE;
			if (stat) {
				addloc(name, prim, type, CLSTATC, size,
					lab, val);
			}
			else if (extn) {
				addloc(name, prim, type, CEXTERN, size,
					0, val);
			}
			else {
				addr -= rsize;
				addloc(name, prim, type, CAUTO, 
						size, addr, val);
			}
			if (ini && !stat) {
				if (Nli >= MAXLOCINIT) {
					error("too many local initializers",
						NULL);
					Nli = 0;
				}
				LIaddr[Nli] = addr;
				LItype[Nli] = prim;
				LIini[Nli] = ini;
				LIval[Nli++] = val;
			}
			if (COMMA == Token)
				Token = scan();
			else
				break;
		}
		semi();
		utype = 0;
	}
	return addr;
}

static int intcmp(int *x1, int *x2) {
	while (*x1 && *x1 == *x2)
		x1++, x2++;
	return *x1 - *x2;
}

static void signature(int fn, int from, int to) {
	int	types[MAXFNARGS+1], i;

	if (to - from > MAXFNARGS)
		error("too many function parameters", Names[fn]);
	for (i=0; i<MAXFNARGS && from < to; i++)
		types[i] = Prims[--to];
	types[i] = 0;
	if (NULL == Stext[fn]) {
		Stext[fn] = galloc((i+1) * sizeof(int), 1);
		memcpy(Stext[fn], types, (i+1) * sizeof(int));
	}
	else if (intcmp((int *) Stext[fn], types))
		error("declaration does not match prior prototype: %s",
			Names[fn]);
}

/*
 * decl :=
 *	  declarator { localdecls stmt_list }
 *	| decl_list ;
 *
 * decl_list :=
 *	  declarator
 *	| declarator , decl_list
 */

void decl(int clss, int prim, int utype) {
	char	name[NAMELEN+1];
	int	pbase, type, size = 0, val, init;
	int	lsize, off;
	int loop = 0;
	pbase = prim;
	name[0] = '\0';
	for (;;) {
		prim = pbase;
		val = 0;
		init = 0;
		type = declarator(0, clss, name, &prim, &size, 
				&val, &init, &off, 0);
		type = upgrade_array(utype, type, &size);
		if (TFUNCTION == type) {
			clss = clss == CSTATIC? CSPROTO: CEXTERN;
			Thisfn = addglob(name, prim, type, clss, size, 0,
					NULL, 0, 0);
			signature(Thisfn, Locs, NSYMBOLS);
			if (LBRACE == Token) {
				clss = clss == CSPROTO? CSTATIC:
					clss == CEXTERN? CPUBLIC: clss;
				Thisfn = addglob(name, prim, type, clss, size,
					0, NULL, 0, 0);
				Token = scan();
				//Thislsize = localdecls(0);
				Nli = 0;
				Thislsize = 0;
				gentext();
				if (CPUBLIC == clss) genpublic(name);
				genaligntext();
				genentry(name);
				//genstack(lsize);
				//genlocinit();
				Retlab = label();
				compound(0);
				genlab(Retlab);
				genconst(name, Thislsize);
				genstack(-Thislsize);
				genexit();
				if (O_debug & D_LSYM)
					dumpsyms("LOCALS: ", name, Locs,
						NSYMBOLS);
			}
			else {
				semi();
			}
			clrlocs();
			return;
		}
		if (CEXTERN == clss && init) {
			error("initialization of 'extern': %s", name);
		}
		addglob(name, prim, type, clss, size, val, NULL, init, off);
		if (COMMA == Token)
			Token = scan();
		else
			break;
	}
	semi();
}

/*
 * structdecl :=
 *	  STRUCT IDENT { member_list } opt_decl ;
 *	| UNION IDENT { member_list } opt_decl ;
 *	| STRUCT { member_list } opt_decl ;
 *	| UNION { member_list } opt_decl ;
 *
 * opt_decl :=
 *      | decl
 *
 * member_list :=
 *	  primtype mdecl_list ;
 *	| primtype mdecl_list ; member_list
 *	| usertype mdecl_list ;
 *	| usertype mdecl_list ; member_list
 *
 * mdecl_list :=
 *	  declarator
 *	| declatator , mdecl_list
 */

void structdecl(int clss, int uniondecl) {
	int	utype, base, prim, size, dummy, type, addr = 0;
	char	name[NAMELEN+1], sname[NAMELEN+1];
	int	align, maxalign, y, usize = 0;

	Token = scan();
	if (IDENT == Token) {
		copyname(sname, Text);
		Token = scan();
	}
	else {
		sname[0] = 0;
	}
	if (Token != LBRACE) {
		prim = primtype(uniondecl? UNION: STRUCT, sname);
		decl(clss, prim, 0);
		return;
	}
	y = addglob(sname, uniondecl? PUNION: PSTRUCT, TSTRUCT,
			CMEMBER, 0, 0, NULL, 0, 0);
	Token = scan();
	utype = 0;
	align = 0;
	maxalign = 1;
	while (	INT == Token || CHAR == Token || VOID == Token ||
		LONG == Token || SHORT == Token || CONST == Token ||
		DOUBLE == Token || FLOAT == Token ||
		SIGNED == Token || UNSIGNED == Token ||
		STRUCT == Token || UNION == Token ||
		(IDENT == Token && (utype = usertype(Text)) != 0)
	) {
		if (CONST == Token) {
			Token = scan();
		}
		if (Token == UNSIGNED) {
			Token = scan();
			if (CHAR == Token ||
				INT == Token ||
				SHORT == Token ||
				LONG == Token)
			{
				base = getunsig(Token);
				Token = scan();
			} else {
				base = PUINT;
			}
		} else if (SIGNED == Token) {
			Token = scan();
			if (CHAR == Token ||
				INT == Token ||
				SHORT == Token ||
				LONG == Token)
			{
				base = getsig(Token);
				Token = scan();
			} else {
				base = PINT;
			}
	
		} else {
			base = utype? Prims[utype & ~STCMASK]: 
				primtype(Token, NULL);
			Token = scan();
		}
		size = 0;
		for (;;) {
			if (eofcheck()) return;
			prim = base;
			type = declarator(1, CMEMBER, name, &prim, &size,
						&dummy, &dummy, &dummy, 0);
			align = size;
			size = objsize(prim, type, size);
			if (align > 1) {
				/* for arrays */
				align = size / align;
			} else {
				align = size;
			}
			if (align > 1) {
				addr = (addr + align-1) / align * align;
			}
			if (align > maxalign) {
				maxalign = align;
			}
			addglob(name, prim, type, CMEMBER, size, addr,
				NULL, 0, 0);
			if (size < 1)
				error("size of struct/union member"
					" is unknown: %s",
					name);
			if (uniondecl) {
				usize = size > usize? size: usize;
			}
			else {
				addr += size;
			}
			if (Token != COMMA) break;
			Token = scan();
		}
		semi();
		utype = 0;
	}
	rbrace();
	if (!uniondecl) {
		/* the total size of the structure should be a multiple 
		 * of the largest alignment of any structure member */
		addr = (addr + maxalign-1) / maxalign * maxalign;
	}
	Sizes[y] = uniondecl? usize: addr;
	if (Token != SEMI)
		decl(clss, Prims[y] | y, y);
	else
		semi();
}

/*
 * typedecl :=
 *	  TYPEDEF primtype decl
 *	| TYPEDEF usertype decl
 *	| TYPEDEF structdecl
 */

void typedecl(void) {
	int	utype, prim;

	Token = scan();
	if (CONST == Token) {
		Token = scan();
	}
	if (UNSIGNED == Token) {
		Token = scan();
		if (CHAR == Token ||
			INT == Token ||
			SHORT == Token ||
			LONG == Token)
		{
			prim = getunsig(Token);
			Token = scan();
		} else {
			prim = PUINT;
		}
		decl(CTYPE, prim, 0);
	} else if (SIGNED == Token) {
		Token = scan();
		if (CHAR == Token ||
			INT == Token ||
			SHORT == Token ||
			LONG == Token)
		{
			prim = getsig(Token);
			Token = scan();
		} else {
			prim = PINT;
		}
		decl(CTYPE, prim, 0);
	} else if (STRUCT == Token || UNION == Token) {
		structdecl(CTYPE, UNION == Token);
	}
	else if ((utype = usertype(Text)) != 0) {
		Token = scan();
		decl(CTYPE, Prims[utype & ~STCMASK], utype);
	}
	else {
		prim = primtype(Token, NULL);
		Token = scan();
		decl(CTYPE, prim, 0);
	}
}

/*
 * top :=
 *	  ENUM enumdecl
 *	| decl
 *	| primtype decl
 *	| storclass decl
 *	| storclass primtype decl
 *	| typedecl
 *	| usertype decl
 *	| storclass usertype decl
 *
 * storclass :=
 *	  EXTERN
 *	| STATIC
 */

void top(void) {
	int	utype, prim, clss = CPUBLIC;

	switch (Token) {
	case EXTERN:	clss = CEXTERN; Token = scan(); break;
	case STATIC:	clss = CSTATIC; Token = scan(); break;
	case VOLATILE:	Token = scan(); break;
	}
	if (CONST == Token) {
		Token = scan();
	}
	switch (Token) {
	case ENUM:
		enumdecl(1);
		break;
	case TYPEDEF:
		typedecl();
		break;
	case STRUCT:
	case UNION:
		structdecl(clss, UNION == Token);
		break;
	case UNSIGNED:
		Token = scan();
		if (CHAR == Token ||
			INT == Token ||
			SHORT == Token ||
			LONG == Token)
		{
			prim = getunsig(Token);
			Token = scan();
		} else {
			prim = PUINT;
		}
		decl(clss, prim, 0);
		break;
	case SIGNED:
		Token = scan();
		if (CHAR == Token ||
			INT == Token ||
			SHORT == Token ||
			LONG == Token)
		{
			prim = getsig(Token);
			Token = scan();
		} else {
			prim = PINT;
		}
		decl(clss, prim, 0);
		break;
	case LONG:
	case SHORT:
		prim = primtype(Token, NULL);
		Token = scan();
		if (INT == Token) {
			Token = scan();
		}
		decl(clss, prim, 0);
		break;
	case CHAR:
	case INT:
	case VOID:
	case DOUBLE:
	case FLOAT:
		prim = primtype(Token, NULL);
		Token = scan();
		decl(clss, prim, 0);
		break;
	case IDENT:
		if ((utype = usertype(Text)) != 0) {
			Token = scan();
			decl(clss, Prims[utype & ~STCMASK], utype);
		}
		else
			decl(clss, PINT, 0);
		break;
	default:
		error("#ERR001 type specifier expected at: %s", Text);
		Token = synch(SEMI);
		break;
	}
}

static void stats(void) {
	printf(	"Memory usage: "
		"Symbols: %5d/%5d, "
		"Names: %5d/%5d, "
		"Nodes: %5d/%5d\n",
		Globs, NSYMBOLS,
		Nbot, POOLSIZE,
		Ndmax, NODEPOOLSZ);
}

void defarg(char *s) {
	char	*p;

	if (NULL == s) return;
	if ((p = strchr(s, '=')) != NULL)
		*p++ = 0;
	else
		p = "";
	addglob(s, 0, TMACRO, 0, 0, 0, globname(p), 0, 0);
	if (*p) *--p = '=';
}

void program(char *name, FILE *in, FILE *out, char *def) {
	init();
	defarg(def);
	Infile = in;
	Outfile = out;
	File = Basefile = name;
	genprelude();
	Token = scan();
	while (XEOF != Token)
		top();
	genpostlude();
	if (O_debug & D_GSYM) dumpsyms("GLOBALS", "", 1, Globs);
	if (O_debug & D_STAT) stats();
}
