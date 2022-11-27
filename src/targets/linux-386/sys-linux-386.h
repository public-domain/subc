/*
 *	NMH's Simple C Compiler, 2013,2014
 *	Linux/386 environment
 */

#define OS		"Linux"
#define ASCMD		"as --32 -o %s %s"
#define LDCMD		"ld -m elf_i386 -o %s %s/lib/%scrt0.o"
#define SYSLIBC		""
#define DEFINES		"__SUBC__\0__linux__\0__gnu_linux__\0__i386__\0\0"
/*
 * __linux__
 * __ANDROID__ __linux__
 * __FreeBSD__ __unix__
 * __NetBSD__
 * __OpenBSD__
 * __Darwin__ __APPLE__ __MACH__
 * WIN32 _WIN32 
 * _WIN64 WIN32 _WIN32
 * __MSDOS__
 * __AMIGA__
 *
 * __I86__
 * __i386__
 * __x86_64__ __LP64__ 
 * __mips__
 * __mips__ __mips64__ __LP64__ __BIG_ENDIAN__
 * __mips__ __mips64__ __mipsel__ __mips64el__ __LP64__
 * __arm__
 * __aarch64__ __LP64__
 * __ppc__ __BIG_ENDIAN__
 * __m68k__ __BIG_ENDIAN__
 * __pdp10__
 * __pdp11__
 * __nova__
 * __riscv__ __riscv32__
 * __vax__
 * __sparc64__ 
 *
 * */
