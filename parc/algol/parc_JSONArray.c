/*
 * Copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
/**
 * @author Glenn Scott, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#include <config.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_JSONArray.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_JSONValue.h>
#include <parc/algol/parc_DisplayIndented.h>

struct parcJSONArray {
    PARCDeque *array;
};

static void
_destroy(PARCJSONArray **arrayPtr)
{
    PARCJSONArray *array = *arrayPtr;
    // Un-reference the JSONValue instances here because parcDeque doesn't (yet) acquire and release its own references.

    for (int i = 0; i < parcDeque_Size(array->array); i++) {
        PARCJSONValue *value = parcDeque_GetAtIndex(array->array, i);
        parcJSONValue_Release(&value);
    }
    parcDeque_Release(&array->array);
}

parcObject_ExtendPARCObject(PARCJSONArray, _destroy, NULL, parcJSONArray_ToString, parcJSONArray_Equals, NULL, NULL, NULL);

static const PARCObjectDescriptor parcArrayValue_ObjInterface = {
    .destroy  = (PARCObjectDestroy *) parcJSONValue_Release,
    .toString = (PARCObjectToString *) parcJSONValue_ToString,
    .equals   = (PARCObjectEquals *) parcJSONValue_Equals
};

void
parcJSONArray_AssertValid(const PARCJSONArray *array)
{
    assertNotNull(array, "Must be a non-null pointer to a PARCJSONArray instance.");
    assertNotNull(array->array, "Must be a non-null pointer to a PARCDeque instance.");
}

PARCJSONArray *
parcJSONArray_Create(void)
{
    PARCJSONArray *result = parcObject_CreateInstance(PARCJSONArray);
    result->array = parcDeque_CreateObjectInterface(&parcArrayValue_ObjInterface);
    return result;
}

parcObject_ImplementAcquire(parcJSONArray, PARCJSONArray);

parcObject_ImplementRelease(parcJSONArray, PARCJSONArray);

bool
parcJSONArray_Equals(const PARCJSONArray *x, const PARCJSONArray *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        result = parcDeque_Equals(x->array, y->array);
    }
    return result;
}

PARCJSONArray *
parcJSONArray_AddValue(PARCJSONArray *array, PARCJSONValue *value)
{
    parcDeque_Append(array->array, parcJSONValue_Acquire(value));
    return array;
}

size_t
parcJSONArray_GetLength(const PARCJSONArray *array)
{
    return parcDeque_Size(array->array);
}

PARCJSONValue *
parcJSONArray_GetValue(const PARCJSONArray *array, size_t index)
{
    return (PARCJSONValue *) parcDeque_GetAtIndex(array->array, index);
}

PARCBufferComposer *
parcJSONArray_BuildString(const PARCJSONArray *array, PARCBufferComposer *composer, bool compact)
{
#ifdef hasPARCIterator
    PARCIterator *iterator = parcDeque_GetIterator(array->array);

    parcBufferComposer_PutChar(composer, '[');

    char *separator = "";

    for (i = parcIterator_Start(); i < parcIterator_Limit(iterator); i = parcIterator_Next(iterator)) {
        PARCJSONValue *value = parcIterator_Get(iterator);
        parcBufferComposer_PutString(composer, separator);
        separator = ", ";
        if (compact) {
            separator = ",";
        }

        parcJSONValue_BuildString(value, composer);
    }
    parcBufferComposer_PutChar(composer, ']');
#else
    parcBufferComposer_PutChar(composer, '[');
    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }

    char *separator = "";

    for (int i = 0; i < parcDeque_Size(array->array); i++) {
        PARCJSONValue *value = parcDeque_GetAtIndex(array->array, i);
        parcBufferComposer_PutString(composer, separator);

        parcJSONValue_BuildString(value, composer, compact);
        separator = ", ";
        if (compact) {
            separator = ",";
        }
    }
    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }
    parcBufferComposer_PutChar(composer, ']');
#endif
    return composer;
}

void
parcJSONArray_Display(const PARCJSONArray *array, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCJSONArray@%p {", array);
    parcDisplayIndented_PrintLine(indentation, "}");
}

static char *
_parcJSONArray_ToString(const PARCJSONArray *array, bool compact)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcJSONArray_BuildString(array, composer, compact);
    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    return result;
}

char *
parcJSONArray_ToString(const PARCJSONArray *array)
{
    return _parcJSONArray_ToString(array, false);
}

char *
parcJSONArray_ToCompactString(const PARCJSONArray *array)
{
    return _parcJSONArray_ToString(array, true);
}
