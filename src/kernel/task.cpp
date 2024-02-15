#include "task.h"

Task::Task(target_t target)
{
    uint32_t stack = (uint32_t)this + PAGE_SIZE;
    stack -= sizeof(task_frame_t);
    
}