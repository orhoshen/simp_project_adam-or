	add $t2, $zero, $imm, 1				# $t2 = 1
	sll $sp, $t2, $imm, 11				# set $sp = 1 << 11 = 2048
	lw $a0, $zero, $imm, 256	 		# load n to $a0
	lw $a1, $zero, $imm, 257 			# load k to $a1 
	jal $ra, $imm, $zero, BINOM			# calculate Newton's binom 
	sw $v0, $zero, $imm, 258			# when done, write the result to memory address 0x102 
	halt $zero, $zero, $zero, 0			# exit program 
BINOM: 
	sub $sp, $sp, $imm, 4	 			# decrease stack pointer by 3 to allocate space for return address, arguments and $s0
	sw $ra, $sp, $zero, 0				# store return address
	sw $a0, $sp, $imm, 1				# store first argument - n 
	sw $a1, $sp, $imm, 2				# store second argument - k
	sw $s0, $sp, $imm, 3				# store $s0 (saved register, used to store binom(n-1,k) result)
	beq $imm, $a1, $zero, LEAF			# if k==0 (first stop condition) -> return 1 and exit function  
	beq $imm, $a1, $a0, LEAF			# if n==k (second stop condition) -> return 1 and exit function 
	sub $a0, $a0, $imm, 1				# otherwise, prepare arguments for first recursive call binom(n-1,k-1) -> n = n-1
	sub $a1, $a1, $imm, 1				# prepare arguments for first recursive call binom(n-1,k-1) -> k = k-1
	jal $ra, $imm, $zero, BINOM			# call first recursive function - binom(n-1,k-1)
	add $s0, $v0, $zero, 0				# store binom(n-1,k-1) in $s0 
	add $a1, $a1, $imm, 1				# prepare arguments for second recursive call binom(n-1,k) -> k-1+1 = k 
	jal $ra, $imm, $zero, BINOM			# call second recursive function - binom(n-1,k)
	add $v0, $s0, $v0, 0				# add store binom(n-1,k) to $v0, so that $v0 = binom(n-1,k) + binom(n-1,k-1)
	beq $imm, $zero, $zero, RETURN		# exit function call
LEAF:
	add $v0, $zero, $imm, 1				# return 1 and exit function call (continue to RETURN)
RETURN: 
	lw $s0, $sp, $imm, 3				# restore $s0 from stack
	lw $a1, $sp, $imm, 2				# restore $a1 from stack (original k)
	lw $a0, $sp, $imm, 1				# restore $a0 from stack (original n)
	lw $ra, $sp, $zero, 0				# restore return address from stack 
	add $sp, $sp, $imm, 4				# release allocated space 
	beq $ra, $zero, $zero, 0			# return to caller 
	.word 256 12
	.word 257 4 
