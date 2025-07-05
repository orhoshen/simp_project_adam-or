########################################################################
#  Test-7 :  Auto-disabling timer-IRQ after N ticks                    #
#           • timer fires every 3 cycles                               #
#           • ISR runs 4 times, then disables timer & irq0             #
#           • main program exits when $a0 == 4                         #
#                                                                      #
#  IO map (as in previous tests)                                       #
#   0  irq0enable        3  irq0status        6  irqhandler            #
#  11  timerenable      12  timercurrent     13  timermax              #
########################################################################

                # ---------- install ISR ----------
add  $s0, $zero, $imm, 6       # $s0 = IO addr irqhandler
add  $t0, $zero, $imm, isr     # $t0 = &isr
out  $t0,  $s0,   $zero        # irqhandler ← isr

                # ---------- arm timer ----------
add  $s1, $zero, $imm, 13      # $s1 = IO addr timermax
add  $t0, $zero, $imm, 3       # fire every 3 cycles
out  $t0,  $s1,   $zero        # timermax ← 3

add  $s2, $zero, $imm, 12      # $s2 = IO addr timercurrent
add  $t0, $zero, $imm, 0
out  $t0,  $s2,   $zero        # timercurrent ← 0

add  $a1, $zero, $imm, 11      # $a1 = IO addr timerenable
add  $t0, $zero, $imm, 1
out  $t0,  $a1,   $zero        # timerenable ← 1

                # ---------- enable IRQ0 ----------
add  $a2, $zero, $imm, 0       # $a2 = IO addr irq0enable
out  $t0,  $a2,   $zero        # irq0enable ← 1   (reuse  t0 = 1)

                # ---------- initialise counters ----------
add  $a0, $zero, $imm, 0       # tick counter (incremented in ISR)
add  $v0, $zero, $imm, 0       # main-loop cycle counter
add  $a3, $zero, $imm, 4       # exit value (4 ticks)

                # ---------- main loop ----------
main_loop:
add  $v0, $v0,   $imm, 1       # burn a cycle
beq  $a0,  $a3,  done          # leave after 4 ticks
bne  $zero,$zero, main_loop    # unconditional jump

done:
halt $zero, $zero, $zero, 0

########################################################################
#  Interrupt Service Routine  (address = ‘isr’)                        #
#    • ++$a0  each time                                                #
#    • when $a0 == 4 → disable timer & IRQ0                            #
#    • always clears irq0status                                        #
########################################################################
isr:
add  $a0, $a0,  $imm, 1        # ++tick counter

add  $t1, $zero, $imm, 4       # threshold = 4
beq  $a0, $t1, stop_timer      # if reached 4 → stop timer

continue_irq:
add  $t0, $zero, $imm, 3       # IO addr irq0status
add  $t2, $zero, $imm, 0
out  $t2,  $t0,  $zero         # irq0status ← 0
reti $zero, $zero, $zero, 0

stop_timer:
add  $t0, $zero, $imm, 11      # timerenable addr
add  $t2, $zero, $imm, 0
out  $t2, $t0,  $zero          # timerenable ← 0
add  $t0, $zero, $imm, 0       # irq0enable addr
out  $t2, $t0,  $zero          # irq0enable ← 0
add  $t0, $zero, $imm, 3       # irq0status addr
out  $t2, $t0,  $zero          # irq0status ← 0
reti $zero, $zero, $zero, 0
