	lw $a0, $zero, $imm, 256	         # load A coordinates from memory address 256 to $a0 ($a0 will hold A.x, $t0 will hold A.y)
	lw $a1, $zero, $imm, 257		     # load B coordinates from memory address 257 to $a1 ($a0 will hold B.x(=A.x), $t1 will hold B.y)
	lw $a2, $zero, $imm, 258			 # load C coordinates from memory address 258 to $a2 ($a2 will hold C.x, $t1 will hold C.y(=B.y))
	srl $t0, $a0, $imm, 8			     # shift A by 8 (equivalent to modulo256 which will give us the pixel's row number) in order to get A.y and keep in $t0
	srl $t1, $a1, $imm, 8				 # shift B by 8 (equivalent to modulo256 which will give us the pixel's row number) in order to get B.y=C.y and keep in $t1
	and $a0, $a0, $imm, 0xFF		     # mask 8 MSBs of A to only keep the pixel's column in order to get A.x=B.x (shift relative to start of line)
	and $a2, $a2, $imm, 0xFF			 # mask 8 MSBs of C to only keep the pixel's column in order to get C.x (shift relative to start of line)
	add $t2, $a0, $zero, 0			     # current pixel coordinates are kept in regs $t2 (x coordinate) and $s2 (y coordinate). first pixel is A
	add $s2, $t0, $zero, 0				 # current pixel coordinates are kept in regs $t2 (x coordinate) and $s2 (y coordinate). first pixel is A
	sub $s0, $t1, $t0, 0				 # calculate C.y-A.y and keep in $s0, for diagonal calculations 
	sub $s1, $a2, $a0, 0				 # calculate C.x-A.x and keep in $s1, for diagonal calculations 
FETCH_PXL:
	jal $ra, $imm, $zero, COLOR_PXL		 # color current pixel (inside of triangle) white
	sub $a1, $s2, $t0, 0				 # $a1 holds y - A.y
	mul $a1, $a1, $s1, 0				 # $a1 holds (y-A.y)*(C.x-A.x)
	sub $a3, $t2, $a0, 0				 # $a3 holds x - A.x
	mul $a3, $a3, $s0, 0				 # $a3 holds (x-A.x)*(C.y-A.y)
	ble $imm, $a3, $a1, FETCH_PXL		 # if (y-A.y)*(C.x-A.x) <= (x-A.x)*(C.y-A.y) the pixel is inside the triangle -> color it 
	add $s2, $s2, $imm, 1				 # otherwise, move to next row (column is updated in COLOR_PXL)
	add $t2, $a0, $imm, 0			  	 # in the next row, we can directly start from the pixel in A's column
	beq $zero, $zero, $imm, FETCH_PXL    # directly write the pixel in the next row (we will not go out of bounds because once we finish with C pixel we exit)
COLOR_PXL: 
	mul $a1, $s2, $imm, 256				 # get original row in frame buffer with formula 256y 
	add	$a1, $a1, $t2, 0				 # $a1 holds original pixel address in frame buffer with formula 256y+x
	out $a1, $zero, $imm, 20			 # write the pixel address to monitoraddr reg 
	add $a1, $zero, $imm, 255			 # hold 255 (represents white)
	out $a1, $zero, $imm, 21			 # write the wanted color to monitordata reg
	add $a1, $zero, $imm, 1				 # hold 1 (represents writing)
	out $a1, $zero, $imm, 22			 # send writing command to monitor by writing 1 to monitorcmd reg
	add $t2, $t2, $imm, 1				 # enlarge pixel.x by 1 in order to move to next column
	bgt $imm, $t2, $a2, EXIT			 # if current_pixel.x > C.x we finished the triangle -> exit program
	beq $ra, $zero, $zero, 0			 # otherwise, fetch the next pixel
EXIT:
	halt $zero, $zero, $zero, 0		     # otherwise, we finished the triangle -> exit program 
.word 256 0      # 3								
.word 257 65280  # 20995							
.word 258 65536  # 21105								