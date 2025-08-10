#ifndef _UNOS_PRINTK_
#define _UNOS_PRINTK_

#include <unostype.h>

typedef enum {
    KINFO,
    KERR,
    KWARN,
    KSUCCESS,
} printk_level_t;

UNFUNCTION Printk(printk_level_t level, const char *fmt, ...);

#endif