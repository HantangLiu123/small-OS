#include "address_map.h"
#include "os.h"
#include <stddef.h>
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
    current->state = READY;
    for (int i = 0; i < MAX_TASKS; i++)
    {
        int next_id = (current->id + 1 + i) % MAX_TASKS;
        if (tasks[next_id].state == READY)
        {
            tasks[next_id].state = RUNNING;
            return &tasks[next_id];
        }
    }
    // should not happen
    return NULL;
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
    int mie_value = 0x8;
    __asm__ volatile("csrc mstatus, %0" ::"r"(mie_value));
    current_task->state = DEAD;
    __asm__ volatile("csrs mstatus, %0" ::"r"(mie_value));

    yield();
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
void tcb_init(int id, void (*func)(void))
{
    uint32_t *sp = &task_stacks[id][STACK_SIZE];
    sp = (uint32_t *)((uint32_t)sp & ~0xF);
    for (int i = 0; i < TASK_REG_NUM; i++)
        tasks[id].regs[i] = 0;
    tasks[id].regs[0] = (uint32_t)task_exit;
    tasks[id].regs[1] = (uint32_t)sp;
    tasks[id].mepc = (uint32_t)func;
    tasks[id].id = id;
    tasks[id].state = READY;
}

static int find_free_slot()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (current_task->state == UNUSED || current_task->state == DEAD)
            return i;
    }
    return -1;
}

int task_create(void (*func)(void))
{
    int mie_value = 0x8;
    __asm__ volatile("csrc mstatus, %0" ::"r"(mie_value));

    int id = find_free_slot();
    if (id < 0)
        goto task_create_cleanup;

    tcb_init(id, func);
    tasks[id].state = READY;

task_create_cleanup:
    __asm__ volatile("csrs mstatus, %0" ::"r"(mie_value));
    return id;
}

void init_task_states()
{
    for (int i = 0; i < MAX_TASKS; i++)
        tasks[i].state = UNUSED;
}

int task_kill(int id)
{
    if (id < 0 || id >= MAX_TASKS)
        return -1;

    int mie_value = 0x8;
    __asm__ volatile("csrc mstatus, %0" ::"r"(mie_value));

    if (tasks[id].state == UNUSED || tasks[id].state == DEAD)
        goto task_kill_cleanup;

    if (id == current_task)
        task_exit();

    tasks[id].state == DEAD;
task_kill_cleanup:
    __asm__ volatile("csrs mstatus, %0" ::"r"(mie_value));
    return 0;
}
