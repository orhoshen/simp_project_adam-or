# verify .word handling only
.word 0x0     , 0xCAFEBABE    # first cell (hex address & data)
.word 10      , 0x0000ABCD    # decimal address, hex data
.word 4095    , 1             # last address in memory