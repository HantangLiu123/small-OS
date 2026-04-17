.section .text
.global interrupt_init
.global trap_entry
.extern c_trap_handler
.extern tasks
.extern current_task
.extern active_tasks
.extern mult

.equ TASK_SIZE, 4

interrupt_init:
    la t0, trap_entry
    csrw mtvec, t0      # 设置中断入口
    li t0, 0x8          # mstatus.MIE
    csrs mstatus, t0
    li t0, 0x800        # mie.MEIE (外部中断，DE1-SoC Timer通常接此)
    csrs mie, t0
    ret

.align 4
trap_entry:
    addi sp, sp, -124
    sw ra, 116(sp)
    sw gp, 112(sp)
    sw tp, 108(sp)
    sw t0, 104(sp)
    sw t1, 100(sp)
    sw t2, 96(sp)
    sw t3, 92(sp)
    sw t4, 88(sp)
    sw t5, 84(sp)
    sw t6, 80(sp)
    sw s0, 76(sp)
    sw s1, 72(sp)
    sw s2, 68(sp)
    sw s3, 64(sp)
    sw s4, 60(sp)
    sw s5, 56(sp)
    sw s6, 52(sp)
    sw s7, 48(sp)
    sw s8, 44(sp)
    sw s9, 40(sp)
    sw s10, 36(sp)
    sw s11, 32(sp)
    sw a0, 28(sp)
    sw a1, 24(sp)
    sw a2, 20(sp)
    sw a3, 16(sp)
    sw a4, 12(sp)
    sw a5, 8(sp)
    sw a6, 4(sp)
    sw a7, 0(sp)

    csrr t0, mepc
    sw t0, 120(sp)

    call c_trap_handler

    beqz a0, trap_ret

    la s0, current_task
    li t0, (s0)
    la s1, active_tasks
    li t1, (s1)
    addi t0, t0, 1
    ble t0, t1, switch_task
    mv t0, zero
switch_task:
    sw t0, (s0)
    la s2, tasks
    li a0, TASK_SIZE
    mv a1, t0
    call mult
    add s2, s2, a0
    lw sp, (s2)

trap_ret:
    lw t0, 120(sp)
    csrw t0, mepc
    lw ra, 116(sp)
    lw gp, 112(sp)
    lw tp, 108(sp)
    lw t0, 104(sp)
    lw t1, 100(sp)
    lw t2, 96(sp)
    lw t3, 92(sp)
    lw t4, 88(sp)
    lw t5, 84(sp)
    lw t6, 80(sp)
    lw s0, 76(sp)
    lw s1, 72(sp)
    lw s2, 68(sp)
    lw s3, 64(sp)
    lw s4, 60(sp)
    lw s5, 56(sp)
    lw s6, 52(sp)
    lw s7, 48(sp)
    lw s8, 44(sp)
    lw s9, 40(sp)
    lw s10, 36(sp)
    lw s11, 32(sp)
    lw a0, 28(sp)
    lw a1, 24(sp)
    lw a2, 20(sp)
    lw a3, 16(sp)
    lw a4, 12(sp)
    lw a5, 8(sp)
    lw a6, 4(sp)
    lw a7, 0(sp)
    addi sp, sp, 124

    mret
    