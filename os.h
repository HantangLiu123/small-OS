#ifndef __OS_H__
#define __OS_H__

#include "address_map.h"
#include <stdint.h>

#define MAX_TASKS 3
#define STACK_SIZE 1024
#define TASK_REG_NUM 31

// 任务控制块
typedef struct
{
    uint32_t regs[TASK_REG_NUM];
    uint32_t mepc;
    TaskState state;
    int id;
} TCB;

typedef enum
{
    UNUSED = 0,
    READY,
    RUNNING,
    BLOCKED,
    DEAD
} TaskState;

// 外部汇编接口
extern void interrupt_init(void);
extern void yield(void);
extern void start_first_task(TCB *task);

// 内核 C 接口
void tcb_init(int id, void (*func)(void));
void timer_init(uint32_t period);
int task_create(void (*func)(void));
void init_task_states();
int task_kill(int id);

// 全局变量声明
extern TCB tasks[MAX_TASKS];
extern uint32_t task_stacks[MAX_TASKS][STACK_SIZE];
extern TCB *current_task;

#endif
