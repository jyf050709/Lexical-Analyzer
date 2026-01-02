.text

.globl exprTest
exprTest:
	addi sp, sp, -160
	sw s0, 152(sp)
	sw s1, 148(sp)
	addi s0, sp, 160
	sw a0, -16(s0)
	sw a1, -20(s0)
	sw a2, -24(s0)
__tail_entry_exprTest:
	lw t1, -16(s0)
	lw t2, -20(s0)
	add t0, t1, t2
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t1, -24(s0)
	lw t2, -16(s0)
	sub t0, t1, t2
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	mul t0, t0, t1
	mv s1, t0
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t1, -20(s0)
	lw t2, -24(s0)
	mul t0, t1, t2
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	mv s1, t0
	mv a0, t0
	lw s1, 148(sp)
	lw s0, 152(sp)
	addi sp, sp, 160
	ret

.globl main
main:
	addi sp, sp, -144
	sw s0, 136(sp)
	addi s0, sp, 144
__tail_entry_main:
	li t0, 34
	mv a0, t0
	lw s0, 136(sp)
	addi sp, sp, 144
	ret

