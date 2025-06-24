.word 0x0, 0xAAAAAAAA     # fill some memory
.word 0x10, 0xBBBBBBBB

beq $zero, $zero, target  # should branch to label 'target'

.word 0x20, 0xCCCCCCCC    # padding between instructions

target: 
add $t0, $t1, $t2         # label should resolve to this instruction's PC