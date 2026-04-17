#include "address_map.h"
#include "os.h"

TCB tasks[MAX_TASKS];
uint32_t task_stacks[MAX_TASKS][STACK_SIZE];
uint8_t current_task = 0;
uint8_t active_tasks = 0; // 记录实际注册的任务数

extern void switch_to(uint32_t *old_sp, uint32_t new_sp);

void yield()
{
    if (active_tasks < 2)
        return; // 只有一个任务时没必要切换

    uint8_t old_task = current_task;
    current_task = (current_task + 1) % active_tasks; // 只在已注册的任务中轮转

    switch_to(&tasks[old_task].sp, tasks[current_task].sp);
}

// 任务 0：流水灯 (保持不变)
void task_led()
{
    volatile int *leds = (int *)LED_BASE;
    uint32_t val = 0x1;
    while (1)
    {
        *leds = val;
        val = (val << 1);
        if (val > 0x200)
            val = 0x1; // 10位 LEDR0-LEDR9

        for (volatile int i = 0; i < 200000; i++)
            ; // 增加延时防止切得太快
        yield();
    }
}

// 任务 1：HEX0 循环显示 0-3
void task_hex()
{
    volatile int *hex = (int *)HEX3_HEX0_BASE;
    // 0-3 的段码 (高电平有效: 1位点亮)
    // 0b00111111, 0b00000110, 0b01011011, 0b01001111
    uint32_t seg_codes[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111};
    uint8_t count = 0;

    while (1)
    {
        // 只修改 HEX0 (低7位)，不影响 HEX1-3
        uint32_t current_val = *hex & 0xFFFFFF80;
        *hex = current_val | (seg_codes[count] & 0x7F);

        count = (count + 1) % 4;

        for (volatile int i = 0; i < 200000; i++)
            ;
        yield();
    }
}

// 任务初始化逻辑修正
void task_init(uint8_t id, void (*task_func)(void))
{
    // 确保 id 不超过数组范围
    if (id >= MAX_TASKS)
        return;

    uint32_t *sp = &task_stacks[id][STACK_SIZE];

    // RISC-V 栈帧构造 (与 switch_to 汇编匹配)
    // 我们需要预留 13 个寄存器的位置 (s0-s11 + ra)
    sp -= 13;

    // 重点：sp[12] 对应汇编里的 48(sp)，即 ra
    sp[12] = (uint32_t)task_func;

    // 记录初始化的栈指针
    tasks[id].sp = (uint32_t)sp;

    if (id >= active_tasks)
        active_tasks = id + 1;
}

int main()
{
    // 1. 初始化任务
    task_init(0, task_led);
    task_init(1, task_hex);

    // 2. 启动第一个任务
    // 我们需要一个临时变量来保存当前的上下文（虽然这个上下文再也不会被用到了）
    uint32_t dummy_sp;
    current_task = 0;

    // 从这里“跳”进任务 0 的代码世界
    switch_to(&dummy_sp, tasks[0].sp);

    while (1)
        ; // 理论上永远不会回到这里
    return 0;
}
