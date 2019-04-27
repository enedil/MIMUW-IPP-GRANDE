#ifndef __STATUS_H__
#define __STATUS_H__

#include <stdbool.h>

typedef bool Status;

#define CHECK_RET(x) \
    if ((x) == false) return false;

#endif /* __STATUS_H__ */
