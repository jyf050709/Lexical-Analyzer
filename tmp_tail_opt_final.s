.text

.globl fact
fact:
	addi sp, sp, -48
	sw ra, 44(sp)
	sw s0, 40(sp)
	sw s1, 36(sp)
	sw s2, 32(sp)
	sw s3, 28(sp)
	addi s0, sp, 48
	sw a0, -24(s0)
	sw a1, -28(s0)
	mv s1, a0
	mv s2, a1
__tail_entry_fact:
	mv t1, s1
	li t2, 1
	slt t0, t2, t1
	xori t0, t0, 1
	mv s3, t0
	beqz t0, L0
	mv t0, s2
	mv a0, t0
	lw s1, 36(sp)
	lw s2, 32(sp)
	lw s3, 28(sp)
	lw ra, 44(sp)
	lw s0, 40(sp)
	addi sp, sp, 48
	ret
L0:
	mv t1, s1
	mv t2, s2
	mul t0, t1, t2
	mv s2, t0
	addi sp, sp, -8
	mv t1, s1
	li t2, 1
	sub t0, t1, t2
	sw t0, 0(sp)
	mv t0, s2
	sw t0, 4(sp)
	lw a0, 0(sp)
	sw a0, -24(s0)
	mv s1, a0
	lw a1, 4(sp)
	sw a1, -28(s0)
	mv s2, a1
	addi sp, sp, 8
	j __tail_entry_fact

.globl main
main:
	addi sp, sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	addi s0, sp, 16
__tail_entry_main:
	addi sp, sp, -16
	li t0, 5
	sw t0, 0(sp)
	li t0, 1
	sw t0, 4(sp)
	lw a0, 0(sp)
	lw a1, 4(sp)
	call fact
	addi sp, sp, 16
	mv t0, a0
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

