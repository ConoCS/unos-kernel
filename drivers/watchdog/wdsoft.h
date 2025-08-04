#ifndef _UNOS_WATCHDOG_SOFTWARE_
#define _UNOS_WATCHDOG_SOFTWARE_

#include <unostype.h>

// Forward declaration
struct WatchdogUnOS;
typedef struct WatchdogUnOS WatchdogUnOS;

// Callback type
typedef VOID (*WATCHDOG_CALLBACK)(WatchdogUnOS *WD);

// Struct definition
struct WatchdogUnOS {
    USINT32 counter;
    USINT32 timeout;
    BOOL active;
    CONST CHARA8 *Tag;
    WATCHDOG_CALLBACK Callback;
};

// Global instance (optional)
GLOBAL WatchdogUnOS *WatchdogUnOSKrnl;

// Function declarations
WatchdogUnOS *InitWatchdog(USINT32 counter, USINT32 timeout, CHARA8 *Tag, WATCHDOG_CALLBACK Callback);
VOID WatchdogCountPlus(WatchdogUnOS *WD);
VOID DisableWatchdogWhile(WatchdogUnOS *WD);
VOID EnableWatchdogAgain(WatchdogUnOS *WD);
VOID DetectWatchdog(WatchdogUnOS *WD);
VOID WDCallback(WatchdogUnOS *WD);
VOID ResetWatchdog(WatchdogUnOS *WD);

#endif
