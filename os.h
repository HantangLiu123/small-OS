#ifndef __OS_H__
#define __OS_H__

#include "address_map.h"
#include <stdint.h>

#define MAX_TASKS 3
#define STACK_SIZE 1024

// 任务控制块
typedef struct
{
    uint32_t sp;
} TCB;

// 外部汇编接口
extern void switch_to(uint32_t *old_sp, uint32_t new_sp);
extern void interrupt_init(void);

// 内核 C 接口
void yield(void);
void task_init(int id, void (*func)(void));
void timer_init(uint32_t period);

// 全局变量声明
extern TCB tasks[MAX_TASKS];
extern uint32_t task_stacks[MAX_TASKS][STACK_SIZE];
extern int current_task;
extern int active_tasks;

#endif
