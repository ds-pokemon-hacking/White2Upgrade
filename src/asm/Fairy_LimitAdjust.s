.type	THUMB_BRANCH_LINK_255_0x21CF574, %function

.thumb

.equ TypeCnt, 0x12
.equ NewTagStart, 0x20

@ BluRose's PC screen fix
FULL_COPY_255_0x21C2A50:
    .word 0xA700
    .size FULL_COPY_255_0x21C2A50, . - FULL_COPY_255_0x21C2A50

FULL_COPY_255_0x21D0846:
    cmp r4, #TypeCnt
    .size FULL_COPY_255_0x21D0846, . - FULL_COPY_255_0x21D0846

FULL_COPY_255_0x21D09AC:
    .word 0xA5BC
    .size FULL_COPY_255_0x21D09AC, . - FULL_COPY_255_0x21D09AC

FULL_COPY_255_0x21D0A00:
    add r1, #NewTagStart
    .size FULL_COPY_255_0x21D0A00, . - FULL_COPY_255_0x21D0A00

FULL_COPY_255_0x21D0A0A:
    cmp r4, #TypeCnt
    .size FULL_COPY_255_0x21D0A0A, . - FULL_COPY_255_0x21D0A0A

THUMB_BRANCH_LINK_255_0x21CF574:
    push {r2-r3, lr}
    ldr r1, =0xA0DC
    add r2, r1
    sub r2, r4
    ldr r3, =0xA268
    cmp r2, r3
    blt keep_r1_A0DC
    ldr r3, =0xA2AC
    cmp r2, r3
    bhi keep_r1_A0DC
    ldr r1, =0xA5BC-0x18C
keep_r1_A0DC:
    ldr r0, =0x9E94
    pop {r2-r3, pc}
   .size THUMB_BRANCH_LINK_255_0x21CF574, . - THUMB_BRANCH_LINK_255_0x21CF574
 
FULL_COPY_255_0x21BF2F0:
    mov r4, #NewTagStart
    .size FULL_COPY_255_0x21BF2F0, . - FULL_COPY_255_0x21BF2F0

FULL_COPY_255_0x21BF300:
    cmp r4, #(NewTagStart + TypeCnt)
    .size FULL_COPY_255_0x21BF300, . - FULL_COPY_255_0x21BF300

FULL_COPY_255_0x21D0A6E:
    mov r4, #NewTagStart
    .size FULL_COPY_255_0x21D0A6E, . - FULL_COPY_255_0x21D0A6E

FULL_COPY_255_0x21D0A7E:
    cmp r4, #(NewTagStart + TypeCnt)
    .size FULL_COPY_255_0x21D0A7E, . - FULL_COPY_255_0x21D0A7E

FULL_COPY_255_0x21D0A22:
    add r7, #NewTagStart
    .size FULL_COPY_255_0x21D0A22, . - FULL_COPY_255_0x21D0A22

FULL_COPY_255_0x21D0A4A:
    add r4, #NewTagStart
    .size FULL_COPY_255_0x21D0A4A, . - FULL_COPY_255_0x21D0A4A

@ BluRose's Hall of Fame fix
THUMB_BRANCH_LINK_265_0x2199FB4:
    LDR R1, =HOF_SPA_LUT
    bx lr
    .size FULL_COPY_265_0x2199FB4, . - FULL_COPY_265_0x2199FB4

FULL_COPY_265_0x219DB2C:
    .word 4
    .size FULL_COPY_265_0x219DB2C, . - FULL_COPY_265_0x219DB2C

FULL_COPY_265_0x219FD4C:
    .word 0
    .size FULL_COPY_265_0x219FD4C, . - FULL_COPY_265_0x219FD4C

@ Summary Screen Fixes?
FULL_COPY_207_0x021B5110:
    lsl r2, #0x11
    .size FULL_COPY_207_0x021B5110, . - FULL_COPY_207_0x021B5110
FULL_COPY_207_0x021B511A:
    mov r1, #0xB0
    .size FULL_COPY_207_0x021B511A, . - FULL_COPY_207_0x021B511A
FULL_COPY_207_0x021B5126:
    mov r2, #0xB0
    .size FULL_COPY_207_0x021B5126, . - FULL_COPY_207_0x021B5126
FULL_COPY_207_0x021B3A5A:
    cmp r4, #TypeCnt
    .size FULL_COPY_207_0x021B3A5A, . - FULL_COPY_207_0x021B3A5A
FULL_COPY_207_0x021B3A32:
    add r0, #(0x264 - 0x1A0)
    .size FULL_COPY_207_0x021B3A32, . - FULL_COPY_207_0x021B3A32
FULL_COPY_207_0x021B6BF0:
    add r1, r6, r0
    mov r0, #(0x264 >> 2)
    lsl r0, #2
    ldr r0, [r1, r0]
    .size FULL_COPY_207_0x021B6BF0, . - FULL_COPY_207_0x021B6BF0
FULL_COPY_207_0x021B6C5C:
    mov r0, #(0x264 >> 2)
    lsl r0, #2
    add r5, sp, #0x24
    nop // one extra instruction pog
    .size FULL_COPY_207_0x021B6C5C, . - FULL_COPY_207_0x021B6C5C
FULL_COPY_207_0x021B8EE0:
    mov r0, #(0x264 >> 2)
    lsl r0, #2
    .size FULL_COPY_207_0x021B8EE0, . - FULL_COPY_207_0x021B8EE0
FULL_COPY_207_0x021B6BE0:
    ldr  r0, [sp, #0x18]
    mov  r1, #0xaf      
    mov  r2, #0         
    mov  r4, #0xaf      
    .size FULL_COPY_207_0x021B6BE0, . - FULL_COPY_207_0x021B6BE0

FULL_COPY_207_0x021B8E68: // patch 3
    add  r5, r0, #0 
    add  r0, r4, #0 
    mov  r1, #0xaf  
    mov  r2, #0     
    .size FULL_COPY_207_0x021B8E68, . - FULL_COPY_207_0x021B8E68

FULL_COPY_207_0x021B8E78:
    lsl  r0, r5, #2
    add  r1, r6, r0
    mov r0, #(0x264 >> 2)
    lsl r0, #2
    .size FULL_COPY_207_0x021B8E78, . - FULL_COPY_207_0x021B8E78

FULL_COPY_207_0x021BA848:
    lsl  r0, r6, #2
    add  r1, r5, r0
    mov r0, #(0x264 >> 2)
    lsl r0, #2
    .size FULL_COPY_207_0x021BA848, . - FULL_COPY_207_0x021BA848

@  168 patch 
FULL_COPY_168_0x021F38E8:
    .word 572
    .size FULL_COPY_168_0x021F38E8, . - FULL_COPY_168_0x021F38E8

@ Type Table Adjustment
TypeEffectivenessLUT:
    // atk \ def                NORMAL,  FIGHT, FLYING, POISON, GROUND,   ROCK,   BUG,  GHOST,  STEEL,   FIRE,  WATER, GRASS,   ELECTRC, PSYCHIC, ICE, DRAGON,   DARK,  FAIRY
    .byte /* TYPE_NORMAL   */      4,      4,      4,      4,      4,      2,      4,      0,      2,      4,      4,      4,      4,      4,      4,      4,      4,      4
    .byte /* TYPE_FIGHTING */      8,      4,      2,      2,      4,      8,      2,      0,      8,      4,      4,      4,      4,      2,      8,      4,      8,      2
    .byte /* TYPE_FLYING   */      4,      8,      4,      4,      4,      2,      8,      4,      2,      4,      4,      8,      2,      4,      4,      4,      4,      4
    .byte /* TYPE_POISON   */      4,      4,      4,      2,      2,      2,      4,      2,      0,      4,      4,      8,      4,      4,      4,      4,      4,      8
    .byte /* TYPE_GROUND   */      4,      4,      0,      8,      4,      8,      2,      4,      8,      8,      4,      2,      8,      4,      4,      4,      4,      4
    .byte /* TYPE_ROCK     */      4,      2,      8,      4,      2,      4,      8,      4,      2,      8,      4,      4,      4,      4,      8,      4,      4,      4
    .byte /* TYPE_BUG      */      4,      2,      2,      2,      4,      4,      4,      2,      2,      2,      4,      8,      4,      8,      4,      4,      8,      2
    .byte /* TYPE_GHOST    */      0,      4,      4,      4,      4,      4,      4,      8,      4,      4,      4,      4,      4,      8,      4,      4,      2,      4
    .byte /* TYPE_STEEL    */      4,      4,      4,      4,      4,      8,      4,      4,      2,      2,      2,      4,      2,      4,      8,      4,      4,      8
    .byte /* TYPE_FIRE     */      4,      4,      4,      4,      4,      2,      8,      4,      8,      2,      2,      8,      4,      4,      8,      2,      4,      4
    .byte /* TYPE_WATER    */      4,      4,      4,      4,      8,      8,      4,      4,      4,      8,      2,      2,      4,      4,      4,      2,      4,      4
    .byte /* TYPE_GRASS    */      4,      4,      2,      2,      8,      8,      2,      4,      2,      2,      8,      2,      4,      4,      4,      2,      4,      4
    .byte /* TYPE_ELECTRIC */      4,      4,      8,      4,      0,      4,      4,      4,      4,      4,      8,      2,      2,      4,      4,      2,      4,      4
    .byte /* TYPE_PSYCHIC  */      4,      8,      4,      8,      4,      4,      4,      4,      2,      4,      4,      4,      4,      2,      4,      4,      0,      4
    .byte /* TYPE_ICE      */      4,      4,      8,      4,      8,      4,      4,      4,      2,      2,      2,      8,      4,      4,      2,      8,      4,      4
    .byte /* TYPE_DRAGON   */      4,      4,      4,      4,      4,      4,      4,      4,      2,      4,      4,      4,      4,      4,      4,      8,      4,      0
    .byte /* TYPE_DARK     */      4,      2,      4,      4,      4,      4,      4,      8,      4,      4,      4,      4,      4,      8,      4,      4,      2,      2
    .byte /* TYPE_FAIRY    */      4,      8,      4,      2,      4,      4,      4,      4,      2,      2,      4,      4,      4,      4,      4,      8,      8,      4
    .size TypeEffectivenessLUT, . - TypeEffectivenessLUT

FULL_COPY_167_0x21BD36C:
    .word TypeEffectivenessLUT
    .size FULL_COPY_167_0x21BD36C, . - FULL_COPY_167_0x21BD36C

FULL_COPY_167_0x21BD4A4:
    .word TypeEffectivenessLUT
    .size FULL_COPY_167_0x21BD4A4, . - FULL_COPY_167_0x21BD4A4

FULL_COPY_167_0x21A5C12:
    .byte TypeCnt, 0x2A, 0x08, 0xDA
    .size FULL_COPY_167_0x21A5C12, . - FULL_COPY_167_0x21A5C12

FULL_COPY_167_0x21AAB26:
    .byte TypeCnt, 0x28, 0x02, 0xDB
    .size FULL_COPY_167_0x21AAB26, . - FULL_COPY_167_0x21AAB26

FULL_COPY_167_0x21BB184:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BB184, . - FULL_COPY_167_0x21BB184

FULL_COPY_167_0x21BB196:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BB196, . - FULL_COPY_167_0x21BB196

FULL_COPY_167_0x21BB1A2:
    .byte TypeCnt, 0x29, 0x06, 0xDB
    .byte TypeCnt, 0x28, 0x01, 0xDB
    .size FULL_COPY_167_0x21BB1A2, . - FULL_COPY_167_0x21BB1A2

FULL_COPY_167_0x21BB1B4:
    .byte TypeCnt, 0x28, 0x00, 0xDB
    .size FULL_COPY_167_0x21BB1B4, . - FULL_COPY_167_0x21BB1B4

FULL_COPY_167_0x21BB1DA:
    .byte TypeCnt, 0x2C, 0x0E, 0xDA
    .size FULL_COPY_167_0x21BB1DA, . - FULL_COPY_167_0x21BB1DA

FULL_COPY_167_0x21BD320:
    .byte TypeCnt, 0x28, 0x01, 0xDA
    .byte TypeCnt, 0x29, 0x01, 0xDB
    .size FULL_COPY_167_0x21BD320, . - FULL_COPY_167_0x21BD320

FULL_COPY_167_0x21BD32C:
    .byte TypeCnt, 0x22, 0x42, 0x43
    .size FULL_COPY_167_0x21BD32C, . - FULL_COPY_167_0x21BD32C

FULL_COPY_167_0x21BD498:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BD498, . - FULL_COPY_167_0x21BD498

FULL_COPY_167_0x21BF9E2:
    .byte TypeCnt, 0x2E, 0x1E, 0xDA
    .size FULL_COPY_167_0x21BF9E2, . - FULL_COPY_167_0x21BF9E2

FULL_COPY_167_0x21C9FF0:
    .byte TypeCnt, 0x2C, 0x3C, 0xDA
    .size FULL_COPY_167_0x21C9FF0, . - FULL_COPY_167_0x21C9FF0

FULL_COPY_167_0x21C9FF6:
    .byte TypeCnt
    .size FULL_COPY_167_0x21C9FF6, . - FULL_COPY_167_0x21C9FF6

FULL_COPY_ARM9_0x20920C9:
    .byte 2
    .size FULL_COPY_ARM9_0x20920C9, . - FULL_COPY_ARM9_0x20920C9

HOF_SPA_LUT:
    @ TYPE_NORMAL   
    .word 0x2D
    @ TYPE_FIGHTING 
    .word 0x26
    @ TYPE_FLYING   
    .word 0x28
    @ TYPE_POISON   
    .word 0x2E
    @ TYPE_GROUND   
    .word 0x2B
    @ TYPE_ROCK     
    .word 0x30
    @ TYPE_BUG      
    .word 0x22
    @ TYPE_GHOST    
    .word 0x29
    @ TYPE_STEEL    
    .word 0x31
    @ TYPE_FIRE     
    .word 0x27
    @ TYPE_WATER    
    .word 0x32
    @ TYPE_GRASS    
    .word 0x2A
    @ TYPE_ELECTRIC 
    .word 0x25
    @ TYPE_PSYCHIC  
    .word 0x2F
    @ TYPE_ICE      
    .word 0x2C
    @ TYPE_DRAGON   
    .word 0x24
    @ TYPE_DARK     
    .word 0x23
    @ TYPE_FAIRY. TODO: Make SPA for this guy, currently loads the same as NORMAL.
    .word 0x2D 
.size HOF_SPA_LUT, . - HOF_SPA_LUT