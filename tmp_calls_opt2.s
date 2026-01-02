.text

.globl f
f:
	addi sp, sp, -48
	sw s0, 40(sp)
	addi s0, sp, 48
	sw a0, -12(s0)
	sw a1, -16(s0)
	sw a2, -20(s0)
	sw a3, -24(s0)
	sw a4, -28(s0)
	sw a5, -32(s0)
	sw a6, -36(s0)
	sw a7, -40(s0)
__tail_entry_f:
	lw t0, 0(s0)
	mv a0, t0
	lw s0, 40(sp)
	addi sp, sp, 48
	ret

.globl g
g:
	addi sp, sp, -16
	sw s0, 8(sp)
	addi s0, sp, 16
	sw a0, -12(s0)
__tail_entry_g:
	lw t1, -12(s0)
	li t2, 1
	add t0, t1, t2
	mv a0, t0
	lw s0, 8(sp)
	addi sp, sp, 16
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

