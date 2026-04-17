.section .text
.global switch_to

# switch_to(uint32_t *old_sp, uint32_t new_sp)
# a0: 指向当前任务 sp 成员的地址
# a1: 下一个任务的 sp 值
switch_to:
    # 1. 在当前栈上开辟空间，保存上下文
    addi sp, sp, -52
    sw ra, 48(sp)
    sw s0, 44(sp)
    sw s1, 40(sp)
    sw s2, 36(sp)
    sw s3, 32(sp)
    sw s4, 28(sp)
    sw s5, 24(sp)
    sw s6, 20(sp)
    sw s7, 16(sp)
    sw s8, 12(sp)
    sw s9, 8(sp)
    sw s10, 4(sp)
    sw s11, 0(sp)

    # 2. 保存旧栈指针到 TCB
    sw sp, 0(a0)

    # 3. 切换到新栈指针
    mv sp, a1

    # 4. 从新栈中恢复上下文
    lw ra, 48(sp)
    lw s0, 44(sp)
    lw s1, 40(sp)
    lw s2, 36(sp)
    lw s3, 32(sp)
    lw s4, 28(sp)
    lw s5, 24(sp)
    lw s6, 20(sp)
    lw s7, 16(sp)
    lw s8, 12(sp)
    lw s9, 8(sp)
    lw s10, 4(sp)
    lw s11, 0(sp)
    addi sp, sp, 52

    # 5. 返回。此时 ra 已经是新任务的地址了！
    ret
