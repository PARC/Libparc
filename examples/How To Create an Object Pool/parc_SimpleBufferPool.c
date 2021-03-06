/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */
#include <stdio.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_LinkedList.h>

#include "parc_SimpleBufferPool.h"

struct PARCSimpleBufferPool {
    size_t bufferSize;
    size_t limit;
    PARCLinkedList *freeList;
    PARCObjectDescriptor *descriptor;
};

static bool
_parcSimpleBufferPool_Destructor(PARCSimpleBufferPool **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCSimpleBufferPool pointer.");

    PARCSimpleBufferPool *pool = *instancePtr;

    parcLinkedList_Apply(pool->freeList, (void (*)) parcObject_SetDescriptor, (const void *) &PARCBuffer_Descriptor);

    parcLinkedList_Release(&pool->freeList);

    return true;
}

static bool
_parcSimpleBufferPool_BufferDestructor(PARCBuffer **bufferPtr)
{
    PARCBuffer *buffer = *bufferPtr;
    *bufferPtr = 0;

    PARCSimpleBufferPool *bufferPool = parcObjectDescriptor_GetTypeState(parcObject_GetDescriptor(buffer));

    if (bufferPool->limit > parcLinkedList_Size(bufferPool->freeList)) {
        parcLinkedList_Append(bufferPool->freeList, buffer);
    } else {
        parcBuffer_Acquire(buffer);
        parcObject_SetDescriptor(buffer, &parcObject_DescriptorName(PARCBuffer));
        parcBuffer_Release(&buffer);
    }

    return false;
}

parcObject_ImplementAcquire(parcSimpleBufferPool, PARCSimpleBufferPool);

parcObject_ImplementRelease(parcSimpleBufferPool, PARCSimpleBufferPool);

parcObject_Override(PARCSimpleBufferPool, PARCObject,
	.destructor = (PARCObjectDestructor *) _parcSimpleBufferPool_Destructor);

PARCSimpleBufferPool *
parcSimpleBufferPool_Create(size_t limit, size_t bufferSize)
{
    PARCSimpleBufferPool *result = parcObject_CreateInstance(PARCSimpleBufferPool);

    if (result != NULL) {
        result->limit = limit;
        result->bufferSize = bufferSize;
        result->freeList = parcLinkedList_Create();

        char *string;
        asprintf(&string, "PARCSimpleBufferPool=%zu", bufferSize);
        result->descriptor = parcObjectDescriptor_CreateExtension(&parcObject_DescriptorName(PARCBuffer), string);
        free(string);
        result->descriptor->destructor = (PARCObjectDestructor *) _parcSimpleBufferPool_BufferDestructor;
        result->descriptor->typeState = (PARCObjectTypeState *) result;
    }

    return result;
}

PARCBuffer *
parcSimpleBufferPool_GetInstance(PARCSimpleBufferPool *bufferPool)
{
    PARCBuffer *result;

    if (parcLinkedList_Size(bufferPool->freeList) > 0) {
        result = parcLinkedList_RemoveFirst(bufferPool->freeList);
    } else {
        result = parcBuffer_Allocate(bufferPool->bufferSize);
        parcObject_SetDescriptor(result, bufferPool->descriptor);
    }

    return result;
}
