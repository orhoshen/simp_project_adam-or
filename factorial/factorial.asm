        # factorial.asm – compute n! for n at MEM[0x100], write result to MEM[0x101]

        lw   $t0, $zero, $imm, 0x100    # $t0 ← MEM[0x100] (load n)                                      [oai_citation:0‡isaproject_040425.pdf](file-service://file-XuwYRykdeUnyzpiZJzYrnp)
        add  $t1, $zero, $imm, 1        # $t1 ← 1        (initialize product= 1)

Loop:   beq  $t0, $zero, Done         # if (n == 0) goto Done
        mul  $t1, $t1, $t0              # product ← product * n
        sub  $t0, $t0, $imm, 1          # n ← n − 1
        beq  $zero, $zero, Loop         # unconditional jump back to Loop

Done:   sw   $t1, $zero, $imm, 0x101   # MEM[0x101] ← product (store result)                        [oai_citation:1‡isaproject_040425.pdf](file-service://file-XuwYRykdeUnyzpiZJzYrnp)
        halt $zero, $zero, $zero, 0     # end execution

        # put the test-value 5 at 0x100, and maybe clear 0x101
        .word 0x100 8
        .word 0x101 0