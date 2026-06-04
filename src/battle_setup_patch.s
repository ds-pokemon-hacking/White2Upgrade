.thumb

.type THUMB_BRANCH_LINK_BtlSetup_SetWildNormal_0x3A, %function

@ BtlSetup_SetWildNormal keeps the player party at +0x24. Later battle setup
@ code also expects the battler party pointer at +0x34; after expansion work this
@ field can still contain player-info data, which is then parsed as a Pokemon.
THUMB_BRANCH_LINK_BtlSetup_SetWildNormal_0x3A:
    ldr r1, [r4, #0x24]
    str r1, [r4, #0x34]

    @ Replay the two instructions replaced by this branch-link patch.
    ldr r0, [sp, #0x20]
    movs r7, #0
    bx lr

    .size THUMB_BRANCH_LINK_BtlSetup_SetWildNormal_0x3A, . - THUMB_BRANCH_LINK_BtlSetup_SetWildNormal_0x3A
