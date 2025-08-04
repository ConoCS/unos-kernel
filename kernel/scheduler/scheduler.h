#ifndef _SCHEDULER_UNOS_
#define _SCHEDULER_UNOS_

#include <stdint.h>
#include "schetask.h"

/* Definisikan ENUM TASK. Berikut daftar ENUM nya
    1. TASK_RUNNING
    2. TASK_READY
    3. TASK_WAITING
    4. TASK_TERMINATED */

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_WAITING,
    TASK_TERMINATED,
    TASK_PRIORITY
} ENUM_TASK;

/* Definisikan STRUCT TASK berisi proses ID task, stack pointer, 
   kapan waktu jalannya, dan pengarah ke task selanjutnya*/

typedef struct task{
    uint64_t pid;
    ENUM_TASK Task;
    uint64_t *StackPointer;
    uint64_t timeslice;

    struct task *Next;
    task_func_t function;
} task_t;


extern task_t TaskPool[64];
extern task_t *CurrentTask;
extern task_func_t FunctionList[];
extern int FunctionCount;
extern int TaskCount;

void dotask();
void createtask(task_func_t function, ENUM_TASK TASKLEVEL);

#endif