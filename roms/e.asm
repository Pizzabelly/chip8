CLS

LD V0, 0
LD V1, 0
LD V2, 0

loop:

LD   F, V0
LD   I, esprite
DRW V1, V2, 5
ADD V1, 5
ADD V2, 6

JP loop

esprite:
db  %11110000,
    %10000000,
    %11110000,
    %10000000,
    %11110000,
