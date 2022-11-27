/*
 *	NMH's Simple C Compiler, 2011--2014
 *	Code generator interface
 */

void cgadd(void);
void cgalign(void);
void cgand(void);
void cgbool(void);
void cgbreq(int n);
void cgbrfalse(int n);
void cgbrge(int n);
void cgbrgt(int n);
void cgbrle(int n);
void cgbrlt(int n);
void cgbrne(int n);
void cgbrtrue(int n);
void cgbruge(int n);
void cgbrugt(int n);
void cgbrule(int n);
void cgbrult(int n);
void cgcall(char *s);
void cgcalr(void);
void cgcalswtch(void);
void cgcase(int v, int l);
void cgclear(void);
void cgclear2(void);
void cgdata(void);
void cgdec1ib(void);
void cgdec1iw(void);
void cgdec1id(void);
void cgdec1iq(void);
void cgdec1pi(int v);
void cgdec2ib(void);
void cgdec2iw(void);
void cgdec2id(void);
void cgdec2iq(void);
void cgdec2pi(int v);
void cgdecgb(char *s);
void cgdecgw(char *s);
void cgdecgd(char *s);
void cgdecgq(char *s);
void cgdeclb(int a);
void cgdeclw(int a);
void cgdecld(int a);
void cgdeclq(int a);
void cgdecpg(char *s, int v);
void cgdecpl(int a, int v);
void cgdecps(int a, int v);
void cgdecsb(int a);
void cgdecsw(int a);
void cgdecsd(int a);
void cgdecsq(int a);
void cgdefb(int v);
void cgdefc(int c);
void cgdefl(int v);
void cgdefp(int v);
void cgdefs(char *name, int off);
void cgdefw(int v);
void cgdefd(int v);
void cgdefq(int v);
void cgdiv(void);
void cgentry(char *s);
void cgeq(void);
void cgexit(void);
void cggbss(char *s, int z);
void cgge(void);
void cggt(void);
void cginc1ib(void);
void cginc1iw(void);
void cginc1id(void);
void cginc1iq(void);
void cginc1pi(int v);
void cginc2ib(void);
void cginc2iw(void);
void cginc2id(void);
void cginc2iq(void);
void cginc2pi(int v);
void cgincgb(char *s);
void cgincgw(char *s);
void cgincgd(char *s);
void cgincgq(char *s);
void cginclb(int a);
void cginclw(int a);
void cgincld(int a);
void cginclq(int a);
void cgincpg(char *s, int v);
void cgincpl(int a, int v);
void cgincps(int a, int v);
void cgincsb(int a);
void cgincsw(int a);
void cgincsd(int a);
void cgincsq(int a);
void cgindb(void);
void cgindw(void);
void cgindd(void);
void cgindq(void);
void cginitlw(int v, int a);
void cgior(void);
void cgjump(int n);
void cglbss(char *s, int z);
void cgldga(char *s);
void cgldgb(char *s);
void cgldgw(char *s);
void cgldgd(char *s);
void cgldgq(char *s);
void cgldinc(void);
void cgldla(int n);
void cgldlab(int id);
void cgldlb(int n);
void cgldlw(int n);
void cgldld(int n);
void cgldlq(int n);
void cgldsa(int n);
void cgldsb(int n);
void cgldsw(int n);
void cgldsd(int n);
void cgldsq(int n);
void cgldswtch(int n);
void cgle(void);
void cglit(int v);
int  cgload2(void);
void cglognot(void);
void cglt(void);
void cgmod(void);
void cgmul(void);
void cgname(char *s);
void cgne(void);
void cgneg(void);
void cgnot(void);
void cgpop2(void);
void cgpopptr(void);
void cgpostlude(void);
void cgprelude(void);
void cgpublic(char *s);
void cgpush(void);
void cgpushlit(int n);
void cgscale(void);
void cgscale2(void);
void cgscale2by(int v);
void cgscaleby(int v);
void cgshl(void);
void cgshr(void);
void cgstack(int n);
void cgstorgb(char *s);
void cgstorgw(char *s);
void cgstorgd(char *s);
void cgstorgq(char *s);
void cgstorib(void);
void cgstoriw(void);
void cgstorid(void);
void cgstoriq(void);
void cgstorlb(int n);
void cgstorlw(int n);
void cgstorld(int n);
void cgstorlq(int n);
void cgstorsb(int n);
void cgstorsw(int n);
void cgstorsd(int n);
void cgstorsq(int n);
void cgsub(void);
void cgswap(void);
void cgand(void);
void cgxor(void);
void cgtext(void);
void cguge(void);
void cgugt(void);
void cgule(void);
void cgult(void);
void cgunscale(void);
void cgunscaleby(int v);
void cgxor(void);
