# -------- initialise ---------------------------------
add $t0, $zero, $imm, 0     # i   = 0
add $t1, $zero, $imm, 5     # limit = 5
add $s0, $zero, $imm, 0     # sum = 0

# -------- loop body ----------------------------------
loop:
beq $t0, $t1, done          # if i == 5 → exit
add $s0, $s0,  $t0          # sum += i
add $t0, $t0,  $imm, 1      # i++
bne $zero, $zero, loop      # unconditional jump
done:
halt $zero, $zero, $zero, 0