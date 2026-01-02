.text

.globl loopTest
loopTest:
	addi sp, sp, -160
	sw ra, 156(sp)
	sw s0, 152(sp)
	addi s0, sp, 160
	sw a0, -12(s0)
	li t0, 0
	sw t0, -20(s0)
	li t0, 0
	sw t0, -24(s0)
L0:
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -12(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t0, t1
	beqz t0, L1
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -24(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -20(s0)
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
	j L0
L1:
	lw t0, -20(s0)
	mv a0, t0
	lw ra, 156(sp)
	lw s0, 152(sp)
	addi sp, sp, 160
	ret

.globl main
main:
	addi sp, sp, -144
	sw ra, 140(sp)
	sw s0, 136(sp)
	addi s0, sp, 144
	addi sp, sp, -4
	li t0, 100
	sw t0, 0(sp)
	lw a0, 0(sp)
	addi sp, sp, 4
	call loopTest
	mv t0, a0
	mv a0, t0
	lw ra, 140(sp)
	lw s0, 136(sp)
	addi sp, sp, 144
	ret

