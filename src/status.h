/** @file
 * Zbiór narzędzi umożliwiający łatwiejsze zarządzanie błędnymi sytuacjami.
 */
#ifndef __STATUS_H__
#define __STATUS_H__

#include <stdbool.h>

/// Status jest aliasem na typ @p bool i służy do sygnalizacji wadliwego
/// działania.
typedef bool Status;

/// CHECK_RET zwraca @p false, jeśli argument jest fałszywy.
#define CHECK_RET(x)                                                           \
    if ((x) == false)                                                          \
        return false;

#endif /* __STATUS_H__ */
