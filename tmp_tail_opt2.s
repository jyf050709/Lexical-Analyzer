.text

.globl fact
fact:
	addi sp, sp, -160
	sw ra, 156(sp)
	sw s0, 152(sp)
	sw s1, 148(sp)
	addi s0, sp, 160
	sw a0, -16(s0)
	sw a1, -20(s0)
__tail_entry_fact:
	lw t1, -16(s0)
	li t2, 1
	slt t0, t2, t1
	xori t0, t0, 1
	mv s1, t0
	beqz t0, L0
	lw t0, -20(s0)
	mv a0, t0
	lw s1, 148(sp)
	lw ra, 156(sp)
	lw s0, 152(sp)
	addi sp, sp, 160
	ret
L0:
	lw t1, -16(s0)
	lw t2, -20(s0)
	mul t0, t1, t2
	mv s1, t0
	addi sp, sp, -8
	lw t1, -16(s0)
	li t2, 1
	sub t0, t1, t2
	sw t0, 0(sp)
	mv t0, s1
	sw t0, 4(sp)
	lw a0, 0(sp)
	sw a0, -16(s0)
	lw a1, 4(sp)
	sw a1, -20(s0)
	addi sp, sp, 8
	j __tail_entry_fact

.globl main
main:
	addi sp, sp, -144
	sw ra, 140(sp)
	sw s0, 136(sp)
	addi s0, sp, 144
__tail_entry_main:
	addi sp, sp, -8
	li t0, 5
	sw t0, 0(sp)
	li t0, 1
	sw t0, 4(sp)
	lw a0, 0(sp)
	lw a1, 4(sp)
	addi sp, sp, 8
	call fact
	mv t0, a0
	lw ra, 140(sp)
	lw s0, 136(sp)
	addi sp, sp, 144
	ret

