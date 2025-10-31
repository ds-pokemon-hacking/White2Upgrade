.thumb

.equ PkmnCnt, 721
.equ RegionalDexPkmCnt, 302
.equ RegionalDexFile, 826

.equ ALWAYS_HAVE_NATL_DEX, 1

.type THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x166, %function
.type THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x124, %function
.type THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x18E, %function
.type THUMB_BRANCH_LINK_Pokedex_ReadMonName_0x14, %function
.type THUMB_BRANCH_LINK_Pokedex_ReadMonEntry_0x24, %function

@ when clicking on dex in the menu: 302, 301
@ sliding animation: 299, 139
@ blah blah unnecessary
@ habitat: 304, 139

@ ARM9 Patches...
@ Not sure what this is...?
FULL_COPY_sub_200C124_0xA0:
    .word PkmnCnt
    .size FULL_COPY_sub_200C124_0xA0, . - FULL_COPY_sub_200C124_0xA0

FULL_COPY_PML_PkmDecryptCheck_0x32:
    nop
    .size FULL_COPY_PML_PkmDecryptCheck_0x32, . - FULL_COPY_PML_PkmDecryptCheck_0x32

@ Enable PersonalData load of "egg indexes"
FULL_COPY_PML_PersonalGetDatID_0x5C:
    .word 0xFFFF - PkmnCnt 
    .size FULL_COPY_PML_PersonalGetDatID_0x5C, . - FULL_COPY_PML_PersonalGetDatID_0x5C

@ Expand evolution methods...
@ Add 6 bytes to file size
FULL_COPY_CheckEvolveSpecies_0xAC:
    movs r1, #0x30
    .size FULL_COPY_CheckEvolveSpecies_0xAC, . - FULL_COPY_CheckEvolveSpecies_0xAC

@ Read 1 more entry
FULL_COPY_CheckEvolveSpecies_0x33E:
    cmp r0, #8
    .size FULL_COPY_CheckEvolveSpecies_0x33E, . - FULL_COPY_CheckEvolveSpecies_0x33E

FULL_COPY_CheckEvolveSpecies_0x36E:
    cmp r0, #8
    .size FULL_COPY_CheckEvolveSpecies_0x36E, . - FULL_COPY_CheckEvolveSpecies_0x36E

FULL_COPY_CheckEvolveSpecies_0x3EC:
    cmp r0, #8
    .size FULL_COPY_CheckEvolveSpecies_0x3EC, . - FULL_COPY_CheckEvolveSpecies_0x3EC
    
FULL_COPY_CheckEvolveSpecies_0x44C:
    cmp r0, #8
    .size FULL_COPY_CheckEvolveSpecies_0x44C, . - FULL_COPY_CheckEvolveSpecies_0x44C

@ Overlay 299 Patches
@ Expand the space allocated by the process.

@ Patch the render and init functions.
FULL_COPY_Pokedex_CreateProc_0x12:
    movs r6, #0x1B
    lsl r6, r6, #8
    .size FULL_COPY_Pokedex_CreateProc_0x12, . - FULL_COPY_Pokedex_CreateProc_0x12

@ Adjust limiters
FULL_COPY_Pokedex_BuildPkmEntry_0x1BC:
    .word PkmnCnt
    .size FULL_COPY_Pokedex_BuildPkmEntry_0x1BC, . - FULL_COPY_Pokedex_BuildPkmEntry_0x1BC

FULL_COPY_Pokedex_ReadMonName_0x28:
    .word PkmnCnt
    .size FULL_COPY_Pokedex_ReadMonName_0x28, . - FULL_COPY_Pokedex_ReadMonName_0x28

@ Call GetMonName
THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x124:
    add r1, r5, r1
    push {r0}
    mov r0, #0xC7
    lsl r0, r0, #0x4
    add r1, r1, r0
    pop {r0}
    str r0, [r1, #0x4]
    bx lr
    .size THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x124, . - THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x124

THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x166:
    add r1, r5, r1
    push {r0}
    mov r0, #0xC7
    lsl r0, r0, #0x4
    add r1, r1, r0
    pop {r0}
    str r0, [r1, #0x4]
    bx lr
    .size THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x166, . - THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x166

THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x18E:
    add r1, r5, r1
    push {r0}
    mov r0, #0xC7
    lsl r0, r0, #0x4
    add r1, r1, r0
    pop {r0}
    str r0, [r1, #0x4]
    bx lr
    .size THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x18E, . - THUMB_BRANCH_LINK_Pokedex_BuildPkmEntry_0x18E

@ Call ReadMonName
THUMB_BRANCH_LINK_Pokedex_ReadMonName_0x14:
    add r0, r5, r0
    push {r1}
    mov r1, #0xC7
    lsl r1, r1, #0x4
    add r0, r1, r0
    mov r1, r6
    pop {r1}
    bx lr
    .size THUMB_BRANCH_LINK_Pokedex_ReadMonName_0x14, . - THUMB_BRANCH_LINK_Pokedex_ReadMonName_0x14

@ Call ReadMon
THUMB_BRANCH_LINK_Pokedex_ReadMonEntry_0x24:
    mov r0, #0xC7
    lsl r0, r0, #0x4
    add r2, r0, r2
    ldr r2, [r2, #0x4]
    mov r0, r5
    bx lr
    .size THUMB_BRANCH_LINK_Pokedex_ReadMonEntry_0x24, . - THUMB_BRANCH_LINK_Pokedex_ReadMonEntry_0x24

@ Search Engine Limiters
FULL_COPY_302_0x021AE244:
    .word PkmnCnt
    .size FULL_COPY_302_0x021AE244, . - FULL_COPY_302_0x021AE244

FULL_COPY_302_0x021AE310:
    .word 2 * PkmnCnt
    .size FULL_COPY_302_0x021AE310, . - FULL_COPY_302_0x021AE310

FULL_COPY_302_0x021AE318:
    .word PkmnCnt
    .size FULL_COPY_302_0x021AE318, . - FULL_COPY_302_0x021AE318

FULL_COPY_302_0x021AE37C: @ a sort of species limiter
    .word 2 * PkmnCnt
    .size FULL_COPY_302_0x021AE37C, . - FULL_COPY_302_0x021AE37C

FULL_COPY_302_0x021AE384:
    .word PkmnCnt
    .size FULL_COPY_302_0x021AE384, . - FULL_COPY_302_0x021AE384
