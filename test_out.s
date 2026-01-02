.text

.globl abs
abs:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t0, t1
	beqz t0, L0
	li t0, 0
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -12(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L0:
	lw t0, -12(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl max
max:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	beqz t0, L2
	lw t0, -12(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
	j L3
L2:
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L3:

.globl min
min:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t0, t1
	beqz t0, L4
	lw t0, -12(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L4:
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl fact
fact:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	xori t0, t0, 1
	beqz t0, L6
	li t0, 1
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L6:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	sw t0, 0(sp)
	lw a0, 0(sp)
	call fact
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	mul t0, t0, t1
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl fib
fib:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	xori t0, t0, 1
	beqz t0, L8
	lw t0, -12(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L8:
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
	lw a0, 0(sp)
	call fib
	addi sp, sp, 16
	mv t0, a0
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 2
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	sw t0, 0(sp)
	lw a0, 0(sp)
	call fib
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl gcd
gcd:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
L10:
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	snez t0, t0
	beqz t0, L11
	lw t0, -16(s0)
	sw t0, -24(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	rem t0, t0, t1
	sw t0, -16(s0)
	lw t0, -24(s0)
	sw t0, -12(s0)
	j L10
L11:
	lw t0, -12(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl test_operators
test_operators:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
	li t0, 0
	sw t0, -24(s0)
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 15
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L12
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L12:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 5
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L14
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 2
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L14:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	mul t0, t0, t1
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 50
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L16
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 4
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L16:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	div t0, t0, t1
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 2
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L18
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 8
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L18:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	rem t0, t0, t1
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L20
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 16
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L20:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	beqz t0, L22
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 32
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L22:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t0, t1
	xori t0, t0, 1
	beqz t0, L24
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 64
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L24:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t0, t1
	seqz t0, t0
	beqz t0, L26
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 128
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L26:
	lw t0, -12(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -16(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	snez t0, t0
	beqz t0, L28
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 256
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
L28:
	lw t0, -24(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl test_logic
test_logic:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	sw a0, -12(s0)
	sw a1, -16(s0)
	sw a2, -20(s0)
	lw t0, -12(s0)
	beqz t0, L32
	lw t0, -16(s0)
	beqz t0, L32
	li t0, 1
	j L33
L32:
	li t0, 0
L33:
	beqz t0, L30
	lw t0, -16(s0)
	bnez t0, L36
	lw t0, -20(s0)
	bnez t0, L36
	li t0, 0
	j L37
L36:
	li t0, 1
L37:
	beqz t0, L34
	li t0, 1
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L34:
L30:
	lw t0, -12(s0)
	seqz t0, t0
	bnez t0, L40
	lw t0, -16(s0)
	beqz t0, L42
	lw t0, -20(s0)
	beqz t0, L42
	li t0, 1
	j L43
L42:
	li t0, 0
L43:
	bnez t0, L40
	li t0, 0
	j L41
L40:
	li t0, 1
L41:
	beqz t0, L38
	li t0, 2
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret
L38:
	li t0, 0
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl test_loop
test_loop:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	li t0, 0
	sw t0, -16(s0)
	li t0, 1
	sw t0, -20(s0)
L44:
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 10
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	slt t0, t1, t0
	xori t0, t0, 1
	beqz t0, L45
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 5
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L46
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -20(s0)
	j L44
L46:
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 8
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	sub t0, t0, t1
	seqz t0, t0
	beqz t0, L48
	j L45
L48:
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	lw t0, -20(s0)
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -20(s0)
	j L44
L45:
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl test_nested
test_nested:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	li t0, 1
	sw t0, -16(s0)
	li t0, 2
	sw t0, -20(s0)
	li t0, 3
	sw t0, -24(s0)
	lw t0, -24(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 1
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -24(s0)
	lw t0, -20(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	li t0, 10
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -20(s0)
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

.globl main
main:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	addi s0, sp, 32
	li t0, 0
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li t0, 5
	neg t0, t0
	sw t0, 0(sp)
	lw a0, 0(sp)
	call abs
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 3
	li a1, 7
	call max
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 3
	li a1, 7
	call min
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 5
	call fact
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 7
	call fib
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 48
	li a1, 18
	call gcd
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 10
	li a1, 5
	call test_operators
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	li a0, 1
	li a1, 1
	li a2, 0
	call test_logic
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	call test_loop
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	addi sp, sp, -4
	sw t0, 0(sp)
	addi sp, sp, -12
	call test_nested
	addi sp, sp, 12
	mv t0, a0
	mv t1, t0
	lw t0, 0(sp)
	addi sp, sp, 4
	add t0, t0, t1
	sw t0, -16(s0)
	lw t0, -16(s0)
	mv a0, t0
	lw ra, 28(sp)
	lw s0, 24(sp)
	addi sp, sp, 32
	ret

