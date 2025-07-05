# ------------ preset data ----------------------------
.word 100 , 0x0000000A      # MEM[100] = 10
.word 104 , 0x00000014      # MEM[104] = 20

# ------------ program -------------------------------
add $t0, $zero, $imm, 100   # $t0 = 100   (base address)
add $t1, $zero, $imm, 4     # $t1 =   4   (offset)

lw  $a0, $t0,  $zero        # $a0 = MEM[100] = 10
lw  $a1, $t0,  $t1          # $a1 = MEM[104] = 20

add $s0, $a0,  $a1          # $s0 = 30

add $s1, $zero, $imm, 108   # $s1 = 108   (store address)
sw  $s0, $s1,  $zero        # MEM[108] = 30

halt $zero, $zero, $zero, 0