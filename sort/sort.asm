	add $s0, $zero, $imm, 256			# initialize $s0 as array's base address 0x100 ('d256) 
	add $t0, $zero, $imm, 0             # initialize $t0 as outer loop counter: i == 0
	add $a0, $zero, $imm, 16			# keep 16 in $a0
LOOP1: 
	beq $imm, $t0, $a0, END             # if i = 16, exit (outer loop executed 16 times)
	add $t1, $zero, $zero, 0			# initialize $t1 as inner loop counter: j == 0
LOOP2:
	sub $t2, $imm, $t0, 15			    # j should run from i to 16-i-1
	beq $imm, $t1, $t2, INC_I           # if inner loop is done, increment i and go to outer loop 
	add $t2, $s0, $t1, 0                # calculate address of array[j]
	lw $s1, $t2, $zero, 0               # read array[j] to $s1
	add $t2, $t2, $imm, 1               # calculate address of array[j+1]
	lw $s2, $t2, $zero, 0               # read array[j+1] to $s2
	blt $imm, $s1, $s2, NO_SWAP         # if array[j]<array[j+1], no need to swap, continue with the inner loop
	sw $s1, $t2, $zero, 0               # swap: put $s1 (array[j]) in array[j+1] 
	sub $t2, $t2, $imm, 1               # calculate address of array[j]
	sw $s2, $t2, $zero, 0               # put $s2 (array[j+1]) in array[j]
NO_SWAP:
	add $t1, $t1, $imm, 1               # j += 1
	beq $imm, $zero, $zero, LOOP2       # go to next inner loop 
INC_I:
	add $t0, $t0, $imm, 1               # i += 1
	beq $imm, $zero, $zero, LOOP1       # go to next outer loop 
END:
	halt $zero, $zero, $zero, 0         # sorting done - exit program 
	.word 256 7
	.word 257 16
	.word 258 4
	.word 259 8
	.word 260 20
	.word 261 1
	.word 262 6
	.word 263 2
	.word 264 3
	.word 265 8
	.word 266 5
	.word 267 4
	.word 268 11
	.word 269 9
	.word 270 30
	.word 271 25
	