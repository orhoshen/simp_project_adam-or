
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
   MONITOR_ADDR_REG = 20   #pixel address in frame buffer
   MONITOR_DATA_REG = 21    # pixel luminance value, White = 0xFF luminance.
   MONITOR_CMD_REG  = 22   #(write pixel when set to 1)
# 
#------------------------------------------------------------------------------
_start:
    # Stage 1: Load corner offsets
    lw   $s0, $zero, $imm, 0x100    # $s0 ← A
    lw   $s1, $zero, $imm, 0x101    # $s1 ← B
    lw   $s2, $zero, $imm, 0x102    # $s2 ← C
    lw   $s3, $zero, $imm, 0x103    # $s3 ← D

    # Stage 2: Compute width (x_axis, D-A) and length (y_axis, (B-A)/256)
    sub  $t0, $s3, $s0              # $t0 = width = D - A
    sub  $t1, $s1, $s0              # $t1 = (B−A) = height*256
    add  $t2, $zero, $imm, 8        # shift amount = 8
    srl  $t1, $t1, $t2              # $t1 = height

    # STAGE 3: nested loop the rectangle, outer loop is rows, inner loop is columns
    add  $s4, $zero, $s0            # $s4 = current row base offset (starts at A)

ROW_LOOP:
    add  $s5, $zero, $imm, 0        # column counter is $s5 = 0 so that pixel address is $s4 (row) + $s5 (column) 

COL_LOOP:
    # pixel_offset = row_base + col
    add  $t3, $s4, $s5

    # draw white pixel at offset t3
    out  $t3, $zero, $imm, MONITOR_ADDR_REG         # monitoraddr ← t3
    add  $t4, $zero, $imm, 0xFF                     # luminance = 255
    out  $t4, $zero, $imm, MONITOR_DATA_REG       # monitordata ← 0xFF
    add  $t5, $zero, $imm, 1                    # preset monitorcmd = 1
    out  $t5, $zero, $imm, MONITOR_CMD_REG       # set monitorcmd to 1

    # advance column
    add  $s5, $s5, $imm, 1
    bne  $imm, $s5, $t0, COL_LOOP         # repeat until col == width

    # advance to next row
    add  $s4, $s4, $imm, 256        # row_base += 256
    sub  $t1, $t1, $imm, 1          # height--
    bne  $imm, $t1, $zero, ROW_LOOP       # repeat until row == height

    #Stage 4: finish  all done
    halt $zero, $zero, $zero, 0
