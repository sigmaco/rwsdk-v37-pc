/****************************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd.
 * or Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. and Canon Inc. will not, under any
 * circumstances, be liable for any lost revenue or other damages
 * arising from the use of this file.
 *
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * patch.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how RenderWare clumps can be generated dynamically.
 ****************************************************************************/

#include "math.h"
#include "rwcore.h"
#include "rpworld.h"
#include "rprandom.h"
#include "rtbezpat.h"
#include "rppatch.h"

#include "skeleton.h"

#include "patch.h"

#define ZERO       ((RwReal)0.000000000000)
#define HALF       ((RwReal)0.500000000000)
#define RECIPSQRT3 ((RwReal)0.577350269189)
#define HALFSQRT3  ((RwReal)0.866025403785)
#define ONE        ((RwReal)1.000000000000)
#define MARGIN     ((RwReal)0.001000000000)
#define UNUSED     ((RwReal)-1)

typedef struct MaterialBundle MaterialBundle;
struct MaterialBundle
{
    RwSurfaceProperties surfProp;
    RwTexture          *quadTexture;
    RwTexture          *triangleTexture;
    RwTexture          *normalTexture;
    RpMaterial         *quadMaterial;
    RpMaterial         *triangleMaterial;
    RpMaterial         *normalMaterial;

};

static void
MaterialBundleDestructor(MaterialBundle * bundle)
{

    RpMaterial         *const normalMaterial = bundle->normalMaterial;
    RpMaterial         *const triangleMaterial =
        bundle->triangleMaterial;
    RpMaterial         *const quadMaterial = bundle->quadMaterial;
    RwTexture          *const normalTexture = bundle->normalTexture;
    RwTexture          *const triangleTexture = bundle->triangleTexture;
    RwTexture          *const quadTexture = bundle->quadTexture;

    /* Destroy materials */

    if (quadMaterial)
    {
        RpMaterialDestroy(quadMaterial);
        bundle->quadMaterial = (RpMaterial *)NULL;
    }

    if (triangleMaterial)
    {
        RpMaterialDestroy(triangleMaterial);
        bundle->triangleMaterial = (RpMaterial *)NULL;
    }

    if (normalMaterial)
    {
        RpMaterialDestroy(normalMaterial);
        bundle->normalMaterial = (RpMaterial *)NULL;
    }

    /* Destroy textures */

    if (quadTexture)
    {
        RwTextureDestroy(quadTexture);
        bundle->quadTexture = (RwTexture *)NULL;
    }

    if (triangleTexture)
    {
        RwTextureDestroy(triangleTexture);
        bundle->triangleTexture = (RwTexture *)NULL;
    }

    if (normalTexture)
    {
        RwTextureDestroy(normalTexture);
        bundle->normalTexture = (RwTexture *)NULL;
    }

    return;
}

static void
MaterialBundleConstructor(MaterialBundle * bundle)
{
    RwChar             *path;
    RwTexture          *checkTexture;
    RpMaterial         *checkMaterial;

    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /* Create textures */

    bundle->normalTexture = 
        RwTextureRead(RWSTRING("normal"), (RwChar *)NULL);
    checkTexture = 
        RwTextureSetFilterMode(bundle->normalTexture, rwFILTERLINEAR);

    bundle->triangleTexture = 
        RwTextureRead(RWSTRING("tri"), (RwChar *)NULL);
    checkTexture = 
        RwTextureSetFilterMode(bundle->triangleTexture, rwFILTERLINEAR);

    bundle->quadTexture = 
        RwTextureRead(RWSTRING("quad"), (RwChar *)NULL);
    checkTexture = 
        RwTextureSetFilterMode(bundle->quadTexture, rwFILTERLINEAR);

    /* Create materials */

    /*
     * ...and materials. These materials are created with a reference
     * count of 1, meaning this application has ownership. Subsequently,
     * when the materials are associated with a geometry and its 
     * triangles, their reference counts are incremented, indicating 
     * ownership also by the geometry and each triangle...
     */
    bundle->normalMaterial = RpMaterialCreate();
    RpMaterialSetTexture(bundle->normalMaterial, bundle->normalTexture);

    bundle->triangleMaterial = RpMaterialCreate();
    RpMaterialSetTexture(bundle->triangleMaterial,
                         bundle->triangleTexture);

    bundle->quadMaterial = RpMaterialCreate();
    RpMaterialSetTexture(bundle->quadMaterial, bundle->quadTexture);

    /*
     * Set the surface reflection coefficients...
     */
    bundle->surfProp.ambient = 0.3f;
    bundle->surfProp.diffuse = 0.7f;
    bundle->surfProp.specular = 0.0f;
    checkMaterial =
        RpMaterialSetSurfaceProperties(bundle->normalMaterial,
                                       &bundle->surfProp);
    checkMaterial =
        RpMaterialSetSurfaceProperties(bundle->triangleMaterial,
                                       &bundle->surfProp);
    checkMaterial =
        RpMaterialSetSurfaceProperties(bundle->quadMaterial,
                                       &bundle->surfProp);

    return;
}

/*
 * See
 * http://web2.iadfw.net/sjbaker1/software/teapot.html
 */
static const RpQuadPatch QuadpotPatch[] = {
    {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
    {{3, 16, 17, 18, 7, 19, 20, 21, 11, 22, 23, 24, 15, 25, 26, 27}},
    {{18, 28, 29, 30, 21, 31, 32, 33, 24, 34, 35, 36, 27, 37, 38, 39}},
    {{30, 40, 41, 0, 33, 42, 43, 4, 36, 44, 45, 8, 39, 46, 47, 12}},
    {{12, 13, 14, 15, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59}},
    {{15, 25, 26, 27, 51, 60, 61, 62, 55, 63, 64, 65, 59, 66, 67, 68}},
    {{27, 37, 38, 39, 62, 69, 70, 71, 65, 72, 73, 74, 68, 75, 76, 77}},
    {{39, 46, 47, 12, 71, 78, 79, 48, 74, 80, 81, 52, 77, 82, 83, 56}},
    {{56, 57, 58, 59, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95}},
    {{59, 66, 67, 68, 87, 96, 97, 98, 91, 99, 100, 101, 95, 102, 103,
      104}},
    {{68, 75, 76, 77, 98, 105, 106, 107, 101, 108, 109, 110, 104, 111,
      112, 113}},
    {{77, 82, 83, 56, 107, 114, 115, 84, 110, 116, 117, 88, 113, 118,
      119, 92}},
    {{120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
      133, 134, 135}},
    {{123, 136, 137, 120, 127, 138, 139, 124, 131, 140, 141, 128, 135,
      142, 143, 132}},
    {{132, 133, 134, 135, 144, 145, 146, 147, 148, 149, 150, 151, 68,
      152, 153, 154}},
    {{135, 142, 143, 132, 147, 155, 156, 144, 151, 157, 158, 148, 154,
      159, 160, 68}},
    {{161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173,
      174, 175, 176}},
    {{164, 177, 178, 161, 168, 179, 180, 165, 172, 181, 182, 169, 176,
      183, 184, 173}},
    {{173, 174, 175, 176, 185, 186, 187, 188, 189, 190, 191, 192, 193,
      194, 195, 196}},
    {{176, 183, 184, 173, 188, 197, 198, 185, 192, 199, 200, 189, 196,
      201, 202, 193}},
    {{203, 203, 203, 203, 206, 207, 208, 209, 210, 210, 210, 210, 211,
      212, 213, 214}},
    {{203, 203, 203, 203, 209, 216, 217, 218, 210, 210, 210, 210, 214,
      219, 220, 221}},
    {{203, 203, 203, 203, 218, 223, 224, 225, 210, 210, 210, 210, 221,
      226, 227, 228}},
    {{203, 203, 203, 203, 225, 229, 230, 206, 210, 210, 210, 210, 228,
      231, 232, 211}},
    {{211, 212, 213, 214, 233, 234, 235, 236, 237, 238, 239, 240, 241,
      242, 243, 244}},
    {{214, 219, 220, 221, 236, 245, 246, 247, 240, 248, 249, 250, 244,
      251, 252, 253}},
    {{221, 226, 227, 228, 247, 254, 255, 256, 250, 257, 258, 259, 253,
      260, 261, 262}},
    {{228, 231, 232, 211, 256, 263, 264, 233, 259, 265, 266, 237, 262,
      267, 268, 241}},
    {{269, 269, 269, 269, 278, 279, 280, 281, 274, 275, 276, 277, 270,
      271, 272, 273}},
    {{269, 269, 269, 269, 281, 288, 289, 290, 277, 285, 286, 287, 273,
      282, 283, 284}},
    {{269, 269, 269, 269, 290, 297, 298, 299, 287, 294, 295, 296, 284,
      291, 292, 293}},
    {{269, 269, 269, 269, 299, 304, 305, 278, 296, 302, 303, 274, 293,
      300, 301, 270}}
};

#define QuadpotPatchCount \
  ((RwInt32)(sizeof(QuadpotPatch)/sizeof(QuadpotPatch[0])))

#define TEASCALE 0.25f

static const RwV3d  QuadpotVertex[] = {
    {TEASCALE * -1.4f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -1.4f, TEASCALE * -0.784f, TEASCALE * 2.4f},
    {TEASCALE * -0.784f, TEASCALE * -1.4f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * -1.4f, TEASCALE * 2.4f},
    {TEASCALE * -1.3375f, TEASCALE * 0.0f, TEASCALE * 2.53125f},
    {TEASCALE * -1.3375f, TEASCALE * -0.749f, TEASCALE * 2.53125f},
    {TEASCALE * -0.749f, TEASCALE * -1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * -0.0f, TEASCALE * -1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * -1.4375f, TEASCALE * 0.0f, TEASCALE * 2.53125f},
    {TEASCALE * -1.4375f, TEASCALE * -0.805f, TEASCALE * 2.53125f},
    {TEASCALE * -0.805f, TEASCALE * -1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * -0.0f, TEASCALE * -1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * -1.5f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -1.5f, TEASCALE * -0.84f, TEASCALE * 2.4f},
    {TEASCALE * -0.84f, TEASCALE * -1.5f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * -1.5f, TEASCALE * 2.4f},
    {TEASCALE * 0.784f, TEASCALE * -1.4f, TEASCALE * 2.4f},
    {TEASCALE * 1.4f, TEASCALE * -0.784f, TEASCALE * 2.4f},
    {TEASCALE * 1.4f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * 0.749f, TEASCALE * -1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * 1.3375f, TEASCALE * -0.749f, TEASCALE * 2.53125f},
    {TEASCALE * 1.3375f, TEASCALE * 0.0f, TEASCALE * 2.53125f},
    {TEASCALE * 0.805f, TEASCALE * -1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * 1.4375f, TEASCALE * -0.805f, TEASCALE * 2.53125f},
    {TEASCALE * 1.4375f, TEASCALE * 0.0f, TEASCALE * 2.53125f},
    {TEASCALE * 0.84f, TEASCALE * -1.5f, TEASCALE * 2.4f},
    {TEASCALE * 1.5f, TEASCALE * -0.84f, TEASCALE * 2.4f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * 1.4f, TEASCALE * 0.784f, TEASCALE * 2.4f},
    {TEASCALE * 0.784f, TEASCALE * 1.4f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * 1.4f, TEASCALE * 2.4f},
    {TEASCALE * 1.3375f, TEASCALE * 0.749f, TEASCALE * 2.53125f},
    {TEASCALE * 0.749f, TEASCALE * 1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * -0.0f, TEASCALE * 1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * 1.4375f, TEASCALE * 0.805f, TEASCALE * 2.53125f},
    {TEASCALE * 0.805f, TEASCALE * 1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * -0.0f, TEASCALE * 1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * 1.5f, TEASCALE * 0.84f, TEASCALE * 2.4f},
    {TEASCALE * 0.84f, TEASCALE * 1.5f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * 1.5f, TEASCALE * 2.4f},
    {TEASCALE * -0.784f, TEASCALE * 1.4f, TEASCALE * 2.4f},
    {TEASCALE * -1.4f, TEASCALE * 0.784f, TEASCALE * 2.4f},
    {TEASCALE * -0.749f, TEASCALE * 1.3375f, TEASCALE * 2.53125f},
    {TEASCALE * -1.3375f, TEASCALE * 0.749f, TEASCALE * 2.53125f},
    {TEASCALE * -0.805f, TEASCALE * 1.4375f, TEASCALE * 2.53125f},
    {TEASCALE * -1.4375f, TEASCALE * 0.805f, TEASCALE * 2.53125f},
    {TEASCALE * -0.84f, TEASCALE * 1.5f, TEASCALE * 2.4f},
    {TEASCALE * -1.5f, TEASCALE * 0.84f, TEASCALE * 2.4f},
    {TEASCALE * -1.75f, TEASCALE * 0.0f, TEASCALE * 1.875f},
    {TEASCALE * -1.75f, TEASCALE * -0.98f, TEASCALE * 1.875f},
    {TEASCALE * -0.98f, TEASCALE * -1.75f, TEASCALE * 1.875f},
    {TEASCALE * -0.0f, TEASCALE * -1.75f, TEASCALE * 1.875f},
    {TEASCALE * -2.0f, TEASCALE * 0.0f, TEASCALE * 1.35f},
    {TEASCALE * -2.0f, TEASCALE * -1.12f, TEASCALE * 1.35f},
    {TEASCALE * -1.12f, TEASCALE * -2.0f, TEASCALE * 1.35f},
    {TEASCALE * -0.0f, TEASCALE * -2.0f, TEASCALE * 1.35f},
    {TEASCALE * -2.0f, TEASCALE * 0.0f, TEASCALE * 0.9f},
    {TEASCALE * -2.0f, TEASCALE * -1.12f, TEASCALE * 0.9f},
    {TEASCALE * -1.12f, TEASCALE * -2.0f, TEASCALE * 0.9f},
    {TEASCALE * -0.0f, TEASCALE * -2.0f, TEASCALE * 0.9f},
    {TEASCALE * 0.98f, TEASCALE * -1.75f, TEASCALE * 1.875f},
    {TEASCALE * 1.75f, TEASCALE * -0.98f, TEASCALE * 1.875f},
    {TEASCALE * 1.75f, TEASCALE * 0.0f, TEASCALE * 1.875f},
    {TEASCALE * 1.12f, TEASCALE * -2.0f, TEASCALE * 1.35f},
    {TEASCALE * 2.0f, TEASCALE * -1.12f, TEASCALE * 1.35f},
    {TEASCALE * 2.0f, TEASCALE * 0.0f, TEASCALE * 1.35f},
    {TEASCALE * 1.12f, TEASCALE * -2.0f, TEASCALE * 0.9f},
    {TEASCALE * 2.0f, TEASCALE * -1.12f, TEASCALE * 0.9f},
    {TEASCALE * 2.0f, TEASCALE * 0.0f, TEASCALE * 0.9f},
    {TEASCALE * 1.75f, TEASCALE * 0.98f, TEASCALE * 1.875f},
    {TEASCALE * 0.98f, TEASCALE * 1.75f, TEASCALE * 1.875f},
    {TEASCALE * -0.0f, TEASCALE * 1.75f, TEASCALE * 1.875f},
    {TEASCALE * 2.0f, TEASCALE * 1.12f, TEASCALE * 1.35f},
    {TEASCALE * 1.12f, TEASCALE * 2.0f, TEASCALE * 1.35f},
    {TEASCALE * -0.0f, TEASCALE * 2.0f, TEASCALE * 1.35f},
    {TEASCALE * 2.0f, TEASCALE * 1.12f, TEASCALE * 0.9f},
    {TEASCALE * 1.12f, TEASCALE * 2.0f, TEASCALE * 0.9f},
    {TEASCALE * -0.0f, TEASCALE * 2.0f, TEASCALE * 0.9f},
    {TEASCALE * -0.98f, TEASCALE * 1.75f, TEASCALE * 1.875f},
    {TEASCALE * -1.75f, TEASCALE * 0.98f, TEASCALE * 1.875f},
    {TEASCALE * -1.12f, TEASCALE * 2.0f, TEASCALE * 1.35f},
    {TEASCALE * -2.0f, TEASCALE * 1.12f, TEASCALE * 1.35f},
    {TEASCALE * -1.12f, TEASCALE * 2.0f, TEASCALE * 0.9f},
    {TEASCALE * -2.0f, TEASCALE * 1.12f, TEASCALE * 0.9f},
    {TEASCALE * -2.0f, TEASCALE * 0.0f, TEASCALE * 0.45f},
    {TEASCALE * -2.0f, TEASCALE * -1.12f, TEASCALE * 0.45f},
    {TEASCALE * -1.12f, TEASCALE * -2.0f, TEASCALE * 0.45f},
    {TEASCALE * -0.0f, TEASCALE * -2.0f, TEASCALE * 0.45f},
    {TEASCALE * -1.5f, TEASCALE * 0.0f, TEASCALE * 0.225f},
    {TEASCALE * -1.5f, TEASCALE * -0.84f, TEASCALE * 0.225f},
    {TEASCALE * -0.84f, TEASCALE * -1.5f, TEASCALE * 0.225f},
    {TEASCALE * -0.0f, TEASCALE * -1.5f, TEASCALE * 0.225f},
    {TEASCALE * -1.5f, TEASCALE * 0.0f, TEASCALE * 0.15f},
    {TEASCALE * -1.5f, TEASCALE * -0.84f, TEASCALE * 0.15f},
    {TEASCALE * -0.84f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * -0.0f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * 1.12f, TEASCALE * -2.0f, TEASCALE * 0.45f},
    {TEASCALE * 2.0f, TEASCALE * -1.12f, TEASCALE * 0.45f},
    {TEASCALE * 2.0f, TEASCALE * 0.0f, TEASCALE * 0.45f},
    {TEASCALE * 0.84f, TEASCALE * -1.5f, TEASCALE * 0.225f},
    {TEASCALE * 1.5f, TEASCALE * -0.84f, TEASCALE * 0.225f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 0.225f},
    {TEASCALE * 0.84f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * 1.5f, TEASCALE * -0.84f, TEASCALE * 0.15f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 0.15f},
    {TEASCALE * 2.0f, TEASCALE * 1.12f, TEASCALE * 0.45f},
    {TEASCALE * 1.12f, TEASCALE * 2.0f, TEASCALE * 0.45f},
    {TEASCALE * -0.0f, TEASCALE * 2.0f, TEASCALE * 0.45f},
    {TEASCALE * 1.5f, TEASCALE * 0.84f, TEASCALE * 0.225f},
    {TEASCALE * 0.84f, TEASCALE * 1.5f, TEASCALE * 0.225f},
    {TEASCALE * -0.0f, TEASCALE * 1.5f, TEASCALE * 0.225f},
    {TEASCALE * 1.5f, TEASCALE * 0.84f, TEASCALE * 0.15f},
    {TEASCALE * 0.84f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * -0.0f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * -1.12f, TEASCALE * 2.0f, TEASCALE * 0.45f},
    {TEASCALE * -2.0f, TEASCALE * 1.12f, TEASCALE * 0.45f},
    {TEASCALE * -0.84f, TEASCALE * 1.5f, TEASCALE * 0.225f},
    {TEASCALE * -1.5f, TEASCALE * 0.84f, TEASCALE * 0.225f},
    {TEASCALE * -0.84f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * -1.5f, TEASCALE * 0.84f, TEASCALE * 0.15f},
    {TEASCALE * 1.6f, TEASCALE * 0.0f, TEASCALE * 2.025f},
    {TEASCALE * 1.6f, TEASCALE * -0.3f, TEASCALE * 2.025f},
    {TEASCALE * 1.5f, TEASCALE * -0.3f, TEASCALE * 2.25f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 2.25f},
    {TEASCALE * 2.3f, TEASCALE * 0.0f, TEASCALE * 2.025f},
    {TEASCALE * 2.3f, TEASCALE * -0.3f, TEASCALE * 2.025f},
    {TEASCALE * 2.5f, TEASCALE * -0.3f, TEASCALE * 2.25f},
    {TEASCALE * 2.5f, TEASCALE * 0.0f, TEASCALE * 2.25f},
    {TEASCALE * 2.7f, TEASCALE * 0.0f, TEASCALE * 2.025f},
    {TEASCALE * 2.7f, TEASCALE * -0.3f, TEASCALE * 2.025f},
    {TEASCALE * 3.0f, TEASCALE * -0.3f, TEASCALE * 2.25f},
    {TEASCALE * 3.0f, TEASCALE * 0.0f, TEASCALE * 2.25f},
    {TEASCALE * 2.7f, TEASCALE * 0.0f, TEASCALE * 1.8f},
    {TEASCALE * 2.7f, TEASCALE * -0.3f, TEASCALE * 1.8f},
    {TEASCALE * 3.0f, TEASCALE * -0.3f, TEASCALE * 1.8f},
    {TEASCALE * 3.0f, TEASCALE * 0.0f, TEASCALE * 1.8f},
    {TEASCALE * 1.5f, TEASCALE * 0.3f, TEASCALE * 2.25f},
    {TEASCALE * 1.6f, TEASCALE * 0.3f, TEASCALE * 2.025f},
    {TEASCALE * 2.5f, TEASCALE * 0.3f, TEASCALE * 2.25f},
    {TEASCALE * 2.3f, TEASCALE * 0.3f, TEASCALE * 2.025f},
    {TEASCALE * 3.0f, TEASCALE * 0.3f, TEASCALE * 2.25f},
    {TEASCALE * 2.7f, TEASCALE * 0.3f, TEASCALE * 2.025f},
    {TEASCALE * 3.0f, TEASCALE * 0.3f, TEASCALE * 1.8f},
    {TEASCALE * 2.7f, TEASCALE * 0.3f, TEASCALE * 1.8f},
    {TEASCALE * 2.7f, TEASCALE * 0.0f, TEASCALE * 1.575f},
    {TEASCALE * 2.7f, TEASCALE * -0.3f, TEASCALE * 1.575f},
    {TEASCALE * 3.0f, TEASCALE * -0.3f, TEASCALE * 1.35f},
    {TEASCALE * 3.0f, TEASCALE * 0.0f, TEASCALE * 1.35f},
    {TEASCALE * 2.5f, TEASCALE * 0.0f, TEASCALE * 1.125f},
    {TEASCALE * 2.5f, TEASCALE * -0.3f, TEASCALE * 1.125f},
    {TEASCALE * 2.65f, TEASCALE * -0.3f, TEASCALE * 0.9375f},
    {TEASCALE * 2.65f, TEASCALE * 0.0f, TEASCALE * 0.9375f},
    {TEASCALE * 2.0f, TEASCALE * -0.3f, TEASCALE * 0.9f},
    {TEASCALE * 1.9f, TEASCALE * -0.3f, TEASCALE * 0.6f},
    {TEASCALE * 1.9f, TEASCALE * 0.0f, TEASCALE * 0.6f},
    {TEASCALE * 3.0f, TEASCALE * 0.3f, TEASCALE * 1.35f},
    {TEASCALE * 2.7f, TEASCALE * 0.3f, TEASCALE * 1.575f},
    {TEASCALE * 2.65f, TEASCALE * 0.3f, TEASCALE * 0.9375f},
    {TEASCALE * 2.5f, TEASCALE * 0.3f, TEASCALE * 1.125f},
    {TEASCALE * 1.9f, TEASCALE * 0.3f, TEASCALE * 0.6f},
    {TEASCALE * 2.0f, TEASCALE * 0.3f, TEASCALE * 0.9f},
    {TEASCALE * -1.7f, TEASCALE * 0.0f, TEASCALE * 1.425f},
    {TEASCALE * -1.7f, TEASCALE * -0.66f, TEASCALE * 1.425f},
    {TEASCALE * -1.7f, TEASCALE * -0.66f, TEASCALE * 0.6f},
    {TEASCALE * -1.7f, TEASCALE * 0.0f, TEASCALE * 0.6f},
    {TEASCALE * -2.6f, TEASCALE * 0.0f, TEASCALE * 1.425f},
    {TEASCALE * -2.6f, TEASCALE * -0.66f, TEASCALE * 1.425f},
    {TEASCALE * -3.1f, TEASCALE * -0.66f, TEASCALE * 0.825f},
    {TEASCALE * -3.1f, TEASCALE * 0.0f, TEASCALE * 0.825f},
    {TEASCALE * -2.3f, TEASCALE * 0.0f, TEASCALE * 2.1f},
    {TEASCALE * -2.3f, TEASCALE * -0.25f, TEASCALE * 2.1f},
    {TEASCALE * -2.4f, TEASCALE * -0.25f, TEASCALE * 2.025f},
    {TEASCALE * -2.4f, TEASCALE * 0.0f, TEASCALE * 2.025f},
    {TEASCALE * -2.7f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -2.7f, TEASCALE * -0.25f, TEASCALE * 2.4f},
    {TEASCALE * -3.3f, TEASCALE * -0.25f, TEASCALE * 2.4f},
    {TEASCALE * -3.3f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -1.7f, TEASCALE * 0.66f, TEASCALE * 0.6f},
    {TEASCALE * -1.7f, TEASCALE * 0.66f, TEASCALE * 1.425f},
    {TEASCALE * -3.1f, TEASCALE * 0.66f, TEASCALE * 0.825f},
    {TEASCALE * -2.6f, TEASCALE * 0.66f, TEASCALE * 1.425f},
    {TEASCALE * -2.4f, TEASCALE * 0.25f, TEASCALE * 2.025f},
    {TEASCALE * -2.3f, TEASCALE * 0.25f, TEASCALE * 2.1f},
    {TEASCALE * -3.3f, TEASCALE * 0.25f, TEASCALE * 2.4f},
    {TEASCALE * -2.7f, TEASCALE * 0.25f, TEASCALE * 2.4f},
    {TEASCALE * -2.8f, TEASCALE * 0.0f, TEASCALE * 2.475f},
    {TEASCALE * -2.8f, TEASCALE * -0.25f, TEASCALE * 2.475f},
    {TEASCALE * -3.525f, TEASCALE * -0.25f, TEASCALE * 2.49375f},
    {TEASCALE * -3.525f, TEASCALE * 0.0f, TEASCALE * 2.49375f},
    {TEASCALE * -2.9f, TEASCALE * 0.0f, TEASCALE * 2.475f},
    {TEASCALE * -2.9f, TEASCALE * -0.15f, TEASCALE * 2.475f},
    {TEASCALE * -3.45f, TEASCALE * -0.15f, TEASCALE * 2.5125f},
    {TEASCALE * -3.45f, TEASCALE * 0.0f, TEASCALE * 2.5125f},
    {TEASCALE * -2.8f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -2.8f, TEASCALE * -0.15f, TEASCALE * 2.4f},
    {TEASCALE * -3.2f, TEASCALE * -0.15f, TEASCALE * 2.4f},
    {TEASCALE * -3.2f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -3.525f, TEASCALE * 0.25f, TEASCALE * 2.49375f},
    {TEASCALE * -2.8f, TEASCALE * 0.25f, TEASCALE * 2.475f},
    {TEASCALE * -3.45f, TEASCALE * 0.15f, TEASCALE * 2.5125f},
    {TEASCALE * -2.9f, TEASCALE * 0.15f, TEASCALE * 2.475f},
    {TEASCALE * -3.2f, TEASCALE * 0.15f, TEASCALE * 2.4f},
    {TEASCALE * -2.8f, TEASCALE * 0.15f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * 0.0f, TEASCALE * 3.15f},
    {TEASCALE * -0.0f, TEASCALE * -0.002f, TEASCALE * 3.15f},
    {TEASCALE * -0.002f, TEASCALE * 0.0f, TEASCALE * 3.15f},
    {TEASCALE * -0.8f, TEASCALE * 0.0f, TEASCALE * 3.15f},
    {TEASCALE * -0.8f, TEASCALE * -0.45f, TEASCALE * 3.15f},
    {TEASCALE * -0.45f, TEASCALE * -0.8f, TEASCALE * 3.15f},
    {TEASCALE * -0.0f, TEASCALE * -0.8f, TEASCALE * 3.15f},
    {TEASCALE * -0.0f, TEASCALE * 0.0f, TEASCALE * 2.85f},
    {TEASCALE * -0.2f, TEASCALE * 0.0f, TEASCALE * 2.7f},
    {TEASCALE * -0.2f, TEASCALE * -0.112f, TEASCALE * 2.7f},
    {TEASCALE * -0.112f, TEASCALE * -0.2f, TEASCALE * 2.7f},
    {TEASCALE * -0.0f, TEASCALE * -0.2f, TEASCALE * 2.7f},
    {TEASCALE * 0.002f, TEASCALE * 0.0f, TEASCALE * 3.15f},
    {TEASCALE * 0.45f, TEASCALE * -0.8f, TEASCALE * 3.15f},
    {TEASCALE * 0.8f, TEASCALE * -0.45f, TEASCALE * 3.15f},
    {TEASCALE * 0.8f, TEASCALE * 0.0f, TEASCALE * 3.15f},
    {TEASCALE * 0.112f, TEASCALE * -0.2f, TEASCALE * 2.7f},
    {TEASCALE * 0.2f, TEASCALE * -0.112f, TEASCALE * 2.7f},
    {TEASCALE * 0.2f, TEASCALE * 0.0f, TEASCALE * 2.7f},
    {TEASCALE * -0.0f, TEASCALE * 0.002f, TEASCALE * 3.15f},
    {TEASCALE * 0.8f, TEASCALE * 0.45f, TEASCALE * 3.15f},
    {TEASCALE * 0.45f, TEASCALE * 0.8f, TEASCALE * 3.15f},
    {TEASCALE * -0.0f, TEASCALE * 0.8f, TEASCALE * 3.15f},
    {TEASCALE * 0.2f, TEASCALE * 0.112f, TEASCALE * 2.7f},
    {TEASCALE * 0.112f, TEASCALE * 0.2f, TEASCALE * 2.7f},
    {TEASCALE * -0.0f, TEASCALE * 0.2f, TEASCALE * 2.7f},
    {TEASCALE * -0.45f, TEASCALE * 0.8f, TEASCALE * 3.15f},
    {TEASCALE * -0.8f, TEASCALE * 0.45f, TEASCALE * 3.15f},
    {TEASCALE * -0.112f, TEASCALE * 0.2f, TEASCALE * 2.7f},
    {TEASCALE * -0.2f, TEASCALE * 0.112f, TEASCALE * 2.7f},
    {TEASCALE * -0.4f, TEASCALE * 0.0f, TEASCALE * 2.55f},
    {TEASCALE * -0.4f, TEASCALE * -0.224f, TEASCALE * 2.55f},
    {TEASCALE * -0.224f, TEASCALE * -0.4f, TEASCALE * 2.55f},
    {TEASCALE * -0.0f, TEASCALE * -0.4f, TEASCALE * 2.55f},
    {TEASCALE * -1.3f, TEASCALE * 0.0f, TEASCALE * 2.55f},
    {TEASCALE * -1.3f, TEASCALE * -0.728f, TEASCALE * 2.55f},
    {TEASCALE * -0.728f, TEASCALE * -1.3f, TEASCALE * 2.55f},
    {TEASCALE * -0.0f, TEASCALE * -1.3f, TEASCALE * 2.55f},
    {TEASCALE * -1.3f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * -1.3f, TEASCALE * -0.728f, TEASCALE * 2.4f},
    {TEASCALE * -0.728f, TEASCALE * -1.3f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * -1.3f, TEASCALE * 2.4f},
    {TEASCALE * 0.224f, TEASCALE * -0.4f, TEASCALE * 2.55f},
    {TEASCALE * 0.4f, TEASCALE * -0.224f, TEASCALE * 2.55f},
    {TEASCALE * 0.4f, TEASCALE * 0.0f, TEASCALE * 2.55f},
    {TEASCALE * 0.728f, TEASCALE * -1.3f, TEASCALE * 2.55f},
    {TEASCALE * 1.3f, TEASCALE * -0.728f, TEASCALE * 2.55f},
    {TEASCALE * 1.3f, TEASCALE * 0.0f, TEASCALE * 2.55f},
    {TEASCALE * 0.728f, TEASCALE * -1.3f, TEASCALE * 2.4f},
    {TEASCALE * 1.3f, TEASCALE * -0.728f, TEASCALE * 2.4f},
    {TEASCALE * 1.3f, TEASCALE * 0.0f, TEASCALE * 2.4f},
    {TEASCALE * 0.4f, TEASCALE * 0.224f, TEASCALE * 2.55f},
    {TEASCALE * 0.224f, TEASCALE * 0.4f, TEASCALE * 2.55f},
    {TEASCALE * -0.0f, TEASCALE * 0.4f, TEASCALE * 2.55f},
    {TEASCALE * 1.3f, TEASCALE * 0.728f, TEASCALE * 2.55f},
    {TEASCALE * 0.728f, TEASCALE * 1.3f, TEASCALE * 2.55f},
    {TEASCALE * -0.0f, TEASCALE * 1.3f, TEASCALE * 2.55f},
    {TEASCALE * 1.3f, TEASCALE * 0.728f, TEASCALE * 2.4f},
    {TEASCALE * 0.728f, TEASCALE * 1.3f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * 1.3f, TEASCALE * 2.4f},
    {TEASCALE * -0.224f, TEASCALE * 0.4f, TEASCALE * 2.55f},
    {TEASCALE * -0.4f, TEASCALE * 0.224f, TEASCALE * 2.55f},
    {TEASCALE * -0.728f, TEASCALE * 1.3f, TEASCALE * 2.55f},
    {TEASCALE * -1.3f, TEASCALE * 0.728f, TEASCALE * 2.55f},
    {TEASCALE * -0.728f, TEASCALE * 1.3f, TEASCALE * 2.4f},
    {TEASCALE * -1.3f, TEASCALE * 0.728f, TEASCALE * 2.4f},
    {TEASCALE * -0.0f, TEASCALE * 0.0f, TEASCALE * 0.0f},
    {TEASCALE * -1.5f, TEASCALE * 0.0f, TEASCALE * 0.15f},
    {TEASCALE * -1.5f, TEASCALE * 0.84f, TEASCALE * 0.15f},
    {TEASCALE * -0.84f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * -0.0f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * -1.5f, TEASCALE * 0.0f, TEASCALE * 0.075f},
    {TEASCALE * -1.5f, TEASCALE * 0.84f, TEASCALE * 0.075f},
    {TEASCALE * -0.84f, TEASCALE * 1.5f, TEASCALE * 0.075f},
    {TEASCALE * -0.0f, TEASCALE * 1.5f, TEASCALE * 0.075f},
    {TEASCALE * -1.425f, TEASCALE * 0.0f, TEASCALE * 0.0f},
    {TEASCALE * -1.425f, TEASCALE * 0.798f, TEASCALE * 0.0f},
    {TEASCALE * -0.798f, TEASCALE * 1.425f, TEASCALE * 0.0f},
    {TEASCALE * -0.0f, TEASCALE * 1.425f, TEASCALE * 0.0f},
    {TEASCALE * 0.84f, TEASCALE * 1.5f, TEASCALE * 0.15f},
    {TEASCALE * 1.5f, TEASCALE * 0.84f, TEASCALE * 0.15f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 0.15f},
    {TEASCALE * 0.84f, TEASCALE * 1.5f, TEASCALE * 0.075f},
    {TEASCALE * 1.5f, TEASCALE * 0.84f, TEASCALE * 0.075f},
    {TEASCALE * 1.5f, TEASCALE * 0.0f, TEASCALE * 0.075f},
    {TEASCALE * 0.798f, TEASCALE * 1.425f, TEASCALE * 0.0f},
    {TEASCALE * 1.425f, TEASCALE * 0.798f, TEASCALE * 0.0f},
    {TEASCALE * 1.425f, TEASCALE * 0.0f, TEASCALE * 0.0f},
    {TEASCALE * 1.5f, TEASCALE * -0.84f, TEASCALE * 0.15f},
    {TEASCALE * 0.84f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * -0.0f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * 1.5f, TEASCALE * -0.84f, TEASCALE * 0.075f},
    {TEASCALE * 0.84f, TEASCALE * -1.5f, TEASCALE * 0.075f},
    {TEASCALE * -0.0f, TEASCALE * -1.5f, TEASCALE * 0.075f},
    {TEASCALE * 1.425f, TEASCALE * -0.798f, TEASCALE * 0.0f},
    {TEASCALE * 0.798f, TEASCALE * -1.425f, TEASCALE * 0.0f},
    {TEASCALE * -0.0f, TEASCALE * -1.425f, TEASCALE * 0.0f},
    {TEASCALE * -0.84f, TEASCALE * -1.5f, TEASCALE * 0.15f},
    {TEASCALE * -1.5f, TEASCALE * -0.84f, TEASCALE * 0.15f},
    {TEASCALE * -0.84f, TEASCALE * -1.5f, TEASCALE * 0.075f},
    {TEASCALE * -1.5f, TEASCALE * -0.84f, TEASCALE * 0.075f},
    {TEASCALE * -0.798f, TEASCALE * -1.425f, TEASCALE * 0.0f},
    {TEASCALE * -1.425f, TEASCALE * -0.798f, TEASCALE * 0.0f}
};

#define QuadpotVertexCount \
  ((RwInt32)(sizeof(QuadpotVertex)/sizeof(QuadpotVertex[0])))

static RtBezierMatrix triSample = {
    {{ONE - HALF, ZERO - HALF, ZERO - HALF, UNUSED},
     {HALFSQRT3 - HALF, HALF - HALF, ZERO - HALF, UNUSED},
     {HALF - HALF, HALFSQRT3 - HALF, ZERO - HALF, UNUSED},
     {ZERO - HALF, ONE - HALF, ZERO - HALF, UNUSED}},
    {{HALFSQRT3 - HALF, ZERO - HALF, HALF - HALF, UNUSED},
     {RECIPSQRT3 - HALF, RECIPSQRT3 - HALF, RECIPSQRT3 - HALF, UNUSED},
     {ZERO - HALF, HALFSQRT3 - HALF, HALF - HALF, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED}},
    {{HALF - HALF, ZERO - HALF, HALFSQRT3 - HALF, UNUSED},
     {ZERO - HALF, HALF - HALF, HALFSQRT3 - HALF, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED}},
    {{ZERO - HALF, ZERO - HALF, ONE - HALF, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED},
     {UNUSED, UNUSED, UNUSED, UNUSED}
     }
};

static void
PrimePatchMesh(MaterialBundle * bundle, RpPatchMesh * patchMesh)
{
    const RwUInt32     *const indexPtr = (RwUInt32 *) & QuadpotPatch;
    RwV3d              *const positions =
        RpPatchMeshGetPositions(patchMesh);
    RwV3d              *const normals =
        RpPatchMeshGetNormals(patchMesh);
    RwTexCoords        *const texCoords = patchMesh->texCoords[0];
    RwUInt32            quadIndex = 0;
    RwUInt32            index;
    RwInt32             triIndex = 0;
    RpTriPatch          triPatch;
    RtBezierMatrix      triControl;
    RtBezierMatrix      triQuad;
    RtBezierMatrix      triNormal;
    RwInt32             j;
    RwBBox              QuadpotBox;
    RwV3d               QuadpotCenter;
    RwV3d               QuadpotRecipDiameter;

    /*
     * Create the patch mesh textures. 
     * These textures are created with a reference count of 1
     * -- meaning this application has ownership.
     * Subsequently, when a texture is associated with a material,
     * its reference count is incremented, indicating ownership 
     * also by the material...
     */
    /* assign RenderWare vertex indices to the vertex data and */
    /* copy the vertex data into the RenderWare patchMesh */

    /* Quad Control Points */

    QuadpotBox.sup.x = QuadpotBox.inf.x = QuadpotVertex[0].x;
    QuadpotBox.sup.y = QuadpotBox.inf.y = QuadpotVertex[0].y;
    QuadpotBox.sup.z = QuadpotBox.inf.z = QuadpotVertex[0].z;

    for (j = 0; j < QuadpotVertexCount; j++)
    {

        normals[j].x = (RwReal) 0;
        normals[j].y = (RwReal) 0;
        normals[j].z = (RwReal) 0;

        if (QuadpotBox.sup.x < QuadpotVertex[j].x)
            (QuadpotBox.sup.x = QuadpotVertex[j].x);
        else if (QuadpotBox.inf.x > QuadpotVertex[j].x)
            (QuadpotBox.inf.x = QuadpotVertex[j].x);

        if (QuadpotBox.sup.y < QuadpotVertex[j].y)
            (QuadpotBox.sup.y = QuadpotVertex[j].y);
        else if (QuadpotBox.inf.y > QuadpotVertex[j].y)
            (QuadpotBox.inf.y = QuadpotVertex[j].y);

        if (QuadpotBox.sup.z < QuadpotVertex[j].z)
            (QuadpotBox.sup.z = QuadpotVertex[j].z);
        else if (QuadpotBox.inf.z > QuadpotVertex[j].z)
            (QuadpotBox.inf.z = QuadpotVertex[j].z);

    }

    RwV3dAdd(&QuadpotCenter, &QuadpotBox.sup, &QuadpotBox.inf);
    RwV3dScale(&QuadpotCenter, &QuadpotCenter, ((RwReal) 0.5));

    RwV3dSub(&QuadpotRecipDiameter, &QuadpotBox.sup, &QuadpotBox.inf);
    QuadpotRecipDiameter.x = ((RwReal) 1) / QuadpotRecipDiameter.x;
    QuadpotRecipDiameter.y = ((RwReal) 1) / QuadpotRecipDiameter.y;
    QuadpotRecipDiameter.z = ((RwReal) 1) / QuadpotRecipDiameter.z;

    index = 0;
    for (quadIndex = 0; quadIndex < QuadpotPatchCount; quadIndex++)
    {
        RpQuadPatch         quadPatch;
        RtBezierMatrix      quadControl;
        RtBezierMatrix      quadNormal;
        RwUInt32           *cpIndices = &quadPatch.cpIndices[0];
        const RwInt32       base = index;

        for (j = 0; j <= 3; j++)
        {
            RwInt32             i;

            for (i = 0; i <= 3; i++)
            {
                const RwUInt32      slot = indexPtr[index++];

                *cpIndices++ = slot;

                quadControl[j][i].x = QuadpotVertex[slot].x;
                quadControl[j][i].y = QuadpotVertex[slot].y;
                quadControl[j][i].z = QuadpotVertex[slot].z;
            }
        }

        RtBezierQuadGetNormals(quadNormal, quadControl);

        index = base;
        for (j = 0; j <= 3; j++)
        {
            RwInt32             i;

            for (i = 0; i <= 3; i++)
            {

                const RwUInt32      slot = indexPtr[index++];

                normals[slot].x += quadNormal[j][i].x;
                normals[slot].y += quadNormal[j][i].y;
                normals[slot].z += quadNormal[j][i].z;
            }
        }

        RpPatchMeshSetQuadPatch(patchMesh, quadIndex, &quadPatch);
        RpPatchMeshSetQuadPatchMaterial(patchMesh,
                                        quadIndex,
                                        bundle->quadMaterial);

    }

    for (index = 0; index < QuadpotVertexCount; index++)
    {
        RwReal              factor;
        RwV3d               unitary;
        RwTexCoords         tex;

        RwV3dSub(&positions[index], &QuadpotVertex[index],
                 &QuadpotCenter);
        RwV3dNormalize(&unitary, &positions[index]);
        positions[index].y -= ((RwReal) 1);

        tex.u = (RwReal) atan2(unitary.x, unitary.y);
        tex.u = ((0 > tex.u) ? (rwPI + tex.u) : tex.u) / rwPI;

        tex.v = ((RwReal) acos(unitary.z) / rwPI);

        texCoords[index] = tex;

        factor = RwV3dDotProduct(&normals[index], &normals[index]);
        if (factor > 0)
        {
            rwInvSqrt(&factor, factor);
            RwV3dScale(&normals[index], &normals[index], factor);
        }

    }

    /* Tri Control Points */

    RtBezierTriangleControlFit3d(triControl, triSample);
    RtBezierQuadFromTriangle(triQuad, triControl);
    RtBezierQuadGetNormals(triNormal, triQuad);

    index = QuadpotVertexCount;
    for (j = 0; j <= 3; j++)
    {
        RwInt32             i;
        RwTexCoords         tex;

        tex.v = ((RwReal) j) / 3;
        for (i = 0; i <= (3 - j); i++)
        {

            positions[index].x = triControl[j][i].x;
            positions[index].y = triControl[j][i].y + ((RwReal) 1);
            positions[index].z = triControl[j][i].z;

            normals[index].x = triNormal[j][i].x;
            normals[index].y = triNormal[j][i].y;
            normals[index].z = triNormal[j][i].z;

            tex.u = ((RwReal) i) / 3;
            texCoords[index] = tex;

            index++;
        }
    }

    /* build the patch topology */
    index = QuadpotVertexCount;
    for (j = 0; j < rpTRIPATCHNUMCONTROLPOINTS; j++)
    {
        triPatch.cpIndices[j] = index++;
    }

    RpPatchMeshSetTriPatch(patchMesh, triIndex, &triPatch);
    RpPatchMeshSetTriPatchMaterial(patchMesh,
                                   triIndex, bundle->triangleMaterial);
    triIndex++;

}

/*
 *****************************************************************************
 */

void
DestroyPatchMesh(PatchState * patchState)
{
    RpPatchMesh        *const Mesh = patchState->Mesh;
    RpClump            *const Clump = patchState->Clump;

    if (Clump)
    {
        RpClumpDestroy(Clump);
        patchState->Clump = (RpClump *) NULL;
    }

    if (Mesh)
    {
        RpPatchMeshDestroy(Mesh);
        patchState->Mesh = (RpPatchMesh *) NULL;
    }

    return;
}

#define RwV3dAddScaled(o, a, b, s)              \
MACRO_START                                     \
{                                               \
    (o)->x = (((a)->x) + ( (b)->x) * (s) );     \
    (o)->y = (((a)->y) + ( (b)->y) * (s) );     \
    (o)->z = (((a)->z) + ( (b)->z) * (s) );     \
}                                               \
MACRO_STOP

#define RwV3dSubScaled(o, a, b, s)              \
MACRO_START                                     \
{                                               \
    (o)->x = (((a)->x) - ( (b)->x) * (s) );     \
    (o)->y = (((a)->y) - ( (b)->y) * (s) );     \
    (o)->z = (((a)->z) - ( (b)->z) * (s) );     \
}                                               \
MACRO_STOP

#define  AXIAL  ( ((RwReal)1)/ 16)
#define  RADIAL ( AXIAL / 4 )

static void
V3dAtRightAngles(RwV3d * right, RwV3d * at)
{
    RwV3d               atAbs;

    atAbs.x = (at->x < 0) ? (-at->x) : (at->x);
    atAbs.y = (at->y < 0) ? (-at->y) : (at->y);
    atAbs.z = (at->z < 0) ? (-at->z) : (at->z);

    if (atAbs.x > atAbs.y)
    {
        if (atAbs.x > atAbs.z)
        {
            const RwReal        factor = at->x;

            right->x = -factor * (at->y + at->z);
            right->y = factor;
            right->z = factor;
        }
        else
        {
            const RwReal        factor = at->z;

            right->x = factor;
            right->y = factor;
            right->z = -factor * (at->x + at->y);
        }
    }
    else
    {
        if (atAbs.y > atAbs.z)
        {
            const RwReal        factor = at->y;

            right->x = factor;
            right->y = -factor * (at->x + at->z);
            right->z = factor;
        }
        else
        {
            const RwReal        factor = at->z;

            right->x = factor;
            right->y = factor;
            right->z = -factor * (at->x + at->y);
        }
    }

    return;
}

static RpGeometry  *
PatchControlPointGeometry(PatchState * patchState,
                          MaterialBundle * materialBundle)
{
    RpGeometry         *geometry = (RpGeometry *) NULL;
    const RpPatchMesh  *const patchMesh = patchState->Mesh;
    const RwInt32       pointCount =
        RpPatchMeshGetNumControlPoints(patchMesh);
    const RwInt32       NormalVertexCount = 5 * pointCount;
    const RwInt32       NormalTriangleCount = 4 * pointCount;

    geometry = RpGeometryCreate(NormalVertexCount,
                                NormalTriangleCount,
                                rpGEOMETRYPRELIT |
                                rpGEOMETRYMODULATEMATERIALCOLOR |
                                rpGEOMETRYTEXTURED);

    if (geometry != NULL)
    {
        RwSphere            boundingSphere;
        RpMaterial         *NormalMaterial =
            materialBundle->normalMaterial;
        RpMorphTarget      *morphTarget =
            RpGeometryGetMorphTarget(geometry, 0);
        RpTriangle         *tlist = RpGeometryGetTriangles(geometry);
        RwTexCoords        *texCoord =
            RpGeometryGetVertexTexCoords(geometry,
                                         rwTEXTURECOORDINATEINDEX0);
        RwV3d              *vlist =
            RpMorphTargetGetVertices(morphTarget);
        RwRGBA             *prelights =
            RpGeometryGetPreLightColors(geometry);
        RwV3d              *const positions =
            RpPatchMeshGetPositions(patchMesh);
        RwV3d              *const normals =
            RpPatchMeshGetNormals(patchMesh);
        RwInt32             i;

        /*
         * There's only one morph target, with index 0...
         */

        /* 
         * Construct the triangle and vertex lists by converting the 
         * Normals and hexagons to triangles.
         * Each Normal and hexagon has its own vertices and normals
         * so that the faces can be rendered flat...
         */

        for (i = 0; i < pointCount; i++)
        {
            /*
             * Calculate the face normal...
             */
            static const RwTexCoords uvPyramid[5] = 
            {
                {0.5, 0.5}, {0, 0}, {0, 1}, {1, 0}, {1, 1}
            };
            static const RwRGBA white = { 0xff, 0xff, 0xff, 0xff };
            static const RwRGBA blue = { 0x00, 0x00, 0xff, 0xff };
            const RwV3d        *const point = &positions[i];
            const RwV3d        *const normal = &normals[i];
            RwV3d               right;
            RwV3d               up;
            RwV3d               at;
            RwReal              recipLength;
            const RwUInt16      base = (RwUInt16)(i * 5);
            
            RwV3dNormalizeMacro(recipLength, &at, normal);

            V3dAtRightAngles(&right, &at);
            RwV3dNormalizeMacro(recipLength, &right, &right);

            RwV3dCrossProduct(&up, &right, &at);
            RwV3dNormalizeMacro(recipLength, &up, &up);

            RwV3dAddScaled(vlist, point, &at, AXIAL);
            vlist++;
            *texCoord++ = uvPyramid[0];
            *prelights++ = white;

            RwV3dSubScaled(vlist, point, &up, RADIAL);
            vlist++;
            *texCoord++ = uvPyramid[1];
            *prelights++ = blue;

            RwV3dSubScaled(vlist, point, &right, RADIAL);
            vlist++;
            *texCoord++ = uvPyramid[2];
            *prelights++ = blue;

            RwV3dAddScaled(vlist, point, &up, RADIAL);
            vlist++;
            *texCoord++ = uvPyramid[3];
            *prelights++ = blue;

            RwV3dAddScaled(vlist, point, &right, RADIAL);
            vlist++;
            *texCoord++ = uvPyramid[4];
            *prelights++ = blue;

            /*
             * Initialize each face triangle with vertex list indices and
             * a material. The first call to RpGeometryTriangleSetMaterial will
             * increment the material's reference count by 2 to indicate
             * ownership by the triangle AND the geometry; subsequent calls
             * only increment it by 1 each time...
             */
            RpGeometryTriangleSetVertexIndices(geometry, tlist,
                                               base, (RwUInt16)(base + 1),
                                               (RwUInt16)(base + 2));
            RpGeometryTriangleSetMaterial(geometry, tlist++,
                                          NormalMaterial);

            RpGeometryTriangleSetVertexIndices(geometry, tlist,
                                               base, (RwUInt16)(base + 2),
                                               (RwUInt16)(base + 3));
            RpGeometryTriangleSetMaterial(geometry, tlist++,
                                          NormalMaterial);

            RpGeometryTriangleSetVertexIndices(geometry, tlist,
                                               base, (RwUInt16)(base + 3),
                                               (RwUInt16)(base + 4));
            RpGeometryTriangleSetMaterial(geometry, tlist++,
                                          NormalMaterial);

            RpGeometryTriangleSetVertexIndices(geometry, tlist,
                                               base, (RwUInt16)(base + 4),
                                               (RwUInt16)(base + 1));
            RpGeometryTriangleSetMaterial(geometry, tlist++,
                                          NormalMaterial);

        }

        /*
         * Need to re-calculate and set the bounding-sphere ourselves
         * before unlocking...
         */

        RpMorphTargetCalcBoundingSphere(morphTarget, &boundingSphere);
        RpMorphTargetSetBoundingSphere(morphTarget, &boundingSphere);

        RpGeometryUnlock(geometry);

    }

    return geometry;
}

/*
 *****************************************************************************
 */

void
CreatePatchMesh(PatchState * patchState)
{
    RpClump            *clump = RpClumpCreate();

    patchState->Clump = clump;
    if (patchState->Clump)
    {
        MaterialBundle      bundle;
        const RwInt32       QuadPatchCount = QuadpotPatchCount;
        const RwInt32       TriPatchCount = 1;
        const RwInt32       VertexCount = (QuadpotVertexCount +
                                           rpTRIPATCHNUMCONTROLPOINTS);
        const RwInt32       patchFlags = (rpPATCHMESHPOSITIONS |
                                          rpPATCHMESHNORMALS |
                                          rpPATCHMESHTEXTURED |
                                          rpPATCHMESHTEXCOORDSETS(1) |
                                          rpPATCHMESHLIGHT);
        MaterialBundleConstructor(&bundle);

        patchState->Mesh = 
            RpPatchMeshCreate(QuadPatchCount,
                              TriPatchCount, 
                              VertexCount,
                              patchFlags);

        if (patchState->Mesh)
        {
            RpAtomic           *atomic;
            RwFrame            *frame = (RwFrame *)NULL;
            RpClump            *checkClump;

            PrimePatchMesh(&bundle, patchState->Mesh);

            /*
             * Stick it in a single-atomic clump
             */
            frame = RwFrameCreate();
            checkClump = RpClumpSetFrame(patchState->Clump, frame);
            atomic = RpAtomicCreate();
            if (atomic)
            {
                RpGeometry         *const geometry =
                    PatchControlPointGeometry(patchState, &bundle);
                frame = RwFrameCreate();
                RpAtomicSetFrame(atomic, frame);

                RpPatchAtomicSetPatchMesh(atomic, patchState->Mesh);
                RpPatchAtomicSetType(atomic, rpPATCHTYPEGENERIC);
                RpClumpAddAtomic(patchState->Clump, atomic);

                atomic = RpAtomicCreate();
                RpAtomicSetFrame(atomic, frame);
                RpAtomicSetGeometry(atomic, geometry, 0);
                RpClumpAddAtomic(clump, atomic);
                RpGeometryDestroy(geometry);

                RpPatchMeshUnlock(patchState->Mesh);

            }
            else
            {
                RpPatchMeshDestroy(patchState->Mesh);
            }

            RwFrameAddChild(RpClumpGetFrame(patchState->Clump), frame);

        }

        /*
         * As a convenience, we can remove the application's ownership of the 
         * geometry, materials and textures it created by calling the 
         * corresponding destroy functions. 
         * This will decrement their reference counts without actually 
         * deleting the resources because they now have other
         * owners (the atomic owns the geometry, the geometry and its 
         * triangles own the materials, each material owns a texture). 
         * Now we can simply use RpClumpDestroy later when the 
         * application has finished with it...
         */

        MaterialBundleDestructor(&bundle);
    }

    return;
}
