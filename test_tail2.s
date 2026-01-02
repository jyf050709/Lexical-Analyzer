.text

.globl helper
helper:
	addi sp, sp, -32
	sw s0, 24(sp)
	sw s1, 20(sp)
	addi s0, sp, 32
	sw a0, -16(s0)
	mv s1, a0
__tail_entry_helper:
	mv t0, s1
	slli t0, t0, 1
	mv a0, t0
	lw s1, 20(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl tail_complex
tail_complex:
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
__tail_entry_tail_complex:
	mv t1, s1
	li t2, 0
	slt t0, t2, t1
	xori t0, t0, 1
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
	li t0, 0
	mv s3, t0
	sw t0, -36(s0)
	mv t0, s1
	slli t0, t0, 1
	mv s3, t0
	sw t0, -36(s0)
	addi sp, sp, -8
	mv t1, s1
	li t2, 1
	sub t0, t1, t2
	sw t0, 0(sp)
	mv t1, s2
	mv t2, s3
	add t0, t1, t2
	sw t0, 4(sp)
	lw a0, 0(sp)
	sw a0, -24(s0)
	mv s1, a0
	lw a1, 4(sp)
	sw a1, -28(s0)
	mv s2, a1
	addi sp, sp, 8
	j __tail_entry_tail_complex

.globl main
main:
	addi sp, sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	addi s0, sp, 16
__tail_entry_main:
	li a0, 5
	li a1, 0
	call tail_complex
	mv t0, a0
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

