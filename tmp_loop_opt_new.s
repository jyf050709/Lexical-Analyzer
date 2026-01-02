.text

.globl loopTest
loopTest:
	addi sp, sp, -32
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	sw s3, 12(sp)
	addi s0, sp, 32
	sw a0, -24(s0)
	mv s2, a0
__tail_entry_loopTest:
	li t0, 0
	mv s1, t0
	li t0, 0
	mv s3, t0
L0:
	mv t1, s3
	mv t2, s2
	slt t0, t1, t2
	beqz t0, L1
	mv t1, s1
	mv t2, s3
	add t0, t1, t2
	mv s1, t0
	mv t1, s3
	li t2, 1
	add t0, t1, t2
	mv s3, t0
	j L0
L1:
	mv t0, s1
	mv a0, t0
	lw s1, 20(sp)
	lw s2, 16(sp)
	lw s3, 12(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl main
main:
	addi sp, sp, -32
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	addi s0, sp, 32
__tail_entry_main:
	li t0, 0
	mv s1, t0
	li t0, 0
	mv s2, t0
L2:
	mv t1, s2
	li t2, 100
	slt t0, t1, t2
	beqz t0, L3
	mv t1, s1
	mv t2, s2
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
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

