        # factorial.asm – compute n! for n at MEM[0x101], write result to MEM[0x100]

        lw   $t0, $zero, $imm, 0x101    # $t0 ← MEM[0x101] (load n)                                      [oai_citation:0‡isaproject_040425.pdf](file-service://file-XuwYRykdeUnyzpiZJzYrnp)
        add  $t1, $zero, $imm, 1        # $t1 ← 1        (initialize product)

Loop:   beq  $t0, $zero, Done         # if (n == 0) goto Done
        mul  $t1, $t1, $t0              # product ← product * n
        sub  $t0, $t0, $imm, 1          # n ← n − 1
        beq  $zero, $zero, Loop         # unconditional jump back to Loop

Done:   sw   $t1, $zero, $imm, 0x100   # MEM[0x100] ← product (store result)                        [oai_citation:1‡isaproject_040425.pdf](file-service://file-XuwYRykdeUnyzpiZJzYrnp)
        halt $zero, $zero, $zero, 0     # end execution