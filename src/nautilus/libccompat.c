/* 
 * This file is part of the Nautilus AeroKernel developed
 * by the Hobbes and V3VEE Projects with funding from the 
 * United States National  Science Foundation and the Department of Energy.  
 *
 * The V3VEE Project is a joint project between Northwestern University
 * and the University of New Mexico.  The Hobbes Project is a collaboration
 * led by Sandia National Laboratories that includes several national 
 * laboratories and universities. You can find out more at:
 * http://www.v3vee.org  and
 * http://xtack.sandia.gov/hobbes
 *
 * Copyright (c) 2015, Kyle C. Hale <kh@u.northwestern.edu>
 * Copyright (c) 2015, The V3VEE Project  <http://www.v3vee.org> 
 *                     The Hobbes Project <http://xstack.sandia.gov/hobbes>
 * All rights reserved.
 *
 * Author: Kyle C. Hale <kh@u.northwestern.edu>
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "LICENSE.txt".
 */

/* 
 *
 * This file is intended to support libc functions
 * 
 *
 */


#define LIBCCOMPAT 1

#include <nautilus/nautilus.h>
#include <nautilus/libccompat.h>
#include <nautilus/thread.h>
#include <nautilus/errno.h>
#include <nautilus/random.h>
#include <dev/hpet.h>


#define GEN_DEF(x) \
    int x (void) { \
        UNDEF_FUN_ERR(); \
        return 0; \
    } 

//Lua
// LUA STRUCTURES....

struct lconv {
    char *decimal_point;      //"."          LC_NUMERIC
    char *grouping;           //""           LC_NUMERIC
    char *thousands_sep;      //""           LC_NUMERIC

    char *mon_decimal_point;  //""           LC_MONETARY
    char *mon_grouping;       //""           LC_MONETARY
    char *mon_thousands_sep;  //""           LC_MONETARY

    char *negative_sign;      //""           LC_MONETARY
    char *positive_sign;      //""           LC_MONETARY
    char *currency_symbol;    //""           LC_MONETARY
    char frac_digits;         //CHAR_MAX     LC_MONETARY
    char n_cs_precedes;       //CHAR_MAX     LC_MONETARY
    char n_sep_by_space;      //CHAR_MAX     LC_MONETARY
    char n_sign_posn;         //CHAR_MAX     LC_MONETARY
    char p_cs_precedes;       //CHAR_MAX     LC_MONETARY
    char p_sep_by_space;      //CHAR_MAX     LC_MONETARY
    char p_sign_posn;         //CHAR_MAX     LC_MONETARY

    char *int_curr_symbol;
    char int_frac_digits;
    char int_n_cs_precedes;
    char int_n_sep_by_space;
    char int_n_sign_posn;
    char int_p_cs_precedes;
    char int_p_sep_by_space;
    char int_p_sign_posn;
};





//=========================================================
static uint64_t dummy_mono_clock = 0;

time_t time (time_t * timer)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);

    if (timer) {
        *timer = tp.tv_nsec;
    }
    return tp.tv_nsec;
}


void 
abort(void) 
{
    printk("Thread called abort\n");
    nk_thread_exit(NULL);
}


int 
__popcountdi2 (long long a)
{
    unsigned long long x2 = (unsigned long long)a;
    x2 = x2 - ((x2 >> 1) & 0x5555555555555555uLL);
    /* Every 2 bits holds the sum of every pair of bits (32) */
    x2 = ((x2 >> 2) & 0x3333333333333333uLL) + (x2 & 0x3333333333333333uLL);
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) (16) */
    x2 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0FuLL;
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) (8) */
    unsigned x = (unsigned)(x2 + (x2 >> 32));
    /* The lower 32 bits hold four 16 bit sums (5 significant bits). */
    /*   Upper 32 bits are garbage */
    x = x + (x >> 16);
    /* The lower 16 bits hold two 32 bit sums (6 significant bits). */
    /*   Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0x0000007F;  /* (7 significant bits) */
}

void 
exit(int status)
{
    printk("Thread called exit (status=%d)\n", status);
    nk_thread_exit((void*)(long)status);
}

int 
clock_gettime (clockid_t clk_id, struct timespec * tp)
{

    if (clk_id != CLOCK_MONOTONIC) {
        printk("NAUTILUS WARNING: using invalid clock type\n");
        return -EINVAL;
    }

    if (!tp) {
        printk("NAUTILUS WARNING: using invalid timespec\n");
        return -EINVAL;
    }

#ifdef NAUT_CONFIG_HPET
    //uint64_t freq = nk_hpet_get_freq();
    uint64_t cnt  = nk_hpet_get_cntr();
    //uint64_t nsec = (1000000000/freq) * cnt;
    uint64_t nsec = cnt * nk_hpet_nanos_per_tick();
    tp->tv_sec    = nsec / 1000000000;
    tp->tv_nsec   = nsec % 1000000000;
#else
    /* runs at "10kHz" */
    tp->tv_nsec = dummy_mono_clock*100000;
    tp->tv_sec  = dummy_mono_clock/10000;
    ++dummy_mono_clock;
#endif

    return 0;
}


void 
__assert_fail (const char * assertion, const char * file, unsigned line, const char * function)
{
    panic("Failed assertion in %s: %s at %s, line %d, RA=%lx\n",
            function,
            assertion,
            file,
            line,
            __builtin_return_address(0));
}


int 
vfprintf (FILE * stream, const char * format, va_list arg)
{
    UNDEF_FUN_ERR();
    return -1;
}


int 
rand (void) {
    int r;

    nk_get_rand_bytes((uint8_t*)&r, 4);

    return RAND_MAX / r;
}

void 
srand (unsigned int seed)
{
    uint64_t s = ((uint64_t)seed) << 32;
    nk_rand_seed(s | 0x330e);
}
    

/* NOTE: these are likely not in any way compliant
 */
void
srand48 (long int seedval)
{
    uint64_t tmp = (((uint64_t) seedval) & 0xffffffffull) << 32;
    nk_rand_seed(tmp | 0x330e);
}


long int
lrand48 (void)
{
    struct nk_rand_info * rand = per_cpu_get(rand);
    uint64_t xi     = rand->xi;
    uint64_t seed   = rand->seed;
    uint64_t n      = rand->n;
    uint64_t m      = 2ull << 48;
    uint64_t a      = 0x5deece66d;
    uint64_t c      = 0xb;
    uint64_t xi_new = (a*xi*n + c) % m;

    nk_rand_set_xi(xi_new);

    return xi_new;
}


union ieee754dbl {
    double d;
    uint64_t ui;
    struct {
        uint32_t mantissa1 : 32;
        uint32_t mantissa0 : 20;
        uint32_t exponent  : 11;
        uint32_t sign      :1;
    } __packed;
} __packed;

double
drand48(void) 
{
    struct nk_rand_info * rand = per_cpu_get(rand);
    uint8_t flags = spin_lock_irq_save(&rand->lock);
    uint64_t xi     = rand->xi;
    uint64_t n      = rand->n;
    uint64_t m      = 2ull << 48;
    uint64_t a      = 0x5deece66dull;
    uint64_t c      = 0xb;
    uint64_t xi_new = (a*xi + c) % m;
    uint16_t *xic   = (uint16_t*)&(rand->xi);

    union ieee754dbl ret;

    nk_rand_set_xi(xi_new);

    ret.sign     = 0;
    ret.exponent = 0x3ff;
    ret.mantissa0 = (xic[2] << 4) | (xic[1] >> 12);
    ret.mantissa1 = ((xic[1] & 0xfff) << 20) | (xic[0] << 4);

    spin_unlock_irq_restore(&rand->lock, flags);
    return ret.d - 1.0;
}

int fprintf (FILE * f, const char * s, ...)
{
    #if 0
        UNDEF_FUN_ERR();
        return -1;
    #else
        va_list arg;
        va_start(arg,s);
        vprintk(s, arg);
        va_end(arg);
        return 0;
    #endif
}

/* ********************


LUA DEF. STARTS HERE...


********************** */

char *
strerror (int errnum)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | stderror ");
    return NULL;
}
FILE *tmpfile(void)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | tmpfile ");
    return NULL;

}
int 
ferror (FILE * f)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | ferror ");
    return -1;
}
FILE *freopen(const char *fname, const char *mode,FILE *stream)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | freopen ");
    return NULL;
}
int 
fclose (FILE * f)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | fclose ");
    return -1;
}


FILE * 
fopen (const char * path, FILE * f)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | fopen ");
    return NULL;
}


FILE *fopen64 (const char * path, FILE * f)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | fopen64 ");
    return NULL;
}

FILE *fdopen (int fd, const char * mode)
{
    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | fdopen ");
    return NULL;
}

char *getenv(const char *name)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | getenv ");
    return NULL;
}

clock_t clock()
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | clock ");
    return -1;
}

char *tmpnam(char *s)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | tmpnam ");
    return NULL;
}

int remove(const char *path)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | remove ");
    return -1;
}
//For LUA
int rename(const char *old, const char *new)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | rename ");
    return -1;
}
//For LUA
int system(const char *command)
{

    UNDEF_FUN_ERR();
    printk("\nlibccompat.c | system ");
    return -1;
}
//For LUA

int fflush (FILE * f)
{
    printk("\nlibccompat.c | fflush ");
    return 0;
}

void (*signal(int sig, void (*func)(int)))(int )
{
    printk("\nlibccompat.c | signal ");
}


//For LUA
int setvbuf(FILE *restrict stream, char *restrict buf, int type,
       size_t size)
{
    printk("\nlibccompat.c | setvbuf ");
    UNDEF_FUN_ERR();
    return -1;
}


//For LUA

int fscanf(FILE *restrict stream, const char *restrict format, ... )
{
    printk("\nlibccompat.c | fscanf ");
    UNDEF_FUN_ERR();
    return -1;

}
//For LUA
void clearerr(FILE *stream)
{
    printk("\nlibccompat.c | clearerr ");
    UNDEF_FUN_ERR();
    return NULL;
}

int printf (const char * s, ...)
{
    printk("\nlibccompat.c | printf ");
    #if 0
        UNDEF_FUN_ERR();
        return -1;
    #else
        va_list arg;
        va_start(arg,s);
        vprintk(s, arg);
        va_end(arg);
        return 0;
    #endif
}

int fputc (int c, FILE * f) 
{
    printk("\nlibccompat.c | fputc ");
    printk("%c");
    return c;
}
int fputs (const char *s, FILE *stream)
{
    printk("\nlibccompat.c | fputs ");
    printk("%s\n", stream);
    return 0;
}
size_t fwrite (const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    printk("\nlibccompat.c | fwrite ");
    UNDEF_FUN_ERR();
    return 1;
}
size_t fread (void *ptr, size_t size, size_t count, FILE *stream)
{
    printk("\nlibccompat.c | fread ");
    UNDEF_FUN_ERR();
    return 1;
}

int getwc (FILE *stream)
{
    printk("\nlibccompat.c | getwc ");
    UNDEF_FUN_ERR();
    return 1;
}
size_t __ctype_get_mb_cur_max(void)
{
    printk("\nlibccompat.c | __ctype_get_mb_cur_max() ");
    UNDEF_FUN_ERR();
    return 0;
}
//For LUA
int fseek(FILE *stream, long offset, int whence)
{
    printk("\nlibccompat.c | Fseek ");
    UNDEF_FUN_ERR();
    return 1;
}
int fseeko64 (FILE *fp, uint64_t offset, int whence)
{
    printk("\nlibccompat.c | Fseeko64 ");
    UNDEF_FUN_ERR();
    return 1;
}
int ungetc (int character, FILE * stream)
{
    printk("\nlibccompat.c | ungetc ");
    UNDEF_FUN_ERR();
    return 1;
}
uint64_t lseek64 (int fd, uint64_t offset, int whence)
{
    printk("\nlibccompat.c | lseek64 ");
    UNDEF_FUN_ERR();
    return 1;
}

uint64_t ftello64 (FILE *stream)
{
    printk("\nlibccompat.c | ftello64 ");
    UNDEF_FUN_ERR();
    return 1;
}
//For LUA
long ftell(FILE *x)
{
    printk("\nlibccompat.c | ftell() ");
    UNDEF_FUN_ERR();
    return -1;
}
int poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
    printk("\nlibccompat.c |poll() ");
    UNDEF_FUN_ERR();
    return 1;
}

int ioctl (int d, unsigned long request, ...)
{
    printk("\nlibccompat.c | ioctl() ");
    UNDEF_FUN_ERR();
    return 1;
}

int syscall (int number, ...)
{
    printk("\nlibccompat.c | syscall ");
    UNDEF_FUN_ERR();
    return 1;
}

char *setlocale (int category, const char *locale)
{
    printk("\nlibccompat.c | setlocate() ");
    UNDEF_FUN_ERR();
    return NULL;
}

locale_t __duplocale(locale_t locobj)
{
    printk("\nlibccompat.c | __duplocale() ");
    UNDEF_FUN_ERR();
    return NULL;
}

char *bindtextdomain (const char * domainname, const char * dirname)
{
    printk("\nlibccompat.c | bindtextdomain ");
    UNDEF_FUN_ERR();
    return NULL;
}

char *textdomain (const char * domainname)
{
    printk("\nlibccompat.c | textdomain ");
    UNDEF_FUN_ERR();
    return NULL;
}

locale_t __newlocale (int category_mask, const char *locale, locale_t base)
{
    printk("\nlibccompat.c | __newlocale() ");
    return (locale_t)((ulong_t)base | (ulong_t)category_mask);
}

char *__nl_langinfo_l (nl_item item, locale_t locale)
{
    printk("\nlibccompat.c | __nl_langinfo_l ");
    UNDEF_FUN_ERR();
    return NULL;
}

char *gettext (const char * msgid)
{
    printk("\nlibccompat.c | gettext ");
    char * ret = (char*)msgid;
    UNDEF_FUN_ERR();
    return ret;
}

int getc(FILE* arg)
{
    printk("\nlibccompat.c | getc ");
    UNDEF_FUN_ERR();
    return -1;

}
//LUA SPECIFIC....................
size_t strftime(char *str, size_t maxsize, const char *format, const struct tm *timeptr)
{
    printk("\nlibccompat.c | strftime ");
    return 0;
}
int feof(FILE * x)
{
    printk("\nlibccompat.c | feof ");
    UNDEF_FUN_ERR();
    return 0;
}

char *fgets(char *str, int n, FILE *stream)
{
    printk("\nlibccompat.c | fgets ");
    UNDEF_FUN_ERR();
    return NULL;
}
void *memchr(const void *str, int c, size_t n)
{
    printk("\nlibccompat.c | memchar ");
    return NULL;
}
void longjmp(int *x, int __y)
{
    printk("\nlibccompat.c | longjmp ");
    UNDEF_FUN_ERR();
}

int setjmp(int *x)
{
    printk("\nlibccompat.c | setjmp ");
    return 0;
}
double fabs(double __x)
{
    printk("\nlibccompat.c | fabs ");
    return 0;
}
double atan(double __x)
{
    printk("\nlibccompat.c | atan ");
    return 45.000;
}
double atan2(double y, double x)
{
    printk("\nlibccompat.c | atan2 ");
    return 135.00;
}
double fmod(double y, double x)
{
    printk("\nlibccompat.c | fmod ");
    return 2.0;
}
double modf(double y, double *x)
{
    printk("\nlibccompat.c | modf ");
    *x = 0;
    return 0.000;
}
double frexp(double x, int *e)
{
    printk("\nlibccompat.c | frexp ");
    *e = 0;
    return 0.5;
}
double ldexp(double x, int exp)
{
    printk("\nlibccompat.c | ldexp ");
    return x;
}
double strtod(const char *str, char **endptr)
{
    printk("\nlibccompat.c | strtod ");
    return 0.0;
}
/*----------*/
double abs(double x)
{
    printk("\nlibccompat.c | abs ");
    //should return absolute value of x
    return x;
}
double sin(double x)
{
    printk("\nlibccompat.c | sin ");
    return x;
}
double sinh(double x)
{
    printk("\nlibccompat.c | sinh ");
return x;
}
double cos(double x)
{
    printk("\nlibccompat.c | cos ");
    return x;
}
double cosh(double x)
{
    printk("\nlibccompat.c | cosh ");
    return x;
}
time_t mktime(struct tm *timeptr)
{
    printk("\nlibccompat.c | mktime ");
    return 0;
}
struct tm *localtime(const time_t *timer)
{
    printk("\nlibccompat.c | localtime");
    return NULL;
}
struct tm *gmtime(const time_t *timer)
{
    printk("\nlibccompat.c | gmtime ");
    return NULL;
}
int strcoll(const char *str1, const char *str2)
{
    printk("\nlibccompat.c | strcoll ");
    return 0;
}
double tan(double x)
{
    printk("\nlibccompat.c | tan ");
return x;
}
double tanh(double x)
{
    printk("\nlibccompat.c | tanh ");
return x;
}
double asin(double x)
{
    printk("\nlibccompat.c | asin ");
return x;
}
double acos(double x)
{
    printk("\nlibccompat.c | acos ");
return x;
}
double ceil(double x)
{
    printk("\nlibccompat.c | ceil ");
return x;
}
double floor(double x)
{
    printk("\nlibccompat.c | floor ");
return x;
}
double difftime(time_t time1, time_t time2)
{
    printk("\nlibccompat.c | difftime ");
    return 0;
}
double sqrt(double x)
{
    printk("\nlibccompat.c | sqrt ");
return x;
}
double pow(double x, double y)
{
    printk("\nlibccompat.c | pow ");
return x;
}
double log(double x)
{
    printk("\nlibccompat.c | log ");
return x;
}
double log10(double x)
{
    printk("\nlibccompat.c | log10 ");
return x;
}
double exp(double x)
{
    printk("\nlibccompat.c | exp ");
return x;
}

/* became lazy... */
GEN_DEF(writev)
GEN_DEF(ungetwc)
GEN_DEF(__errno_location)
GEN_DEF(write)
GEN_DEF(wcrtomb)
GEN_DEF(mbrtowc)
//GEN_DEF(getc)
GEN_DEF(__iswctype_l)
GEN_DEF(wcslen)
GEN_DEF(__strtof_l)
//GEN_DEF(stderr)
GEN_DEF(wmemset)
//GEN_DEF(stdin)
GEN_DEF(fileno)
GEN_DEF(__fxstat64)
GEN_DEF(putc)
GEN_DEF(__wcscoll_l)
GEN_DEF(__towlower_l)
GEN_DEF(wctob)
GEN_DEF(mbsrtowcs)
GEN_DEF(read)
GEN_DEF(wmemmove)
GEN_DEF(__strxfrm_l)
GEN_DEF(wmemchr)
GEN_DEF(__freelocale)
GEN_DEF(__wcsftime_l)
GEN_DEF(wmemcpy)
GEN_DEF(putwc)
GEN_DEF(__stack_chk_fail)
GEN_DEF(__wcsxfrm_l)
GEN_DEF(wcscmp)
GEN_DEF(wcsnrtombs)
GEN_DEF(__strcoll_l)
//GEN_DEF(stdout)
GEN_DEF(btowc)
//GEN_DEF(memchr)
GEN_DEF(strtold_l)
GEN_DEF(wmemcmp)
GEN_DEF(__strtod_l)
//GEN_DEF(setvbuf)
GEN_DEF(__wctype_l)
GEN_DEF(__towupper_l)
GEN_DEF(__uselocale)
GEN_DEF(__strftime_l)
GEN_DEF(mbsnrtowcs)
GEN_DEF(pthread_mutex_init)
GEN_DEF(pthread_mutex_lock)
GEN_DEF(pthread_mutex_unlock)
GEN_DEF(wcscoll)
//GEN_DEF(strcoll)
GEN_DEF(towupper)
GEN_DEF(towlower)
GEN_DEF(iswctype)
//GEN_DEF(strftime)
GEN_DEF(wcsftime)
GEN_DEF(wctype)
GEN_DEF(strtold)
//GEN_DEF(strtod)
GEN_DEF(strtof)
GEN_DEF(__ctype_b_loc)
GEN_DEF(__ctype_toupper_loc)
GEN_DEF(__ctype_tolower_loc)
GEN_DEF(strxfrm)
GEN_DEF(wcsxfrm)
GEN_DEF(__kernel_standard);
GEN_DEF(__get_cpu_features);
