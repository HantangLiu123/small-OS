#include "address_map.h"
#include "os.h"
#include <stdint.h>

// 寄存器映射结构体
typedef struct
{
    volatile uint32_t status;
    volatile uint32_t control;
    volatile uint32_t period_l;
    volatile uint32_t period_h;
} TimerDevice;

uint32_t c_trap_handler(void)
{
    uint32_t mcause;
    uint32_t yield_req = 0;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));

    if (mcause & 0x80000000)
    {
        uint32_t code = mcause & 0x7FFFFFFF;
        if (code == 11)
        { // 外部中断
            TimerDevice *timer = (TimerDevice *)TIMER_BASE;
            timer->status = 0;
            yield_req = 1; // 标记需要切换
        }
    }
    return yield_req;
}

void timer_init(uint32_t period)
{
    TimerDevice *timer = (TimerDevice *)TIMER_BASE;
    timer->control = 0x8;
    timer->period_l = period & 0xFFFF;
    timer->period_h = (period >> 16) & 0xFFFF;
    timer->status = 0;
    timer->control = 0x7;
}

// 统一的任务初始化
void task_init(int id, void (*func)(void))
{
    if (id >= MAX_TASKS)
        return;

    uint32_t *sp = &task_stacks[id][STACK_SIZE];
    sp -= 31;

    sp[30] = (uint32_t)func; // ra
    sp[29] = (uint32_t)func;
    tasks[id].sp = (uint32_t)sp;

    if (id >= active_tasks)
        active_tasks = id + 1;
}

void yield()
{
    int mstatus_value = 0x8;
    __asm__ volatile("csrci mstatus, 0x8"); // 关中断保护临界区
    int old = current_task;
    current_task = (current_task + 1) % MAX_TASKS;
    switch_to(&tasks[old].sp, tasks[current_task].sp);
}
