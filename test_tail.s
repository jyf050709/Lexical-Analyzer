.text

.globl sum
sum:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	addi s0, sp, 32
	sw a0, -20(s0)
	sw a1, -24(s0)
	mv s1, a0
	mv s2, a1
__tail_entry_sum:
	mv t1, s1
	li t2, 0
	slt t0, t2, t1
	xori t0, t0, 1
	beqz t0, L0
	mv t0, s2
	mv a0, t0
	lw s1, 20(sp)
	lw s2, 16(sp)
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L0:
	addi sp, sp, -8
	mv t1, s1
	li t2, 1
	sub t0, t1, t2
	sw t0, 0(sp)
	mv t1, s2
	mv t2, s1
	add t0, t1, t2
	sw t0, 4(sp)
	lw a0, 0(sp)
	sw a0, -20(s0)
	mv s1, a0
	lw a1, 4(sp)
	sw a1, -24(s0)
	mv s2, a1
	addi sp, sp, 8
	j __tail_entry_sum

.globl main
main:
	addi sp, sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	addi s0, sp, 16
__tail_entry_main:
	li a0, 10
	li a1, 0
	call sum
	mv t0, a0
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret

