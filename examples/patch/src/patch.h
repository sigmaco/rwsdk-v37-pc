#if (!defined(_PATCH_H))
#define _PATCH_H

typedef struct PatchState PatchState;
struct PatchState
{
    RpClump            *Clump;
    RpPatchMesh        *Mesh;
};

extern void         DestroyPatchMesh(PatchState * patchState);
extern void         CreatePatchMesh(PatchState * patchState);

#endif /* (!defined(_PATCH_H)) */
