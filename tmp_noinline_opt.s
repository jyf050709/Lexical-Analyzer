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
	sw s1, 4(sp)
	addi s0, sp, 16
	sw a0, -16(s0)
__tail_entry_g:
	lw t1, -16(s0)
	li t2, 31
	add t0, t1, t2
	mv s1, t0
	mv a0, t0
	lw s1, 4(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

.globl main
main:
	addi sp, sp, -48
	sw ra, 44(sp)
	sw s0, 40(sp)
	sw s1, 36(sp)
	addi s0, sp, 48
__tail_entry_main:
	addi sp, sp, -16
	li t0, 1
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 2
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 3
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 4
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 5
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 6
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 7
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 8
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	addi sp, sp, -16
	li t0, 9
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s1, t0
	mv a0, t0
	lw s1, 36(sp)
	lw ra, 44(sp)
	lw s0, 40(sp)
	addi sp, sp, 48
	ret

