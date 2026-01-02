.text

.globl f
f:
	addi sp, sp, -64
	sw s0, 56(sp)
	sw s1, 52(sp)
	sw s2, 48(sp)
	addi s0, sp, 64
	sw a0, -20(s0)
	sw a1, -24(s0)
	sw a2, -28(s0)
	sw a3, -32(s0)
	sw a4, -36(s0)
	sw a5, -40(s0)
	sw a6, -44(s0)
	sw a7, -48(s0)
	lw t0, 0(s0)
	sw t0, -52(s0)
	mv s2, a0
	lw s1, -52(s0)
__tail_entry_f:
	mv t1, s2
	mv t2, s1
	add t0, t1, t2
	mv a0, t0
	lw s1, 52(sp)
	lw s2, 48(sp)
	lw s0, 56(sp)
	addi sp, sp, 64
	ret

.globl main
main:
	addi sp, sp, -16
	sw s0, 8(sp)
	addi s0, sp, 16
__tail_entry_main:
	li t0, 10
	mv a0, t0
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

