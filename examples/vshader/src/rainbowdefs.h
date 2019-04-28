#ifndef VERT_DEFS_H
#define VERT_DEFS_H


/* For use in the vertex shader descriptor */
#define VSD_REG_POS                     0
#define VSD_REG_NORMAL                  1
#define VSD_REG_TEXCOORDS               2

/* Input register - For use in the vertex shader code */
#define VSIN_REG_POS                    v0
#define VSIN_REG_NORMAL                 v1
#define VSIN_REG_TEXCOORDS              v2


/* Temporary register - For use in the vertex shader code */
#define VSTMP_REG_NORMAL                r0
#define VSTMP_REG_HIGHLIGHT             r1
#define VSTMP_REG_EYE                   r2
#define VSTMP_REG_POS                   r3

/* Vertex shader defines */
#define VSCONST_REG_BASE                0

#define VSCONST_REG_TRANSFORM_OFFSET    VSCONST_REG_BASE
#define VSCONST_REG_TRANSFORM_SIZE      4

#define VSCONST_REG_WORLD_TRANSFORM_OFFSET    VSCONST_REG_TRANSFORM_OFFSET + VSCONST_REG_TRANSFORM_SIZE
#define VSCONST_REG_WORLD_TRANSFORM_SIZE      4

#define VSCONST_REG_WORLD_INVERSE_OFFSET    VSCONST_REG_WORLD_TRANSFORM_OFFSET + VSCONST_REG_WORLD_TRANSFORM_SIZE
#define VSCONST_REG_WORLD_INVERSE_SIZE      4

#define VSCONST_REG_EYEPOS_OFFSET           VSCONST_REG_WORLD_INVERSE_OFFSET + VSCONST_REG_WORLD_INVERSE_SIZE                
#define VSCONST_REG_EYEPOS_SIZE             1

/* Constant register - For use in the vertex shader code */
#define VSCONST_REG_TRANSFORM_X         c[VSCONST_REG_TRANSFORM_OFFSET + 0]
#define VSCONST_REG_TRANSFORM_Y         c[VSCONST_REG_TRANSFORM_OFFSET + 1]
#define VSCONST_REG_TRANSFORM_Z         c[VSCONST_REG_TRANSFORM_OFFSET + 2]
#define VSCONST_REG_TRANSFORM_W         c[VSCONST_REG_TRANSFORM_OFFSET + 3]
#define VSCONST_REG_WORLD_TRANSFORM_X   c[VSCONST_REG_WORLD_TRANSFORM_OFFSET + 0]
#define VSCONST_REG_WORLD_TRANSFORM_Y   c[VSCONST_REG_WORLD_TRANSFORM_OFFSET + 1]
#define VSCONST_REG_WORLD_TRANSFORM_Z   c[VSCONST_REG_WORLD_TRANSFORM_OFFSET + 2]
#define VSCONST_REG_WORLD_TRANSFORM_W   c[VSCONST_REG_WORLD_TRANSFORM_OFFSET + 3]
#define VSCONST_REG_WORLD_INVERSE_X     c[VSCONST_REG_WORLD_INVERSE_OFFSET + 0]
#define VSCONST_REG_WORLD_INVERSE_Y     c[VSCONST_REG_WORLD_INVERSE_OFFSET + 1]
#define VSCONST_REG_WORLD_INVERSE_Z     c[VSCONST_REG_WORLD_INVERSE_OFFSET + 2]
#define VSCONST_REG_WORLD_INVERSE_W     c[VSCONST_REG_WORLD_INVERSE_OFFSET + 3]
#define VSCONST_REG_EYEPOS              c[VSCONST_REG_EYEPOS_OFFSET]

#endif /* VERT_DEFS_H */
