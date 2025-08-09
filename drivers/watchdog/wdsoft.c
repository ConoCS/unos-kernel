#include <unoskrnl.h>

WatchdogUnOS *WatchdogUnOSKrnl;

WatchdogUnOS *InitWatchdog(USINT32 counter, USINT32 timeout, CHARA8 *Tag, VOID(*Callback)(WatchdogUnOS *WD)) {
    // definisikan memori
    if(WatchdogUnOSKrnl == NULL) {
        WatchdogUnOSKrnl = (WatchdogUnOS*)kmalloc(sizeof(WatchdogUnOS));
        if (WatchdogUnOSKrnl == NULL) {
            serial_printf("[Error] WatchdogUnOSKrnl kmalloc failed\n");
            return NULL;
        }
        serial_printf("Pointer kmalloc WatchdogUnOSKrnl: %p\n", WatchdogUnOSKrnl);
        memset(WatchdogUnOSKrnl, 0, sizeof(WatchdogUnOS));
    }
    // Kalo watchdog sudah aktif, return
    if(WatchdogUnOSKrnl->active == TRUE) return WatchdogUnOSKrnl;

    // setup watchdog berdasarkan value
    USINT32 CounterWatchdog = counter;
    USINT32 TimeoutWatchdog = timeout;

    // Masukan data dan aktifkan watchdog
    WatchdogUnOSKrnl->active = TRUE;
    WatchdogUnOSKrnl->Callback = Callback;  
    WatchdogUnOSKrnl->counter = CounterWatchdog;
    WatchdogUnOSKrnl->timeout = TimeoutWatchdog;
    WatchdogUnOSKrnl->Tag = 0;

    //printf watchdog aktif
    serial_printf("[OK] Watchdog aktif\n");

    return WatchdogUnOSKrnl;
}

VOID ResetWatchdog(WatchdogUnOS *WD) {
    // kalo mati, return
    if(WD->active != TRUE) return;

    WD->counter = 0;
}

VOID WatchdogCountPlus(WatchdogUnOS *WD) {
    // kalo mati, return
    if(WD->active != TRUE) {
        WD->counter = 0;
        return;
    }
    WD->counter++;
}

VOID DisableWatchdogWhile(WatchdogUnOS *WD) {
    if (WD && WD->active == TRUE) {
        WD->active = FALSE;
        serial_printf("[WD] Watchdog dinonaktifkan\n");
    }
}

VOID EnableWatchdogAgain(WatchdogUnOS *WD) {
    if(WD && WD->active == FALSE) {
        WD->active = TRUE;
        serial_printf("[WD] Watchdog diaktifkan kembali\n");
    }
}

VOID DetectWatchdog(WatchdogUnOS *WD) {
    if(WD->active != TRUE) return;

    // deteksi watchdog
    if(WD->counter >= WD->timeout) {
        WD->Callback(WatchdogUnOSKrnl);
    } else {
        return;
    }
}

VOID WDCallback(WatchdogUnOS *WD) {
    (void)WD;
    RaiseKernelPanicError(0x0000121, WATCHDOG_TIMEOUT_TIMER_EXCEPTION);
    serial_printf("EXCEPTION: EXCEPTION HAPPENED\n");
    serial_printf("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n");
    serial_printf("\nError: WATCHDOG_TIMEOUT_TIMER_EXCEPTION\n");
}

