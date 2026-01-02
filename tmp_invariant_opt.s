.text

.globl g
g:
	addi sp, sp, -32
	sw s0, 24(sp)
	sw s1, 20(sp)
	addi s0, sp, 32
	sw a0, -16(s0)
	mv s1, a0
__tail_entry_g:
	mv t1, s1
	li t2, 31
	add t0, t1, t2
	mv s1, t0
	mv a0, t0
	lw s1, 20(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl foo
foo:
	addi sp, sp, -48
	sw ra, 44(sp)
	sw s0, 40(sp)
	sw s1, 36(sp)
	sw s2, 32(sp)
	sw s3, 28(sp)
	addi s0, sp, 48
	sw a0, -24(s0)
	mv s1, a0
__tail_entry_foo:
	addi sp, sp, -16
	mv t0, s1
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s3, t0
	li t0, 0
	mv s2, t0
	li t0, 0
	mv s1, t0
L0:
	mv t1, s2
	mv t2, s3
	slt t0, t1, t2
	beqz t0, L1
	mv t1, s1
	li t2, 1
	add t0, t1, t2
	mv s1, t0
	mv t1, s2
	li t2, 1
	add t0, t1, t2
	mv s2, t0
	j L0
L1:
	mv t0, s1
	mv a0, t0
	lw s1, 36(sp)
	lw s2, 32(sp)
	lw s3, 28(sp)
	lw ra, 44(sp)
	lw s0, 40(sp)
	addi sp, sp, 48
	ret

.globl main
main:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	sw s3, 12(sp)
	addi s0, sp, 32
__tail_entry_main:
	addi sp, sp, -16
	li t0, 100
	sw t0, 0(sp)
	lw a0, 0(sp)
	call g
	addi sp, sp, 16
	mv t0, a0
	mv s3, t0
	li t0, 0
	mv s2, t0
	li t0, 0
	mv s1, t0
L2:
	mv t1, s2
	mv t2, s3
	slt t0, t1, t2
	beqz t0, L3
	mv t1, s1
	li t2, 1
	add t0, t1, t2
	mv s1, t0
	mv t1, s2
	li t2, 1
	add t0, t1, t2
	mv s2, t0
	j L2
L3:
	mv t0, s1
	mv a0, t0
	lw s1, 20(sp)
	lw s2, 16(sp)
	lw s3, 12(sp)
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

