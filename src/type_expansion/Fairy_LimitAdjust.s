
.thumb

.equ TypeCnt, 0x12
.equ NewTagStart, 0x20

.type THUMB_BRANCH_LINK_255_0x21CF574, %function

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

FULL_COPY_255_0x0219B908:
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
    .size FULL_COPY_255_0x0219B908, . - FULL_COPY_255_0x0219B908

FULL_COPY_265_0x219BADC:
    .word 4
    .size FULL_COPY_265_0x219BADC, . - FULL_COPY_265_0x219BADC

FULL_COPY_265_0x219BB1C:
    .word 0
    .size FULL_COPY_265_0x219BB1C, . - FULL_COPY_265_0x219BB1C

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

FULL_COPY_167_0x21A5A92:
    .byte TypeCnt, 0x2A, 0x08, 0xDA
    .size FULL_COPY_167_0x21A5A92, . - FULL_COPY_167_0x21A5A92

FULL_COPY_167_0x21AA9A6:
    .byte TypeCnt, 0x28, 0x02, 0xDB
    .size FULL_COPY_167_0x21AA9A6, . - FULL_COPY_167_0x21AA9A6

FULL_COPY_167_0x21BB004:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BB004, . - FULL_COPY_167_0x21BB004

FULL_COPY_167_0x21BB016:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BB016, . - FULL_COPY_167_0x21BB016

FULL_COPY_167_0x21BB022:
    .byte TypeCnt, 0x29, 0x06, 0xDB
    .byte TypeCnt, 0x28, 0x01, 0xDB
    .size FULL_COPY_167_0x21BB022, . - FULL_COPY_167_0x21BB022

FULL_COPY_167_0x21BB034:
    .byte TypeCnt, 0x28, 0x00, 0xDB
    .size FULL_COPY_167_0x21BB034, . - FULL_COPY_167_0x21BB034

FULL_COPY_167_0x21BB05A:
    .byte TypeCnt, 0x2C, 0x0E, 0xDA
    .size FULL_COPY_167_0x21BB05A, . - FULL_COPY_167_0x21BB05A

FULL_COPY_167_0x21BD1A0:
    .byte TypeCnt, 0x28, 0x01, 0xDA
    .byte TypeCnt, 0x29, 0x01, 0xDB
    .size FULL_COPY_167_0x21BD1A0, . - FULL_COPY_167_0x21BD1A0

FULL_COPY_167_0x21BD1AC:
    .byte TypeCnt, 0x22, 0x42, 0x43
    .size FULL_COPY_167_0x21BD1AC, . - FULL_COPY_167_0x21BD1AC

FULL_COPY_167_0x21BD318:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BD318, . - FULL_COPY_167_0x21BD318
	
@ Instances where type 17 is checked for type none
@ DoesMonHaveType - used for Conversion, Color Change
FULL_COPY_167_0x21BB026:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BB026, . - FULL_COPY_167_0x21BB026

@ Color Change
FULL_COPY_167_0x21BF862:
    .byte TypeCnt
    .size FULL_COPY_167_0x21BF862, . - FULL_COPY_167_0x21BF862

@ Conversion 2
FULL_COPY_167_0x21C9E70:
    .byte TypeCnt
    .size FULL_COPY_167_0x21C9E70, . - FULL_COPY_167_0x21C9E70
	
FULL_COPY_167_0x21C9E76:
    .byte TypeCnt
    .size FULL_COPY_167_0x21C9E76, . - FULL_COPY_167_0x21C9E76
	
@ BattleHandler_Transform - used for Transform, Imposter
FULL_COPY_167_0x21AE0B0:
    .byte TypeCnt
    .size FULL_COPY_167_0x21AE0B0, . - FULL_COPY_167_0x21AE0B0
