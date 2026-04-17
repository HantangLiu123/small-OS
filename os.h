#ifndef __OS_H__
#define __OS_H__

#include <stdint.h>

#define MAX_TASKS 3
#define STACK_SIZE 1024 // 每个任务 4KB 栈空间

typedef struct
{
    uint32_t sp;         // 栈指针，必须是结构体第一个成员（方便汇编访问）
    uint32_t *stack_ptr; // 栈内存起始地址
} TCB;

void yield(void);
void task_init(uint8_t id, void (*task_func)(void));

#endif
