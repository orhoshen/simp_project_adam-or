# Test1: Basic ALU operations
add $t0, $imm, $imm, 5     # $t0 = 5
add $t1, $t0, $imm, 3      # $t1 = 8
sub $t2, $t1, $imm, 2      # $t2 = 6
mul $s0, $t2, $imm, 3      # $s0 = 18
and $s1, $s0, $imm, 0xF    # $s1 = 2 (18 & 0xF)
or  $s2, $s1, $imm, 0x20   # $s2 = 34 (2 | 32)
xor $t0, $s2, $imm, 0x10   # $t0 = 34 ^ 16 = 50
sll $t1, $t0, $imm, 1      # $t1 = 100
sra $t2, $t1, $imm, 2      # $t2 = 25 (signed shift right)
srl $s0, $t2, $imm, 1      # $s0 = 12
halt $zero, $zero, $zero, 0