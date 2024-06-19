#include "task.h"

#include <string.h>
#include <memory.h>
#include <task_queue.h>
#include <arena.h>

extern Bitmap kernel_map;
extern void task_switch(Task *next);

static Task* task_stable[NR_TASKS];

Task::Task()
{

}

void Task::create(target_t target, const char *name, uint32_t priority, uint32_t uid)
{
    uint32_t stack = (uint32_t)this + PAGE_SIZE;
    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = target;

    // TODO 处理溢出
    strcpy((char*)this->name, name);
    
    this->stack = (uint32_t*)stack;
    this->priority = priority;
    this->ticks = priority;
    this->jiffies = 0;
    this->state = TASK_READY;
    this->uid = uid;
    // 初始化位图
    this->vmap = (Bitmap*)kmalloc(sizeof(Bitmap));
    void *buf = (void*)memory::alloc_kpage(1);
    this->vmap->init((char*)buf, PAGE_SIZE, KERNEL_MEMORY_SIZE / PAGE_SIZE);

    this->pde = KERNEL_PAGE_DIR;
    this->magic = ONIX_MAGIC;
}

void Task::block(task_state_t state)
{
    TaskQueue::task_block(this, nullptr, state);
}


void Task::unblock()
{
    TaskQueue::task_unblock(this);
}


