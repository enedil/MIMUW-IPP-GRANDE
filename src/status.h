#ifndef _STATUS_H__
#define _STATUS_H__

#include <stdbool.h>

typedef bool Status;

#define CHECK_RET(x) \
    if ((x) == false) return false;

#endif /* _STATUS_H__ */
