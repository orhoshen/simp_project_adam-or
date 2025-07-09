# disk_ping.asm  –  issue one READ command, wait for it to finish, halt

        add  $t0, $zero, $imm, 0        # sector 0
        out  $t0, $zero, $zero, 15      # disksector = 0

        add  $t0, $zero, $imm, 1000     # buffer addr 1000
        out  $t0, $zero, $zero, 16      # diskbuffer = 1000

        add  $t0, $zero, $imm, 1        # diskcmd = 1 (read)
        out  $t0, $zero, $zero, 14      # issue command -> diskstatus set to 1 and dcnt=1024

        add  $t2, $zero, $imm, POLL     # t2 ← POLL addr
        jal  $ra,  $t2,   $zero, 0      # wait until diskstatus == 0

        halt $zero,$zero,$zero,0

# ---------- sub-routine ----------
POLL:
        in   $v0, $zero, $zero, 17      # v0 ← diskstatus
        bne  $ra, $v0,  $zero, 0        # if v0 != 0 jump to $ra (return)
        jal  $zero,$t2,  $zero, 0       # else loop