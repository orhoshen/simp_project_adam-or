########################################################################
#  Test-8 : Nested IRQ – timer (IRQ0) + disk (IRQ1)                    #
#  Registers available:  t0-t2 , s0-s2 , a0-a3 , v0                    #
########################################################################
#  IO map (same as previous tests)
#   0 irq0enable      1 irq0status      2 irq1enable      3 irq1status
#   6 irqhandler
#   7 disksector      8 diskbuffer      9 diskcmd        10 diskstatus
#  11 timerenable    12 timercurrent   13 timermax
########################################################################

                # ---------- 1. install common IRQ handler ----------
add  $s0, $zero, $imm, 6       # $s0 = IO addr irqhandler
add  $t0, $zero, $imm, isr     # $t0 = &isr
out  $t0,  $s0,   $zero        # irqhandler ← isr

                # ---------- 2. set up the 2-tick timer ----------
add  $s1, $zero, $imm, 13      # $s1 = IO addr timermax
add  $t0, $zero, $imm, 2       # fire every 2 ticks
out  $t0,  $s1,   $zero        # timermax ← 2

add  $s2, $zero, $imm, 12      # $s2 = IO addr timercurrent
add  $t0, $zero, $imm, 0
out  $t0,  $s2,   $zero        # timercurrent ← 0

add  $t1, $zero, $imm, 11      # $t1 = IO addr timerenable
add  $t0, $zero, $imm, 1
out  $t0,  $t1,   $zero        # timerenable ← 1

                # ---------- 3. enable both IRQ0 and IRQ1 ----------
add  $t1, $zero, $imm, 0       # $t1 = IO addr irq0enable
out  $t0,  $t1,   $zero        # irq0enable ← 1   (reuse $t0 = 1)

add  $t1, $zero, $imm, 2       # $t1 = IO addr irq1enable
out  $t0,  $t1,   $zero        # irq1enable ← 1
s
                # ---------- 4. start a single disk READ (cmd = 2) ---
add  $t1, $zero, $imm, 7       # disksector
add  $t0, $zero, $imm, 0
out  $t0,  $t1,   $zero

add  $t1, $zero, $imm, 8       # diskbuffer
out  $t0,  $t1,   $zero

add  $t1, $zero, $imm, 9       # diskcmd
add  $t0, $zero, $imm, 2       # cmd = READ
out  $t0,  $t1,   $zero

                # ---------- 5. zero software counters --------------
add  $a0, $zero, $imm, 0       # timer-tick counter
add  $a1, $zero, $imm, 0       # disk-done counter
add  $v0, $zero, $imm, 0       # burn-cycles counter

                # ---------- 6. main wait-loop ----------------------
main_loop:
add  $v0, $v0,   $imm, 1       # spin
add  $t0, $zero, $imm, 1
bne  $a1,  $t0,  main_loop     # stay until exactly one disk IRQ happened
halt $zero, $zero, $zero, 0    # SUCCESS

########################################################################
#  Interrupt Service Routine (dispatcher)                              #
########################################################################
isr:
                # ---- check IRQ1 (disk) first (higher priority) -----
add  $t0, $zero, $imm, 3       # $t0 = IO addr irq1status
lw   $t1, $t0,   $zero         # $t1 ← irq1status
beq  $t1, $zero, check_timer

    # ----- disk IRQ body -----
add  $a1, $a1,   $imm, 1       # disk_done++
add  $t2, $zero, $imm, 0       # clear irq1status
out  $t2, $t0,   $zero
    # optionally disable IRQ1 so it fires only once
add  $t0, $zero, $imm, 2       # irq1enable
out  $t2, $t0,   $zero
reti  $zero, $zero, $zero, 0

check_timer:
                # ---- check IRQ0 (timer) ----------------------------
add  $t0, $zero, $imm, 1       # $t0 = IO addr irq0status
lw   $t1, $t0,   $zero
beq  $t1, $zero, isr_end       # neither IRQ? (shouldn’t happen)

    # ----- timer IRQ body -----
add  $a0, $a0,   $imm, 1       # tick++
add  $t2, $zero, $imm, 0
out  $t2, $t0,   $zero         # clear irq0status

isr_end:
reti  $zero, $zero, $zero, 0
