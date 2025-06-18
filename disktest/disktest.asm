#------------------------------------------------------------------------------
# disktest.asm
#Overview
#------------------------------------------------------------------------------
# Each sector = 128 words. We are interesting in only sectors 0-3
# for (i = 0; i < 128; i++): sector4[i] = sector0[i] + sector1[i] + sector2[i] + sector3[i]
# We would like to avoid reading each sector 128 times as this we take a long time so we decided to load each of the sectors
# We pick five non-overlapping buffers:
#   SECT0 → 1000-1127, SECT1 → 1128-1255, SECT2 → 1256-1383, SECT3 → 1384-1511, RESULT(SECT4) → 1512-1639
#------------------------------------------------------------------------------

    ### Constants ###
NUM_OF_SECTORS_TO_TEST = 4
NUM_LINES  = 128
SECT0_BUF  = 1000
SECT1_BUF  = 1128
SECT2_BUF  = 1256
SECT3_BUF  = 1384
RESULT_BUF = 1512

DISKCMD_REG_ADDR    = 14
DISKSECTOR_REG_ADDR = 15
DISKBUFFER_REG_ADDR = 16
DISKSTATUS_REG_ADDR = 17

DISK_CMD_IDLE = 0
DISK_CMD_READ = 1
DISK_CMD_WRITE = 2

        ### STAGE 1: read the sectors ###
    add   $s0, $zero, $imm, NUM_OF_SECTORS_TO_TEST  #s0 = 4 for loop of our sectors
    add   $s1, $zero, $imm, RESULT_BUF               #s1 = 1512 to loop the buffers of the address
READ_SECTORS_LOOP_LABEL:
    jal   $ra, $imm, $zero, POLL_LABEL                      # wait until diskstatus==0 (free)
    sub   $s0, $s0, $imm, 1                                 #preset the disk sector (and index)
    sub   $s1, $s1, $imm, 128                               #preset the buffer address   
    out   $s0, $zero, $imm, DISKSECTOR_REG_ADDR             # set disksector to current sector 3->2->1->0
    out   $s1, $zero, $imm, DISKBUFFER_REG_ADDR             # set diskbuffer to the correct sector buffer
    add   $t0, $zero, $imm, DISK_CMD_READ                   #preset t0 to 1 because diskcmd =1 is read mode
    out   $t0, $zero, $imm, DISKCMD_REG_ADDR                # set diskcmd to READ (1)
    jal   $ra, $imm, $zero, POLL_LABEL                      # wait until done
    bne   $imm, $s0, $zero, READ_SECTORS_LOOP_LABEL            # loop until s0 == 0


        ### STAGE 2: Sum the sectors ###  for i = 0…127 ⇒ RESULT[i] = SECT0[i] + SECT1[i] + SECT2[i] + SECT3[i]
#initialize    
    add   $s0, $zero, $imm, NUM_LINES  # $s0 = loop counter (128)
    add   $s1, $zero, $imm, SECT0_BUF  # pointer into sector0 buffer
    add   $s2, $zero, $imm, SECT1_BUF
    add   $s3, $zero, $imm, SECT2_BUF
    add   $s4, $zero, $imm, SECT3_BUF
    add   $s5, $zero, $imm, RESULT_BUF
SUM_LOOP_LABEL:
    lw    $t2, $zero, $s1, 0           # t2 = SECT0[i]
    lw    $t3, $zero, $s2, 0           # t3 = SECT1[i]
    add   $t2, $t2, $t3, 0              # t2 = SECT0[i]+SECT1[i]
    lw    $t3, $zero, $s3, 0           # t3 = SECT2[i]
    add   $t2, $t2, $t3, 0              #t2 = SECT0[i]+SECT1[i] + SECT2[i]
    lw    $t3, $zero, $s4, 0           # t3 = SECT3[i]
    add   $t2, $t2, $t3, 0              # t2 = sum of four sectors

    sw    $t2, $s5, $zero, 0           # RESULT[i] = t2

    sub   $s0, $s0, $imm, 1            # i++
    add   $s1, $s1, $imm, 1
    add   $s2, $s2, $imm, 1
    add   $s3, $s3, $imm, 1
    add   $s4, $s4, $imm, 1
    add   $s5, $s5, $imm, 1
    bne $imm, $s0, $zero, SUM_LOOP_LABEL   # while s0 != 0, repeat


    ### WRITE RESULT BUFFER to sector 4
WRITE_RESULT:
    jal   $ra, $imm, $zero, POLL_LABEL
    add   $t1, $zero, $imm, NUM_OF_SECTORS_TO_TEST    #preset sector number : write the result to sector 4
    out   $t1, $zero, $imm, DISKSECTOR_REG_ADDR        # set disksector ← 4
    add   $t1, $zero, $imm, RESULT_BUF                  #preset
    out   $t1, $zero, $imm, DISKBUFFER_REG_ADDR        # set diskbuffer ← 1512
    add   $t1, $zero, $imm, DISK_CMD_WRITE          #preset write mode
    out   $t1, $zero, $imm, DISKCMD_REG_ADDR        # set diskcmd ← WRITE (2)
    jal   $ra, $imm, $zero, POLL_LABEL              # wait until done

    halt  $zero, $zero, $zero, 0      # finished !

   ### POLL_LABEL: spin until diskstatus == 0
POLL_LABEL:
    in   $t6, $zero, $imm, DISKSTATUS_REG_ADDR   # t6 = diskstatus
    beq  $imm, $t6, $zero, POLL_RETURN_LABEL           # if 0, disk is free → return
    j    POLL_LABEL                              # else keep polling
POLL_RETURN_LABEL:
    jr   $ra                                      # back to caller
