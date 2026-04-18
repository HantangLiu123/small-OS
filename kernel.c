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

static TCB *schedule(TCB *current)
{
    int next = (current_task + 1) % active_tasks;
    current_task = next;
    return &tasks[next];
}

TCB *c_trap_handler(TCB *current)
{
    uint32_t mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));

    // interrupt?
    if (mcause & 0x80000000)
    {
        uint32_t code = mcause & 0x7FFFFFFF;

        switch (code)
        {
        case 16:
            // timer interrupt
            TimerDevice *timer = (TimerDevice *)TIMER_BASE;
            timer->status = 0;
            return schedule(current);

        default:
            return current;
        }
    }

    // exception
    switch (mcause)
    {
    case 11: {                                   // ecall
        uint32_t syscall_id = current->regs[16]; // a7

        // 跳过 ecall
        current->mepc += 4;

        if (syscall_id == 0)
        { // yield
            return schedule(current);
        }

        return current;
    }

    default:
        while (1)
            ; // panic
    }
}

static void task_exit()
{
    while (1)
        ;
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
    sp = (uint32_t *)((uint32_t)sp & ~0xF);
    for (int i = 0; i < TASK_REG_NUM; i++)
        tasks[id].regs[i] = 0;
    tasks[id].regs[0] = (uint32_t)task_exit;
    tasks[id].regs[1] = (uint32_t)sp;
    tasks[id].mepc = (uint32_t)func;

    if (id >= active_tasks)
        active_tasks = id + 1;
}
