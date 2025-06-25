#------------------------------------------------------------------------------
# disktest.asm
# Purpose:
#   Sum the contents of disk sectors 0-3 and write the result to sector 4.
#   Uses DMA disk read/write and memory-mapped I/O.
#------------------------------------------------------------------------------

# Constants (commented for reference)
# SECT0_BUF = 1000
# SECT1_BUF = 1128
# SECT2_BUF = 1256
# SECT3_BUF = 1384
# RESULT_BUF = 1512

# Disk I/O Register Indices
# diskcmd    = 14
# disksector = 15
# diskbuffer = 16
# diskstatus = 17

# STAGE 1: Load sectors 0–3 into memory buffers
# Using: $s0 = sector index (0–3), $s1 = buffer ptr, $t0 = constant 1, $t1 = sector #
#        $t2 = poll scratch

    add   $s0, $zero, $imm, 4         # sector counter = 4
    add   $s1, $zero, $imm, 1512      # start of RESULT_BUF + 128*0

READ_SECTORS_LOOP:
    jal   $ra, $imm, $zero, POLL     # wait for disk to be free

    sub   $s0, $s0, $imm, 1          # s0 = s0 - 1 → sector number
    sub   $s1, $s1, $imm, 128        # s1 = s1 - 128 → current buffer addr

    out   $s0, $zero, $imm, 15       # disksector = s0
    out   $s1, $zero, $imm, 16       # diskbuffer = s1

    add   $t0, $zero, $imm, 1        # diskcmd = 1 (read)
    out   $t0, $zero, $imm, 14       # issue read command

    jal   $ra, $imm, $zero, POLL     # wait for disk to complete

    bne   $imm, $s0, $zero, READ_SECTORS_LOOP  # repeat if more sectors to load

# STAGE 2: Sum the buffers into RESULT_BUF
# Using: $s0 = loop counter, $s1–$s2 = pointers to SECT0–SECT1, $a0–$a1 = pointers to SECT2–SECT3, $gp = result buffer
#        $t0–$t2 = working registers

    add   $s0, $zero, $imm, 128      # loop counter = 128 words
    add   $s1, $zero, $imm, 1000     # s1 = SECT0_BUF
    add   $s2, $zero, $imm, 1128     # s2 = SECT1_BUF
    add   $a0, $zero, $imm, 1256     # a0 = SECT2_BUF
    add   $a1, $zero, $imm, 1384     # a1 = SECT3_BUF
    add   $gp, $zero, $imm, 1512     # gp = RESULT_BUF

SUM_LOOP:
    lw    $t0, $zero, $s1, 0         # t0 = mem[s1]
    lw    $t1, $zero, $s2, 0         # t1 = mem[s2]
    add   $t0, $t0, $t1, 0           # t0 += t1

    lw    $t1, $zero, $a0, 0         # t1 = mem[a0]
    add   $t0, $t0, $t1, 0           # t0 += t1

    lw    $t1, $zero, $a1, 0         # t1 = mem[a1]
    add   $t0, $t0, $t1, 0           # t0 += t1

    sw    $t0, $gp, $zero, 0         # store to result buffer

    sub   $s0, $s0, $imm, 1
    add   $s1, $s1, $imm, 1
    add   $s2, $s2, $imm, 1
    add   $a0, $a0, $imm, 1
    add   $a1, $a1, $imm, 1
    add   $gp, $gp, $imm, 1

    bne   $imm, $s0, $zero, SUM_LOOP

# STAGE 3: Write result to sector 4
WRITE_RESULT:
    jal   $ra, $imm, $zero, POLL

    add   $t0, $zero, $imm, 4        # sector 4
    out   $t0, $zero, $imm, 15

    add   $t0, $zero, $imm, 1512     # result buffer
    out   $t0, $zero, $imm, 16

    add   $t0, $zero, $imm, 2        # diskcmd = 2 (write)
    out   $t0, $zero, $imm, 14

    jal   $ra, $imm, $zero, POLL

    halt  $zero, $zero, $zero, 0     # done

# POLL: Wait until diskstatus == 0
POLL:
    in    $t2, $zero, $imm, 17       # diskstatus
    beq   $imm, $t2, $zero, POLL_DONE
    beq   $imm, $zero, $zero, POLL
POLL_DONE:
    jal $zero, $ra, $zero, 0