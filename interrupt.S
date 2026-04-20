.section .text
.global interrupt_init
.global yield
.global start_first_task
.extern c_trap_handler

interrupt_init:
    la t0, trap_entry
    csrw mtvec, t0      # 设置中断入口
    li t0, 0x10000        # mie.MEIE (外部中断，DE1-SoC Timer通常接此)
    csrs mie, t0
    li t0, 0x8
    csrs mstatus, t0
    ret

trap_entry:
    # -------- 找 current task --------
    addi sp, sp, -16
    sw a0, 0(sp)

    la a0, current_task

    # -------- 保存寄存器 --------
    sw ra,   0(a0)
    # sw sp,   4(a0)
    sw gp,   8(a0)
    sw tp,   12(a0)
    sw t0,   16(a0)
    sw t1,   20(a0)
    sw t2,   24(a0)
    sw s0,   28(a0)
    sw s1,   32(a0)
    # sw a0,   36(a0)
    sw a1,   40(a0)
    sw a2,   44(a0)
    sw a3,   48(a0)
    sw a4,   52(a0)
    sw a5,   56(a0)
    sw a6,   60(a0)
    sw a7,   64(a0)
    sw s2,   68(a0)
    sw s3,   72(a0)
    sw s4,   76(a0)
    sw s5,   80(a0)
    sw s6,   84(a0)
    sw s7,   88(a0)
    sw s8,   92(a0)
    sw s9,   96(a0)
    sw s10,  100(a0)
    sw s11,  104(a0)
    sw t3,   108(a0)
    sw t4,   112(a0)
    sw t5,   116(a0)
    sw t6,   120(a0)

    lw t0, 0(sp) # original a0
    sw t0, 36(a0)
    addi sp, sp, 16
    sw sp,   4(a0)

    csrr t0, mepc
    sw t0, 124(a0)

    # -------- 调用 C handler --------
    call c_trap_handler   # 返回 next task 指针

    # a0 = next task

    # -------- 恢复 next --------
    mv t6, a0

    lw ra,   0(t6)
    lw sp,   4(t6)
    lw gp,   8(t6)
    lw tp,   12(t6)
    lw t0,   16(t6)
    lw t1,   20(t6)
    lw t2,   24(t6)
    lw s0,   28(t6)
    lw s1,   32(t6)
    lw a0,   36(t6)
    lw a1,   40(t6)
    lw a2,   44(t6)
    lw a3,   48(t6)
    lw a4,   52(t6)
    lw a5,   56(t6)
    lw a6,   60(t6)
    lw a7,   64(t6)
    lw s2,   68(t6)
    lw s3,   72(t6)
    lw s4,   76(t6)
    lw s5,   80(t6)
    lw s6,   84(t6)
    lw s7,   88(t6)
    lw s8,   92(t6)
    lw s9,   96(t6)
    lw s10,  100(t6)
    lw s11,  104(t6)
    lw t3,   108(t6)
    lw t4,   112(t6)

    lw t5, 124(t6)
    csrw mepc, t5

    lw t5,   116(t6)
    lw t6,   120(t6)

    mret

yield:
    li a7, 0
    ecall
    ret

# void start_first_task(TCB *task)
start_first_task:

    li t0, 0x80
    csrs mstatus, t0

    mv t6, a0

    lw ra,   0(t6)
    lw sp,   4(t6)
    lw gp,   8(t6)
    lw tp,   12(t6)
    lw t0,   16(t6)
    lw t1,   20(t6)
    lw t2,   24(t6)
    lw s0,   28(t6)
    lw s1,   32(t6)
    lw a0,   36(t6)
    lw a1,   40(t6)
    lw a2,   44(t6)
    lw a3,   48(t6)
    lw a4,   52(t6)
    lw a5,   56(t6)
    lw a6,   60(t6)
    lw a7,   64(t6)
    lw s2,   68(t6)
    lw s3,   72(t6)
    lw s4,   76(t6)
    lw s5,   80(t6)
    lw s6,   84(t6)
    lw s7,   88(t6)
    lw s8,   92(t6)
    lw s9,   96(t6)
    lw s10,  100(t6)
    lw s11,  104(t6)
    lw t3,   108(t6)
    lw t4,   112(t6)

    lw t5, 124(t6)
    csrw mepc, t5

    lw t5,   116(t6)
    lw t6,   120(t6)

    mret
