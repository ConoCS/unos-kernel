#include <unoskrnl.h>

void print1(){
    serial_print("Hello print 1\n");
}

void print2(){
    serial_print("Hello print 2\n");
}

void print3(){
    serial_print("Hello print 3\n");
}

void print4(){
    serial_print("Hello print 4\n");
}

void drawrectis() {
     drawrect(10, 10, 100, 100, 0x00112233); 
}

task_t TaskPool[64];
task_t *CurrentTask = &TaskPool[0];
task_func_t FunctionList[] = {print1, print2, print3, print4, drawrectis};
int FunctionCount = sizeof(FunctionList)/ sizeof(FunctionList[0]);
int TaskCount;

void createtask(task_func_t function, ENUM_TASK TASKLEVEL) {
    /* Default value nya = 0. isi argumennya 0 jika gatau mau diisi apa*/
    if (function == 0) {
        serial_print("[Warning] Ada createtask yang tidak memiliki function value\n");
    }
    /* Cek terlebih dahulu apakah TaskPool sudah penuh atau belum*/
    if (TaskCount >= 64) {
        serial_print("Task count penuh!\n");
        return;
    }

    /* Assign NewTAsk ke TaskPool*/
    task_t *NewTask = &TaskPool[TaskCount];

    /* Assign properti lain buat task nya */
    NewTask->pid = TaskCount + 1;
    NewTask->Task = TASKLEVEL;
    NewTask->StackPointer = 0;
    NewTask->timeslice = 5;
    NewTask->function = function;
    
    /* Assing task next untuk nya*/
    if(TaskCount == 0) {
        NewTask->Next = 0;
    } else {
        TaskPool[TaskCount - 1].Next = NewTask;
    }

    /* Tambahin counter buat TaskCount */
    TaskCount++;

}