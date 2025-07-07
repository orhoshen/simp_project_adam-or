# ===== Bubble-sort 16-word array at MEM[0x100-0x10F] =====
        # Registers
        # $s0  – base address (0x100)
        # $t0  – i  (outer loop counter 0..14)
        # $t1  – j  (inner loop counter 0..(14-i))
        # $t2  – scratch address/value
        # $s1  – array[j]
        # $s2  – array[j+1]
        # $a0  – constant 16
        # $a1  – constant 15

        add  $s0, $zero, $imm, 0x100    # base address 0x100
        add  $a0, $zero, $imm, 16       # constant 16  
        add  $t0, $zero, $imm, 0        # i = 0

LOOP1:  beq  $imm, $t0, $a0, END        # if i == 16 → END
        add  $t1, $zero, $zero, 0       # j = 0

LOOP2:  sub  $t2, $a0,  $t0, 1          # t2 = 16-i-1  (last j to process)
        beq  $imm, $t1, $t2, INC_I      # if j == t2 → finish inner loop

        # ---- compare array[j] and array[j+1] ----
        add  $t2, $s0,  $t1, 0          # addr = base + j
        lw   $s1,  $t2,  $zero, 0       # s1 = array[j]
        add  $t2, $t2,  $imm, 1         # addr = base + j + 1
        lw   $s2,  $t2,  $zero, 0       # s2 = array[j+1]

        blt  $imm, $s1, $s2, NO_SWAP    # if s1 < s2 → no swap (ascending)

        # ---- swap elements ----
        sw   $s1,  $t2,  $zero, 0       # array[j+1] = s1
        sub  $t2,  $t2,  $imm, 1        # addr = base + j
        sw   $s2,  $t2,  $zero, 0       # array[j]   = s2

NO_SWAP:
        add  $t1, $t1,  $imm, 1         # j++
        beq  $zero, $zero, $zero, LOOP2 # unconditional jump to LOOP2

INC_I:  add  $t0, $t0,  $imm, 1         # i++
        beq  $zero, $zero, $zero, LOOP1 # unconditional jump to LOOP1

END:    halt $zero, $zero, $zero, 0

        # ---------- test data ----------
        .word 256 7
        .word 257 16
        .word 258 4
        .word 259 8
        .word 260 20
        .word 261 1
        .word 262 6
        .word 263 2
        .word 264 3
        .word 265 8
        .word 266 5
        .word 267 4
        .word 268 11
        .word 269 9
        .word 270 30
        .word 271 25