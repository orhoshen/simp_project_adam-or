# ===== Bubble-sort 16-word array at MEM[0x100-0x10F] =====
# Registers in use
#   $s0  – base address (0x100)
#   $t0  – i  (outer loop counter 0-14)
#   $t1  – j  (inner loop counter 0-(14-i))
#   $t2  – scratch (limit / address)
#   $s1  – array[j]
#   $s2  – array[j+1]
#   $a0  – constant 15   (highest valid index)

        add  $s0, $zero, $imm, 0x100     # base address 0x100
        add  $a0, $zero, $imm, 15
		add  $a1, $zero, $imm, 9         # $a1 = 9  (I/O reg leds)        # constant 15
        add  $t0, $zero, $imm, 0         # i = 0

# ---------- outer loop ------------------------------------------------
LOOP1:  beq  $imm, $t0,  $a0,  END       # if i == 15 ⟹ END
        add  $t1, $zero, $imm, 0         # j = 0

# ---------- show current i on LEDs (IO[9]) ----------------------------
        out  $t0,  $a1,  $zero, 0        # leds = i		

# ---------- inner loop ------------------------------------------------
LOOP2:
        # compute current upper-bound  (limit = 14 – i)
        sub  $t2,  $a0,  $t0, 0          # t2 = 15 – i
        sub  $t2,  $t2,  $imm, 1         # t2 = t2 – 1  → 14 – i
        beq  $imm, $t1,  $t2,  INC_I     # if j == limit ⟹ outer++ 

        # ---- load A[j] and A[j+1] ----
        add  $t2,  $s0,  $t1, 0          # t2 = base + j
        lw   $s1,  $t2,  $zero, 0        # s1 = A[j]
        add  $t2,  $t2,  $imm, 1         # t2 = base + j + 1
        lw   $s2,  $t2,  $zero, 0        # s2 = A[j+1]

        # ---- compare & swap if needed (ascending order) ----
        blt  $imm, $s1,  $s2,  NO_SWAP   # if A[j] < A[j+1] → already OK
        sw   $s1,  $t2,  $zero, 0        # A[j+1] = s1
        sub  $t2,  $t2,  $imm, 1         # t2 = base + j
        sw   $s2,  $t2,  $zero, 0        # A[j]   = s2

NO_SWAP:
        add  $t1,  $t1,  $imm, 1         # j++
        beq  $imm, $zero, $zero, LOOP2   # unconditional jump (always taken)

# ---------- advance outer counter ------------------------------------
INC_I:  add  $t0,  $t0,  $imm, 1         # i++
        beq  $imm, $zero, $zero, LOOP1   # unconditional jump back to LOOP1

# ---------- finished --------------------------------------------------
END:    
		add  $t2, $zero, $imm, 10           # $t2 = 10  (I/O reg display7seg)
       	add  $a0, $zero, $imm, 0xDEADBEEF   # pattern 0xDEADBEEF
       	out  $a0,  $t2,  $zero, 0           # display7seg = pattern
		halt $zero, $zero, $zero, 0

# ---------- test data (MEM[0x100-0x10F]) -----------------------------
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
