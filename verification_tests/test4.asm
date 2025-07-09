# ------------------------------------------------------
# Test4: JAL (jump-and-link) + big immediate path
# Goal:
#   1. Force the assembler to emit a 32-bit immediate word
#      (value is far outside –128…127).
#   2. Verify that JAL stores the correct link in $ra
#      and that control returns from the sub-routine.
#   3. Program stops at the final HALT if everything works.
# ------------------------------------------------------

          # ---- main ------------------------------------
add  $a0,  $zero, $imm, 0x12345678   # large const → bigimm
jal  $ra, $imm, $zero, subroutine   # rd=$ra, rs=$imm ⇒ bigimm=1, next word=addr(subroutine)
halt $zero,  $zero, $zero, 0         # expect to reach here and stop

          # ---- sub-routine -----------------------------
subroutine:
add  $v0,  $a0,   $a0                # double the argument
add  $t0,  $ra,   $imm, 0            # copy $ra so we can branch via JAL
jal  $zero, $t0, $zero, 0          # rd=$zero discards link, rs=$t0 is target, imm=0
#jal  $t0,  $zero, $zero              # return: PC = $t0, link thrown away
                                      # (uses register-target form)
halt $zero,  $zero, $zero, 0         # <-- should **never** execute