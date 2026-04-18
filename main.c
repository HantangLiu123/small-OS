#include "address_map.h"
#include "os.h"

TCB tasks[MAX_TASKS];
uint32_t task_stacks[MAX_TASKS][STACK_SIZE] __attribute__((aligned(16)));
int current_task = 0;
int active_tasks = 0; // 记录实际注册的任务数

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

int main()
{
    __asm__ volatile("csrci mstatus, 0x8");
    timer_init(50000000);
    interrupt_init();
    // 初始化任务
    task_init(0, task_led);
    task_init(1, task_hex);

    current_task = 0;
    start_first_task(&tasks[0]);

    while (1)
        ; // 理论上永远不会回到这里
    return 0;
}
