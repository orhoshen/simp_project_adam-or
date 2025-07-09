
#------------------------------------------------------------------------------
# rectangle.asm
#
# Memory:
#   [0x100] = A = top-left pixel offset
#   [0x101] = B = bottom-left pixel offset
#   [0x102] = C = bottom-right pixel offset
#   [0x103] = D = top-right pixel offset
#
# Screen is 256×256; memory starts at frame buffer and is 256*256 words of 8 bits.
# I/O reg:
   #MONITOR_ADDR_REG = 20   #pixel address in frame buffer
   #MONITOR_DATA_REG = 21    # pixel luminance value, White = 0xFF luminance.
   #MONITOR_CMD_REG  = 22   #(write pixel when set to 1)
# 
#------------------------------------------------------------------------------

_start:
    # Stage 1: Load corner offsets
    add  $t0, $zero, $imm, 0x100
    lw   $s0, $t0,   $zero          # $s0 ← A
    add  $t0, $zero, $imm, 0x101
    lw   $s1, $t0,   $zero          # $s1 ← B
    add  $t0, $zero, $imm, 0x103
    lw   $a0, $t0,   $zero          # $a0 ← D

    # Stage 2: Compute width (x_axis, D-A) and length (y_axis, (B-A)/256)
    sub  $s2, $a0, $s0              # $s2 = width   (stable)
    add  $s2, $s2, $imm, 1          #inclusive borders
    sub  $t0, $s1, $s0              # $t0 = (B−A)
    srl  $v0, $t0, $imm, 8          # $v0 = height  (stable)
    add  $v0, $v0, $imm, 1          $inclusive borders

    #Stage 2.1: load of monitor IOs
    add  $a0, $zero, $imm, 20      # $a0 = MONITOR_ADDR_REG reg $a0 is repurposed as D is no longer needed
    add  $a3, $zero, $imm, 21      # $a3 = MONITOR_DATA_REG
    add  $t2, $zero, $imm, 22      # $t2 = MONITOR_CMD_REG

    # STAGE 3: nested loop the rectangle, outer loop is rows, inner loop is columns
    add  $a1, $zero, $s0            # $a1 = row base offset (A)
    add  $s1, $zero, $zero          # $s1 = row counter = 0

ROW_LOOP:
    add  $a2, $zero, $zero        # column counter is $a2 = 0 so that pixel address is $a1 (row) + $a2 (column) 

COL_LOOP:
    # pixel_offset = row_base + col
    add  $t0, $a1, $a2, $zero

    # draw white pixel at offset t0

    out  $t0, $zero, $imm, 20       # monitoraddr  ← pixel offset
    add  $t1, $zero, $imm, 0xFF     # $t1 = 0xFF
    out  $t1, $zero, $imm, 21       # monitordata ← 0xFF
    add  $t1, $zero, $imm, 1        # $t1 = 1
    out  $t1, $zero, $imm, 22       # monitorcmd  ← 1

    # advance column
    add  $a2, $a2, $imm, 1
    bne  $imm, $a2, $s2, COL_LOOP

    # advance to next row
    add  $a1, $a1, $imm, 256        # row_base += 256
    add  $s1, $s1, $imm, 1          # row_ctr  += 1
    bne  $imm, $s1, $v0, ROW_LOOP          

    #Stage 4: finish  all done
    halt $zero, $zero, $zero, 0

    .word  0x100 , 0   # A 
    .word  0x101 , 0x500   # B 
    .word  0x102 , 0x505   # C 
    .word  0x103 , 5   # D 