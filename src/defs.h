/*
 *	NMH's Simple C Compiler, 2011--2021
 *	Definitions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cg.h"
#include "sys.h"

#define VERSION		"2021-08-15"

#ifndef SCCDIR
 #define SCCDIR		"."
#endif

#ifndef AOUTNAME
 #define AOUTNAME	"a.out"
#endif

#define SCCLIBC		"%s/lib/libscc.a"

#define PREFIX		'C'
#define LPREFIX		'L'

#define TEXTLEN		2048
#define NAMELEN		64

#define MAXFILES	32

#define MAXIFDEF	16
#define MAXNMAC		32
#define MAXCASE		256
#define MAXBREAK	16
#define MAXLOCINIT	32
#define MAXFNARGS	32

/* assert(NSYMBOLS < PSTRUCT) */
#define NSYMBOLS	16384
#define POOLSIZE	262144
#define NODEPOOLSZ	4096	/* ints */

/* types */
enum {
	TVARIABLE = 1,
	TARRAY,
	TFUNCTION,
	TCONSTANT,
	TMACRO,
	TSTRUCT,
	TTYPEDEF,
};

/* primitive types */
enum {
	PU8 = 1,
	PI8,
	PU16,
	PI16,
	PU32,
	PI32, /* 5 */
	PU64,
	PI64,
	PF32,
	PF64, /* 10 */
	U8PTR,
	I8PTR,
	U16PTR,
	I16PTR,
	U32PTR,
	I32PTR,
	U64PTR,
	I64PTR,
	F32PTR,
	F64PTR, /* 20 */
	U8PP,
	I8PP,
	U16PP,
	I16PP,
	U32PP,
	I32PP,
	U64PP,
	I64PP,
	F32PP,
	F64PP, /* 30 */
	PVOID,
	VOIDPTR,
	VOIDPP,
	FUNPTR,
	PTYPE   = 0x01000000,
	PSTRUCT = 0x02000000,
	PUNION  = 0x03000000,
	STCPTR  = 0x04000000,
	STCPP   = 0x05000000,
	UNIPTR  = 0x06000000,
	UNIPP   = 0x07000000,
	TYPEPTR = 0x08000000,
	TYPEPP  = 0x09000000,
	STCMASK = 0x0F000000
};

/* storage classes */
enum {
	CPUBLIC = 1,
	CEXTERN,
	CSTATIC,
	CLSTATC,
	CAUTO,
	CSPROTO,
	CMEMBER,
	CSTCDEF,
	CTYPE
};

/* lvalue structure */
enum {
	LVSYM,
	LVPRIM,
	LVADDR,
	LV
};

/* debug options */
enum {
	D_LSYM = 1,
	D_GSYM = 2,
	D_STAT = 4
};

/* addressing modes */
enum {
	empty,
	addr_auto,
	addr_static,
	addr_globl,
	addr_label,
	literal,
	auto_byte,
	auto_word,
	auto_dword,
	auto_qword,
	static_byte,
	static_word,
	static_dword,
	static_qword,
	globl_byte,
	globl_word,
	globl_dword,
	globl_qword
};

/* compare instructions */
enum {
	cnone,
	equal,
	not_equal,
	less,
	greater,
	less_equal,
	greater_equal,
	below,
	above,
	below_equal,
	above_equal
};

/* boolean instructions */
enum {
	bnone,
	lognot,
	normalize
};

/* AST node */
struct node_stc {
	int		op;
	struct node_stc	*left, *right;
	int		args[1];
};

#define node	struct node_stc

/* tokens */
enum {
	SLASH, STAR, MOD, PLUS, MINUS, LSHIFT, RSHIFT,
	GREATER, GTEQ, LESS, LTEQ, EQUAL, NOTEQ, AMPER,
	CARET, PIPE, LOGAND, LOGOR,

	ARROW, ASAND, ASXOR, ASLSHIFT, ASMINUS, ASMOD, ASOR, ASPLUS,
	ASRSHIFT, ASDIV, ASMUL, ASSIGN, AUTO, BREAK, CASE, CHAR, COLON,
	COMMA, CONST, CONTINUE, DECR, DEFAULT, DO, DOUBLE, DOT, 
	ELLIPSIS, ELSE, ENUM,
	EXTERN, FLOAT, FLOATLIT, FOR, IDENT, IF, INCR, 
	INT, INTLIT, LBRACE, LBRACK, LONG,
	LPAREN, NOT, QMARK, RBRACE, RBRACK, REGISTER, RETURN, RPAREN,
	SEMI, SHORT, SIGNED, SIZEOF, STATIC, STRLIT, STRUCT, SWITCH, 
	TILDE, TYPEDEF, UNION, UNSIGNED, VOID, VOLATILE, WHILE, 
	XEOF, XMARK, XEOL,

	P_DEFINE, P_ELSE, P_ELIF, P_ELIFNOT, P_ELSENOT, P_ENDIF, 
	P_ERROR, P_IF, P_IFDEF,
	P_IFNDEF, P_INCLUDE, P_LINE, P_PRAGMA, P_UNDEF
};

/* AST operators */
enum {
/*0*/	OP_GLUE, OP_ADD, OP_ADDR, OP_ASSIGN, OP_BINAND, OP_BINIOR,
/*6*/	OP_BINXOR, OP_BOOL, OP_BRFALSE, OP_BRTRUE, OP_CALL, OP_CALR,
/*12*/	OP_COMMA, OP_DEC, OP_DIV, OP_EQUAL, OP_GREATER, OP_GTEQ,
/*18*/	OP_IDENT, OP_IFELSE, OP_LAB, OP_LDLAB, OP_LESS, OP_LIT,
/*24*/	OP_LOGNOT, OP_LSHIFT, OP_LTEQ, OP_MOD, OP_MUL, OP_NEG,
/*30*/	OP_NOT, OP_NOTEQ, OP_PLUS, OP_PREDEC, OP_PREINC, OP_POSTDEC,
/*36*/	OP_POSTINC, OP_RSHIFT, OP_RVAL, OP_SCALE, OP_SCALEBY, OP_SUB
};

