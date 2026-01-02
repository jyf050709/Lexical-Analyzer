.text

.globl exprTest
exprTest:
	addi sp, sp, -32
	sw s0, 24(sp)
	sw s1, 20(sp)
	addi s0, sp, 32
	sw a0, -16(s0)
	sw a1, -20(s0)
	sw a2, -24(s0)
__tail_entry_exprTest:
	lw t0, -16(s0)
	lw t1, -20(s0)
	add t0, t0, t1
	lw t1, -24(s0)
	lw t2, -16(s0)
	sub t1, t1, t2
	mul t0, t0, t1
	mv s1, t0
	lw t0, -20(s0)
	lw t1, -24(s0)
	mul t0, t0, t1
	mv t1, s1
	add t0, t1, t0
	mv s1, t0
	mv a0, t0
	lw s1, 20(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl main
main:
	addi sp, sp, -16
	sw s0, 8(sp)
	addi s0, sp, 16
__tail_entry_main:
	li t0, 34
	mv a0, t0
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

