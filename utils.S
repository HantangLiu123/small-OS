.global mult

mult:
    mv t0, zero
    mv t1, zero
    bgez a1, mloop
    neg a1, a1
    li t1, 1
mloop:
    beqz a1, mdone
    add t0, t0, a0
    addi a1, a1, -1
    j mloop
mdone:
    beqz t1, mult_ret
    neg t0, t0
mult_ret:
    mv a0, t0
    ret
