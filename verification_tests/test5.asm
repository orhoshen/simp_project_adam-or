########################################################################
#  Test-5: DISK DMA round-trip (WRITE + READ)                          #
#                                                                      #
#  Memory map of I/O registers (from spec):                            #
#     14  diskcmd      – 1 = READ  , 2 = WRITE                         #
#     15  disksector   – sector number (0-127)                         #
#     16  diskbuffer   – base address in MEM                           #
#     17  diskstatus   – 0 = idle , 1 = busy                           #
########################################################################

                # ---------- preset data ----------
.word 0 , 0xAABBCCDD       # MEM[0] = AABBCCDD

                # ---------- constants -----------
add  $s0, $zero, $imm, 14  # $s0 = diskcmd
add  $s1, $zero, $imm, 15  # $s1 = disksector
add  $s2, $zero, $imm, 16  # $s2 = diskbuffer
add  $a1, $zero, $imm, 17  # $a1 = diskstatus   (use $a1—R4—for 4th const)

                # ---------- set sector & buffer ------
add  $t0, $zero, $imm, 0   # sector = 0
out  $t0,  $s1,   $zero    # disksector ← 0

add  $t1, $zero, $imm, 0   # buffer address = 0
out  $t1,  $s2,   $zero    # diskbuffer ← 0

                # ---------- issue WRITE -------------
add  $t2, $zero, $imm, 2   # diskcmd = 2 (WRITE)
out  $t2,  $s0,   $zero

                # ---------- wait_busy ---------------
wait_busy:
in   $a0,  $a1,   $zero    # $a0 ← diskstatus
bne  $a0,  $zero, wait_busy

                # ---------- clobber MEM[0] ----------
add  $a2, $zero, $imm, 0   # temp = 0
sw   $a2,  $zero, $zero    # MEM[0] ← 0

                # ---------- issue READ --------------
add  $t2, $zero, $imm, 1   # diskcmd = 1 (READ)
out  $t2,  $s0,   $zero

                # ---------- wait_busy again ----------
wait_busy2:
in   $a0,  $a1,   $zero
bne  $a0,  $zero, wait_busy2

                # ---------- verify -------------------
lw   $v0,  $zero, $zero    # $v0 should reload AABBCCDD

halt $zero, $zero, $zero, 0
