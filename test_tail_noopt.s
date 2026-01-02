.text

.globl sum
sum:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	xori t0, t0, 1
	beqz t0, L0
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L0:
	addi sp, sp, -16
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	sw t0, 0(sp)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -12(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, 4(sp)
	lw a0, 0(sp)
	lw a1, 4(sp)
	call sum
	addi sp, sp, 16
	mv t0, a0
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl main
main:
	addi sp, sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	addi s0, sp, 16
	li a0, 10
	li a1, 0
	call sum
	mv t0, a0
	mv a0, t0
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

