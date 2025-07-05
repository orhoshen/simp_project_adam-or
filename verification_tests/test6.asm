########################################################################
#  Test-6 : Timer-IRQ (IRQ0) + irqhandler/irqreturn/reti               #
#  Registers available:  t0-t2 , s0-s2 , a0-a3 , v0                    #
########################################################################

                # ---------- IO addresses ----------
#   0 irq0enable     3 irq0status     6 irqhandler
#  11 timerenable   12 timercurrent  13 timermax

                # ---------- program IRQ vector ----------
add  $s0, $zero, $imm, 6       # $s0 = IO addr irqhandler
add  $t0, $zero, $imm, isr     # $t0 = &isr
out  $t0,  $s0,   $zero        # irqhandler ← isr

                # ---------- configure timer ----------
add  $s1, $zero, $imm, 13      # $s1 = IO addr timermax
add  $t0, $zero, $imm, 5       # fire after 5 ticks
out  $t0,  $s1,   $zero        # timermax ← 5

add  $s2, $zero, $imm, 12      # $s2 = IO addr timercurrent
add  $t0, $zero, $imm, 0
out  $t0,  $s2,   $zero        # timercurrent ← 0

add  $a1, $zero, $imm, 11      # $a1 = IO addr timerenable
add  $t0, $zero, $imm, 1
out  $t0,  $a1,   $zero        # timerenable ← 1

                # ---------- enable IRQ0 ----------
add  $a2, $zero, $imm, 0       # $a2 = IO addr irq0enable
out  $t0,  $a2,   $zero        # irq0enable ← 1   (reuse t0 = 1)

                # ---------- clear working regs ----------
add   $a0, $zero, $imm, 0        # $a0 = 0 (will be incremented by ISR)
add   $v0, $zero, $imm, 0        # debug / scratch
add   $a2, $zero, $imm, 11       # $a2 = 11  (loop-exit value)

                # ---------- main loop ----------
add  $v0, $v0,   $imm, 1       # burn a cycle
beq  $a0,  $a2,  done          # exit when $a0 == $a2  (beq works!)
bne  $zero,$zero, main_loop    # unconditional jump, keep spinning

main_loop:

done:
halt $zero, $zero, $zero, 0

########################################################################
#  Interrupt Service Routine  (address = ‘isr’)                        #
########################################################################
isr:
add  $a0, $a0,   $imm, 1       # $a0++ each timer tick

add  $a3, $zero, $imm, 3       # $a3 = IO addr irq0status
add  $t0, $zero, $imm, 0
out  $t0,  $a3,   $zero        # clear irq0status

reti $zero, $zero, $zero, 0
