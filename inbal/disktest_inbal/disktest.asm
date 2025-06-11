	add $t0, $zero, $imm, 8					# $t0 will hold the number of the current sector being read (starting with 7, ending with 0) 
READ_SECTORS:
	jal $ra, $imm, $zero, POLL				# wait until diskstatus==0 (disk not busy)
	sub $t0, $t0, $imm, 1                   # update current sector number (initialized to 8, so we start with sector 7)
	out $t0, $zero, $imm, 15				# write current sector number to disksector register (starting with 7, ending with 0)
	add $t2, $zero, $imm, 1000				# set $t2 to 1000 (data will be tranferred from disk to this memory address) 
	out $t2, $zero, $imm, 16				# write 1000 to diskbuffer register 
	add $t2, $zero, $imm, 1					# set $t2 to 1
	out $t2, $zero, $imm, 14				# write 1 to diskcmd register (which will lead to the sector being read)
	add $s0, $zero, $imm, 128				# hold 128 in $s0 (number of lines in each sector)
	add $s1, $zero, $imm, 1000				# hold 1000 in $s1 (memory address to which we read current sector)
	add $s2, $zero, $imm, 1128				# hold 1128 in $s2 (memory address in which we will sum all sectors, and will be written to sector 8 in the end)
SUM_SECTORS:
	jal $ra, $imm, $zero, POLL				# wait until reading is complete 
	lw $t1, $zero, $s1, 0					# read current sector line to $t1 
	lw $t2, $zero, $s2, 0					# read current value of this line in sector 8 to $t2 
	add $t2, $t2, $t1, 0					# add the current sector's line to the previous sum of all relevant lines of previous sectors 
	sw $t2, $s2, $zero, 0					# update current sector 8 sum 
	sub $s0, $s0, $imm, 1					# update line number 
	add $s1, $s1, $imm, 1					# update sector 8 address 
	add $s2, $s2, $imm, 1					# update current sector address 
	bne $imm, $zero, $s0, SUM_SECTORS		# continue adding lines until all 128 lines have been added 
	bne $imm, $zero, $t0, READ_SECTORS		# once finished with all of the lines, move to next sector. if we finished sector 0, move to writing sector 8 
WRITE_SECTOR8:
	jal $ra, $imm, $zero, POLL				# wait until diskstatus==0 (disk not busy)
	add $t2, $zero, $imm, 8					# set $t2 to 8 (number of wanted sector)
	out $t2, $zero, $imm, 15				# write 8 to disksector register  
	add $t2, $zero, $imm, 1128				# set $t2 to 1128 (address in main memory where we prepared sector 8 data)
	out $t2, $zero, $imm, 16				# write 1128 to diskbuffer register
	add $t2, $zero, $imm, 2					# set $t2 to 2
	out $t2, $zero, $imm, 14				# write 2 to diskcmd register (which will lead to sector 8 being written)
	jal $ra, $imm, $zero, POLL				# wait until writing is complete 
	halt $zero, $zero, $zero, 0				# done -> exit program
POLL:
	in $t2, $zero, $imm, 17					# check disk status
	beq $ra, $zero, $t2, 0  				# if disk is free (diskstatus=0) -> continue with the code
	bne $imm, $zero, $t2, POLL				# if disk is busy -> continue polling 