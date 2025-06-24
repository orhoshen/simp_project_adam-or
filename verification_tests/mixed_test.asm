# ------------ scatter a few .word values ------------
.word 0x0   , 0xAAAAAAA1      # mem[0]  = AAAA…
.word 0x10  , 0xBBBBBBB2      # mem[16] = BBBB…
.word 0x30  , 0xCCCCCCC3      # mem[48] = CCCC…

# ------------ start of code stream ------------------
start:
beq  $zero, $zero, target     # PC 49 → big-imm to label ‘target’

add  $t0  , $t1   , $t2       # PC 51 — ordinary 1-word R-type

jal  comeback                  # PC 52 → big-imm backwards

halt                           # PC 54 — 1-word, ends code

# ------------ branch / jal destinations -------------
target:                        # should resolve to PC 55
add   $s0 , $s1 , $s2          # dummy op

comeback:
add   $v0 , $zero, $zero       # will execute after jal
