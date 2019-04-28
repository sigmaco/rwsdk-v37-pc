/**********************************************************************
 *
 * File : RpAName.c
 *
 * Abstract : Simple plugin to allow atomics to have names assigned.
 *            Used only to demonstrate the steps needed to write a
 *            RenderWare plugin.
 *            For illustration only.  See accompanying ex4.doc.
 *
 **********************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd. or
 * Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. will not, under any
 * circumstances, be liable for any lost revenue or other damages arising
 * from the use of this file.
 *
 * Copyright (c) 1999 Criterion Software Ltd.
 * All Rights Reserved.
 *
 * RenderWare is a trademark of Canon Inc.
 *
 ****************************************************************************/

/*--- Include Files ---*/

#include <rwcore.h>
#include <rpworld.h>

#include "RpAName.h"

/****************************************************************************/
/* Local defines */

/* Given an atomic, return a pointer to the extension */
#define RPEXTFROMATOMIC(a)\
    ((char **)(((RwUInt8 *)(a)) + rpExtensionOffset))

/* note that this is a hack and should be a registered plugin ID */
#define rwID_EXAMPLE 0xff

/****************************************************************************/
/* Local variables */

static RwInt32 rpExtensionOffset = -1;

/****************************************************************************/
/* Local functions for the atomic name plugin */


static void *
rpConstructor(void *atom,
              RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
    if (rpExtensionOffset > 0)
    {
        char **extData = RPEXTFROMATOMIC(atom);

        /* atomics have no name to start with */
        *extData = NULL;
    }

    /* success */
    return (atom);
}

static void *
rpDestructor(void *atom,
             RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
    if (rpExtensionOffset > 0)
    {
        char **extData = RPEXTFROMATOMIC(atom);

        if (*extData)
        {
            RwFree(*extData);
        }
    }

    /* success */
    return (atom);
}

/* rpStreamWrite function */

static RwStream *
rpStreamWrite(RwStream *stream, RwInt32 len,
              const void *data, RwInt32 offset, RwInt32 size)
{
    char **extData = RPEXTFROMATOMIC(data);

    RwStreamWrite(stream, *extData, len);

    return stream;
}

/* rpStreamRead function */

static RwStream *
rpStreamRead(RwStream *stream, RwInt32 len,
             void *data, RwInt32 offset, RwInt32 size)
{
    char **extData = RPEXTFROMATOMIC(data);
    char  *name    = RwMalloc(len, rwID_NAOBJECT);

    if (name)
    {
        RwStreamRead(stream, name, len);

        *extData = name;

        return stream;
    }

    /* no name to read, or problem with memory */
    *extData = NULL;
    return stream;
}

/* rpStreamGetSize function */

static RwInt32
rpStreamGetSize(const void *data, RwInt32 offset, RwInt32 size)
{
    char **extData = RPEXTFROMATOMIC(data);

    if (*extData)
    {
        return rwstrlen(*extData) + 1;
    }

    /* no data to write */
    return 0;
}

/*********************************************************************************/

/*   Atomic Name Application Programming Interface    */

/*********************************************************************************/

RwInt32
RpAtomicNameInitialize(void)
{
    rpExtensionOffset =
        RpAtomicRegisterPlugin(sizeof(char *),
               MAKECHUNKID(rwVENDORID_CRITERIONTK, rwID_EXAMPLE),
               rpConstructor,
               rpDestructor,
               NULL);

    RpAtomicRegisterPluginStream(MAKECHUNKID(rwVENDORID_CRITERIONTK, rwID_EXAMPLE),
                                 rpStreamRead,
                                 rpStreamWrite,
                                 rpStreamGetSize);

    /* return the offset */
    return (rpExtensionOffset);
}

/* RpAtomicNameSetName function */

const char *
RpAtomicNameSetName(RpAtomic *atom, const char *name)
{
    RwInt32 len = rwstrlen(name);

    if ((rpExtensionOffset > 0) & (len > 0))
    {
        char **extData = RPEXTFROMATOMIC(atom);

        /* blow away any old name */
        if (*extData)
        {
            RwFree(*extData);
        }

        *extData = RwMalloc(len + 1, rwID_NAOBJECT);
        rwstrcpy(*extData, name);

        return name;
    }

    /* plugin not initialized or no name provided */
    return NULL;
}


/* RpAtomicNameGetName function */

const char *
RpAtomicNameGetName(RpAtomic *atom)
{
    if (rpExtensionOffset > 0)
    {
        char **extData = RPEXTFROMATOMIC(atom);

        return *extData;
    }

    /* plugin not initialized */
    return NULL;
}

