.text

.globl loopTest
loopTest:
	addi sp, sp, -160
	sw s0, 152(sp)
	sw s1, 148(sp)
	sw s2, 144(sp)
	addi s0, sp, 160
	sw a0, -20(s0)
	li t0, 0
	mv s1, t0
	li t0, 0
	mv s2, t0
L0:
	mv t1, s2
	lw t2, -20(s0)
	slt t0, t1, t2
	beqz t0, L1
	mv t1, s1
	mv t2, s2
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
	lw s1, 148(sp)
	lw s2, 144(sp)
	lw s0, 152(sp)
	addi sp, sp, 160
	ret

.globl main
main:
	addi sp, sp, -160
	sw s0, 152(sp)
	sw s1, 148(sp)
	sw s2, 144(sp)
	addi s0, sp, 160
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
	lw s1, 148(sp)
	lw s2, 144(sp)
	lw s0, 152(sp)
	addi sp, sp, 160
	ret

