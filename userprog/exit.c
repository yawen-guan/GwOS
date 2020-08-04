#include "wait.h"
#include "debug.h"
#include "thread.h"


/**
 * @brief 子进程结束自身
 * 
 * @param status 
 */
void sys_exit(int32_t status) {
   struct pcb* child = running_thread();
   child->exit_status = status; 
   if (child->parent_pid == -1) {
      PANIC("sys_exit: child->parent_pid is -1\n");
   }
   //give all the children to init
   list_traversal(&thread_all_list, change_parent_to_init, child->pid);
   release_prog_resource(child); 
   // wakeup parent
   struct pcb* parent = pid2pcb(child->parent_pid);
   if (parent != NULL && parent->status == TASK_WAITING) {
      thread_unblock(parent);
   }
   thread_block(TASK_HANGING);
}
