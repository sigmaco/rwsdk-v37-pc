vs.1.1

#include "rainbowdefs.h"
 
;------------------------------------------------------------------------------
; Transform the position by the combined world, camera & projection matrix 
;------------------------------------------------------------------------------

dp4 oPos.x, VSIN_REG_POS, VSCONST_REG_TRANSFORM_X
dp4 oPos.y, VSIN_REG_POS, VSCONST_REG_TRANSFORM_Y
dp4 oPos.z, VSIN_REG_POS, VSCONST_REG_TRANSFORM_Z
dp4 oPos.w, VSIN_REG_POS, VSCONST_REG_TRANSFORM_W

;------------------------------------------------------------------------------
; Transform the normal
;------------------------------------------------------------------------------

dp3 VSTMP_REG_NORMAL.x, VSIN_REG_NORMAL, VSCONST_REG_WORLD_INVERSE_X
dp3 VSTMP_REG_NORMAL.y, VSIN_REG_NORMAL, VSCONST_REG_WORLD_INVERSE_Y
dp3 VSTMP_REG_NORMAL.z, VSIN_REG_NORMAL, VSCONST_REG_WORLD_INVERSE_Z

;------------------------------------------------------------------------------
; Normalize the normal
;------------------------------------------------------------------------------

dp3 VSTMP_REG_NORMAL.w, VSTMP_REG_NORMAL, VSTMP_REG_NORMAL
rsq VSTMP_REG_NORMAL.w, VSTMP_REG_NORMAL.w
mul VSTMP_REG_NORMAL, VSTMP_REG_NORMAL, VSTMP_REG_NORMAL.w

;------------------------------------------------------------------------------
; Compute world space position
;------------------------------------------------------------------------------

dp4 VSTMP_REG_POS.x, VSIN_REG_POS, VSCONST_REG_WORLD_TRANSFORM_X
dp4 VSTMP_REG_POS.y, VSIN_REG_POS, VSCONST_REG_WORLD_TRANSFORM_Y
dp4 VSTMP_REG_POS.z, VSIN_REG_POS, VSCONST_REG_WORLD_TRANSFORM_Z
dp4 VSTMP_REG_POS.w, VSIN_REG_POS, VSCONST_REG_WORLD_TRANSFORM_W

;------------------------------------------------------------------------------
; Vector from world space position to eye
;------------------------------------------------------------------------------

add VSTMP_REG_EYE, VSCONST_REG_EYEPOS, -VSTMP_REG_POS

;------------------------------------------------------------------------------
; Normalize e
;------------------------------------------------------------------------------

dp3 VSTMP_REG_EYE.w, VSTMP_REG_EYE, VSTMP_REG_EYE
rsq VSTMP_REG_EYE.w, VSTMP_REG_EYE.w
mul VSTMP_REG_EYE, VSTMP_REG_EYE, VSTMP_REG_EYE.w

;------------------------------------------------------------------------------
; h = Normalize( n + e )
;------------------------------------------------------------------------------

add VSTMP_REG_HIGHLIGHT, VSTMP_REG_NORMAL, VSTMP_REG_EYE

;------------------------------------------------------------------------------
; Normalize h
;------------------------------------------------------------------------------

dp3 VSTMP_REG_HIGHLIGHT.w, VSTMP_REG_HIGHLIGHT, VSTMP_REG_HIGHLIGHT
rsq VSTMP_REG_HIGHLIGHT.w, VSTMP_REG_HIGHLIGHT.w
mul VSTMP_REG_HIGHLIGHT, VSTMP_REG_HIGHLIGHT, VSTMP_REG_HIGHLIGHT.w

;------------------------------------------------------------------------------
; h dot e
;------------------------------------------------------------------------------

dp3 oT0.x, VSTMP_REG_NORMAL, VSTMP_REG_EYE

;------------------------------------------------------------------------------
; h dot n
;------------------------------------------------------------------------------

dp3 oT0.y, VSTMP_REG_HIGHLIGHT, VSTMP_REG_NORMAL

mov oT1.x, VSTMP_REG_EYE
mov oT1.y, VSTMP_REG_HIGHLIGHT 
