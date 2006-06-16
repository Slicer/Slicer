/*
  The following code is basically copied from 
  Robert Seeger (RHS) at 
  this website http://wiki.tcl.tk/12975
*/


#include "utilities.h"

#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include  <errno.h>
#include <string.h>


#define ERR_STRLEN 128

#define SIZE_INT 20  /* %d Signed decimal integer: +/- + 19 chars */
#define SIZE_UINT 21 /* %u Unsigned decimal integer: "+" + 20 chars */
#define SIZE_IINT 21 /* %i Signed decimal string: +/- + 20 chars */
#define SIZE_OINT 23 /* %o Unsigned octal string: 0 + 22 chars */
#define SIZE_XINT 18 /* %xX Unsigned hexadecimal string: "0x" + 16 chars */
#define SIZE_CHAR 3  /* %c Unicode char */
#define SIZE_DOUBLE 320 /* %f Float: up to 316 chars */
/* Should be define a SIZE_LONG_DOUBLE for modifier 'L'? */
#define SIZE_POINTER SIZE_XINT /* %p Pointer: same as hex "0x" + 16 chars */

#define bigger(x,y) (x < y ? y : x)



int formatStringLength(const char *format, ...) 
{
    int size;
    va_list ap;

    va_start(ap, format);
    size = vaFormatStringLength(format, ap);
    va_end(ap);

    return size;
}



int vaFormatStringLength(const char *format, va_list ap) 
{
    int total;
    char *ptr = (char *)format;
    char *tmp;
    size_t minsize = 0;

    /* Start with the length of the format string + 1 for the \0 */
    total = strlen(format) + 1;

    /* Iterate over the characters in the format string */
    while(*ptr != '\0') {
        if(*ptr++ == '%') {  /* Its a format starter */
            if(*ptr == '%') { /* Its just a percent sign */
                total--;
                ptr++;
                continue;
            }
            /* Handle the "non-sizing" modifiers */
            while( strchr("-+ 0#", *ptr) != NULL ) {
                ptr++;
                total--;
            }

            /* Next has to be a size modifier, if its a number */
            tmp = ptr;
            minsize = strtoul(ptr, (char **) &ptr, 10);
            total -= ptr - tmp;

            /* Ok, now the "after the decimal" part */
            /* Here, we just add it to the total    */
            /* we don't modify the min size         */
            if ( *ptr == '.' ) {
                ptr++;
                tmp = ptr;
                total += strtoul(ptr, (char **) &ptr, 10);
                total -= (ptr - tmp) +1;
            }

            /* Last before format specifier is the length modifier */
            while ( strchr("hlL", *ptr) != NULL ) {
                ptr++;
                total--;
            }

            switch(*ptr) {
                case 'd':
                    total += bigger(minsize, SIZE_INT) - 2;
                    va_arg(ap, int);
                    break;
                case 'u':
                    total += bigger(minsize, SIZE_UINT) - 2;
                    va_arg(ap, int);
                    break;
                case 'i':
                    total += bigger(minsize, SIZE_IINT) - 2;
                    va_arg(ap, int);
                    break;
#ifdef _MSC_VER
                case 'I':
                    total += bigger(minsize, SIZE_IINT) - 2;
                    va_arg(ap, __int64);
                    break;
#endif
                case 'o':
                    total += bigger(minsize, SIZE_OINT) - 2;
                    va_arg(ap, int);
                    break;
                case 'x':
                case 'X':
                    total += bigger(minsize, SIZE_XINT) - 2;
                    va_arg(ap, int);
                    break;
                case 'c':
                    total += bigger(minsize, SIZE_CHAR) - 2;
                    va_arg(ap, int);
                    break;
                case 's':
                    tmp = (char *)va_arg(ap, char *);
                    total += bigger(minsize, strlen(tmp)) -2;
                    break;
                case 'f':
                    total += bigger(minsize, SIZE_DOUBLE) - 2;
                    va_arg(ap, double);
                    break;
                case 'e':
                case 'E':
                    /* This is a cheat, we come up with a better size */
                    total += bigger(minsize, SIZE_DOUBLE) - 2;
                    va_arg(ap, double);
                    break;
                case 'g':
                case 'G':
                    /* This is a cheat, we come up with a better size */
                    total += bigger(minsize, SIZE_DOUBLE) - 2;
                    va_arg(ap, double);
                    break;
                case 'p':
                    /* This is a cheat, we come up with a better size */
                    total += bigger(minsize, SIZE_POINTER) - 2;
                    va_arg(ap, void *);
                    break;
            }
        }
    }

    if(total <= 0) {
        total = 1;
    }
    return total;
}



