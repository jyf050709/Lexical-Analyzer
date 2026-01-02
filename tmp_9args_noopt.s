.text

.globl f
f:
	addi sp, sp, -48
	sw ra, 44(sp)
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
	lw t0, 0(s0)
	sw t0, -44(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -44(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	mv a0, t0
	lw ra, 44(sp)
	lw s0, 40(sp)
	addi sp, sp, 48
	ret

.globl main
main:
	addi sp, sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	addi s0, sp, 16
	addi sp, sp, -48
	li t0, 1
	sw t0, 4(sp)
	li t0, 2
	sw t0, 8(sp)
	li t0, 3
	sw t0, 12(sp)
	li t0, 4
	sw t0, 16(sp)
	li t0, 5
	sw t0, 20(sp)
	li t0, 6
	sw t0, 24(sp)
	li t0, 7
	sw t0, 28(sp)
	li t0, 8
	sw t0, 32(sp)
	li t0, 9
	sw t0, 36(sp)
	lw a0, 4(sp)
	lw a1, 8(sp)
	lw a2, 12(sp)
	lw a3, 16(sp)
	lw a4, 20(sp)
	lw a5, 24(sp)
	lw a6, 28(sp)
	lw a7, 32(sp)
	lw t0, 36(sp)
	sw t0, 0(sp)
	call f
	addi sp, sp, 48
	mv t0, a0
	mv a0, t0
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

