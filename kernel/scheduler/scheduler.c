#include <unoskrnl.h>

int is_function_valid(task_func_t func) {
    for(int i = 0; i < FunctionCount; i++) {
        if (FunctionList[i] == func) {
            return 1;
        } 
    }
    return 0;
}

void dotask() {
    /* VOID DO TASK membuat 3 task misalnya*/
    task_t *CurrentTask = &TaskPool[0];
    
    for(int i = 0; i < 10; i++) {
        /* TASK PRIORITY */
        for(int j = 0; j < TaskCount; j++) {
            if (TaskPool[j].Task == TASK_PRIORITY) {
                serial_print("Task Priority detected\n");
                for (int k = 0; k <= 1; k++) {
                    task_t *PriorityTask = &TaskPool[j];
                    serial_print("Task Running: ");
                    serial_print_hex((uint64_t)PriorityTask->pid);
                    serial_print("\n");
                    
                        if(PriorityTask->function == 0) {
                            continue;
                        }  else if(!is_function_valid(CurrentTask->function)){
                            serial_print("[Error] Task yang dimasukin ke fungsi tidak ada\n");
                         } else {
                                PriorityTask->function();
                            }

                    TaskPool[j].Task = TASK_TERMINATED;
                }
            }
        }
        /* Cek apakah ada task yang next nya kosong. nilai nya 0 */
        if (TaskPool[i].Task == 0) {
            serial_print("Task terminated... \n");
            break;
        } 
        /* Kode normal buat jalanin task normal. Jika ada TASK_TERMINATED, ignore */
        if(CurrentTask->Task == TASK_TERMINATED) {
            continue;
        }
        serial_print("Task Running: ");
        serial_print_hex((uint64_t)CurrentTask->pid);
        serial_print("\n");

        /* Jalankan fungsi*/
        if(CurrentTask->function == 0) {
            continue;
        }  else if(!is_function_valid(CurrentTask->function)){
            serial_print("[Error] Task yang dimasukin ke fungsi tidak ada\n");
        } else {
            CurrentTask->function();
        }

        /* Terminasi fungsi jika fungsi selanjutnya adalah TASK_TERMINATED*/
        if(CurrentTask->Task == TASK_TERMINATED) {
            serial_print("Task termintated... \n");
            break;
        }

        CurrentTask = CurrentTask->Next;
    }

}

