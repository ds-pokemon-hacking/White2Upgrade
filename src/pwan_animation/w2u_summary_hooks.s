.thumb

.type THUMB_BRANCH_LINK_207_0x21B337E, %function
.type THUMB_BRANCH_LINK_207_0x21B3396, %function
.type THUMB_BRANCH_LINK_207_0x21B31AA, %function

.extern W2U_SummaryAnim_Update
.extern W2U_SummaryAnim_PreDraw
.extern W2U_SummaryAnim_Draw
.extern W2U_SummaryAnim_Term

@ Preserve PSTATUS_UPDATE's normal cell-system tick, then draw the custom
@ full-frame animation when the current summary page is showing a retargeted
@ PWAN smoke-test species.
THUMB_BRANCH_LINK_207_0x21B337E:
    push {r0-r3, lr}
    ldr r3, =0x0204B7C1
    blx r3
    mov r0, r4
    bl W2U_SummaryAnim_Update
    pop {r0-r3, pc}
    .size THUMB_BRANCH_LINK_207_0x21B337E, . - THUMB_BRANCH_LINK_207_0x21B337E

@ Re-hide the vanilla summary MCSS immediately before the draw call, then write
@ the reserved OAM slots for our replacement after the normal draw path.
THUMB_BRANCH_LINK_207_0x21B3396:
    push {r0-r3, lr}
    mov r0, r4
    bl W2U_SummaryAnim_PreDraw
    pop {r0-r3}
    push {r0-r3}
    ldr r3, =0x02049ACD
    blx r3
    mov r0, r4
    bl W2U_SummaryAnim_Draw
    pop {r0-r3, pc}
    .size THUMB_BRANCH_LINK_207_0x21B3396, . - THUMB_BRANCH_LINK_207_0x21B3396

@ Run the original summary cell teardown, then clear our reserved OAM slots.
THUMB_BRANCH_LINK_207_0x21B31AA:
    push {r0-r3, lr}
    mov r0, r5
    ldr r3, =0x021B3EF9
    blx r3
    mov r0, r5
    bl W2U_SummaryAnim_Term
    pop {r0-r3, pc}
    .size THUMB_BRANCH_LINK_207_0x21B31AA, . - THUMB_BRANCH_LINK_207_0x21B31AA
