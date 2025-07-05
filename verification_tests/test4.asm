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
jal  subroutine, $zero, $zero        # call, link should be PC+2 in $ra
halt $zero,  $zero, $zero, 0         # expect to reach here and stop

          # ---- sub-routine -----------------------------
subroutine:
add  $v0,  $a0,   $a0                # double the argument
add  $t0,  $ra,   $imm, 0            # copy $ra so we can branch via JAL
jal  $t0,  $zero, $zero              # return: PC = $t0, link thrown away
                                      # (uses register-target form)
halt $zero,  $zero, $zero, 0         # <-- should **never** execute