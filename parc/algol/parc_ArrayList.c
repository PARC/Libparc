/*
 * Copyright (c) 2013-2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
 * @copyright (c) 2013-2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <config.h>

#include <stdio.h>

#include <stdbool.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

PARCListInterface *PARCArrayListAsPARCList = &(PARCListInterface) {
    .Add                    = (bool      (*)(void *, void *))                       parcArrayList_Add,
    .AddAtIndex             = (void      (*)(void *, int index, void *))            parcArrayList_InsertAtIndex,
    .AddCollection          = (bool      (*)(void *, PARCCollection *))             NULL,
    .AddCollectionAtIndex   = (bool      (*)(void *, int index, PARCCollection *)) NULL,
    .Clear                  = (void      (*)(void *))                                    parcArrayList_Clear,
    .Contains               = (bool      (*)(const void *, const PARCObject *))                 NULL,
    .ContainsCollection     = (bool      (*)(const void *, const PARCCollection *))             NULL,
    .Copy                   = (void *    (*)(const PARCList *))                     parcArrayList_Copy,
    .Destroy                = (void      (*)(void **))                              parcArrayList_Destroy,
    .Equals                 = (bool      (*)(const void *, const void *))           parcArrayList_Equals,
    .GetAtIndex             = (void *    (*)(const void *, size_t))                 parcArrayList_Get,
    .HashCode               = (PARCHashCode (*)(const void *))                               NULL,
    .IndexOf                = (size_t    (*)(const void *, const PARCObject *element))          NULL,
    .IsEmpty                = (bool      (*)(const void *))                         parcArrayList_IsEmpty,
    .LastIndexOf            = (size_t    (*)(void *, const PARCObject *element))        NULL,
    .Remove                 = (bool      (*)(void *, const PARCObject *))                NULL,
    .RemoveAtIndex          = (void *    (*)(PARCList *, size_t))                   parcArrayList_RemoveAtIndex,
    .RemoveCollection       = (bool      (*)(void *, const PARCCollection *))             NULL,
    .RetainCollection       = (bool      (*)(void *, const PARCCollection *))             NULL,
    .SetAtIndex             = (void *    (*)(void *, size_t index, void *))         parcArrayList_Set,
    .Size                   = (size_t    (*)(const void *))                         parcArrayList_Size,
    .SubList                = (PARCList *(*)(const void *, size_t, size_t))               NULL,
    .ToArray                = (void**    (*)(const void *))                               NULL,
};

struct parc_array_list {
    void **array;
    size_t numberOfElements;
    size_t limit;
    bool (*equalsElement)(void *x, void *y);
    void (*destroyElement)(void **elementAddress);
};

static size_t
_remaining(const PARCArrayList *array)
{
    return array->limit - array->numberOfElements;
}

static PARCArrayList *
_ensureCapacity(PARCArrayList *array, size_t newCapacity)
{
    void *newArray = parcMemory_Reallocate(array->array, newCapacity * sizeof(void *));

    if (newArray == NULL) {
        return NULL;
    }
    array->array = newArray;
    array->limit = newCapacity;

    return array;
}

static PARCArrayList *
_ensureRemaining(PARCArrayList *array, size_t remnant)
{
    assertNotNull(array, "Parameter must be a non-null PARCArrayList pointer.");

    if (_remaining(array) < remnant) {
        size_t newCapacity = parcArrayList_Size(array) + remnant;
        return _ensureCapacity(array, newCapacity);
    }
    return array;
}

bool
parcArrayList_IsValid(const PARCArrayList *instance)
{
    bool result = false;

    if (instance != NULL) {
        if (instance->numberOfElements == 0 ? true : instance->array != NULL) {
            result = true;
        }
    }

    return result;
}

void
parcArrayList_AssertValid(const PARCArrayList *instance)
{
    trapIllegalValueIf(instance == NULL, "Parameter must be a non-null PARC_ArrayList pointer.");
    assertTrue(instance->numberOfElements == 0 ? true : instance->array != NULL, "PARC_ArrayList size is inconsistent.");
}

static PARCArrayList *
_parcArrayList_Init(PARCArrayList *arrayList,
                    size_t nElements,
                    size_t limit,
                    void **array,
                    bool (*equalsElement)(void *x, void *y),
                    void (*destroyElement)(void **elementAddress))
{
    if (arrayList != NULL) {
        arrayList->array = array;
        arrayList->numberOfElements = nElements;
        arrayList->limit = limit;
        arrayList->equalsElement = equalsElement;
        arrayList->destroyElement = destroyElement;
    }
    return arrayList;
}

PARCArrayList *
parcArrayList_Create(void (*destroyElement)(void **element))
{
    PARCArrayList *result = parcMemory_AllocateAndClear(sizeof(PARCArrayList));
    assertNotNull(result, "Memory allocation of PARCArrayList failed");

    return _parcArrayList_Init(result, 0, 0, NULL, NULL, destroyElement);
}

PARCArrayList *
parcArrayList_Create_Capacity(bool (*equalsElement)(void *x, void *y), void (*destroyElement)(void **element), size_t size)
{
    PARCArrayList *result = parcMemory_AllocateAndClear(sizeof(PARCArrayList));
    assertNotNull(result, "Memory allocation of PARCArrayList failed");

    _parcArrayList_Init(result, 0, 0, NULL, equalsElement, destroyElement);

    if (result != NULL) {
        _ensureRemaining(result, size);
    }

    return result;
}

bool
parcArrayList_Add(PARCArrayList *array, const void *pointer)
{
    parcArrayList_OptionalAssertValid(array);

    if (_ensureRemaining(array, 1) == NULL) {
        trapOutOfMemory("Cannot increase space for PARCArrayList.");
    }
    array->array[array->numberOfElements++] = (void *) pointer;

    return true;
}

PARCArrayList *
parcArrayList_AddAll(PARCArrayList *array, void *argv[], size_t argc)
{
    for (size_t i = 0; i < argc; i++) {
        parcArrayList_Add(array, argv[i]);
    }

    return array;
}

bool
parcArrayList_IsEmpty(const PARCArrayList *list)
{
    parcArrayList_OptionalAssertValid(list);

    return list->numberOfElements == 0;
}

bool
parcArrayList_Equals(const PARCArrayList *a, const PARCArrayList *b)
{
    bool result = true;

    if (a != b) {
        if (a == NULL || b == NULL) {
            result = false;
        } else if (a->numberOfElements != b->numberOfElements) {
            result = false;
        } else {
            for (size_t i = 0; i < a->numberOfElements; i++) {
                if (a->equalsElement != NULL) {
                    if (!a->equalsElement(a->array[i], b->array[i])) {
                        result = false;
                        break;
                    }
                } else {
                    if (a->array[i] != b->array[i]) {
                        result = false;
                        break;
                    }
                }
            }
        }
    }

    return result;
}

void *
parcArrayList_RemoveAtIndex(PARCArrayList *array, size_t index)
{
    trapOutOfBoundsIf(index >= array->numberOfElements, "Index must be within the range [0, %zu)", array->numberOfElements);

    void *element = array->array[index];

    // Adjust the list to elide the element.
    for (size_t i = index; i < array->numberOfElements - 1; i++) {
        array->array[i] = array->array[i + 1];
    }
    array->numberOfElements--;

    return element;
}

void
parcArrayList_Set(const PARCArrayList *array, size_t index, void *pointer)
{
    trapOutOfBoundsIf(index >= array->numberOfElements, "Index must be within the range [0, %zu)", array->numberOfElements);

    array->array[index] = pointer;
}

void *
parcArrayList_Get(const PARCArrayList *array, size_t index)
{
    trapOutOfBoundsIf(index >= array->numberOfElements, "Index must be within the range [0, %zu)", array->numberOfElements);

    return array->array[index];
}

void *
parcArrayList_Peek(const PARCArrayList *list)
{
    return parcArrayList_Get(list, parcArrayList_Size(list) - 1);
}

size_t
parcArrayList_Size(const PARCArrayList *pointerArray)
{
    return pointerArray->numberOfElements;
}

void
parcArrayList_Destroy(PARCArrayList **arrayPtr)
{
    assertNotNull(arrayPtr, "Parameter must be a non-null pointer to a PARC_ArrayList pointer.");

    PARCArrayList *array = *arrayPtr;

    parcArrayList_OptionalAssertValid(array);

    assertTrue(array->numberOfElements == 0 ? true : array->array != NULL, "PARC_ArrayList is inconsistent.");

    if (array->destroyElement != NULL) {
        for (size_t i = 0; i < array->numberOfElements; i++) {
            if (array->array[i] != NULL) {
                array->destroyElement(&array->array[i]);
            }
        }
    }

    if (array->array != NULL) {
        parcMemory_Deallocate((void **) &(array->array));
    }

    parcMemory_Deallocate((void **) &array);
    *arrayPtr = 0;
}

PARCArrayList *
parcArrayList_Copy(const PARCArrayList *original)
{
    parcArrayList_OptionalAssertValid(original);

    PARCArrayList *result = parcArrayList_Create(original->destroyElement);

    if (result != NULL) {
        for (size_t i = 0; i < original->numberOfElements; i++) {
            parcArrayList_Add(result, original->array[i]);
        }
    }

    return result;
}

void
parcArrayList_StdlibFreeFunction(void **elementPtr)
{
    if (elementPtr != NULL) {
        free(*elementPtr);
        *elementPtr = 0;
    }
}

void
parcArrayList_PARCMemoryFreeFunction(void **elementPtr)
{
    if (elementPtr != NULL) {
        parcMemory_Deallocate((void **) elementPtr);
        *elementPtr = 0;
    }
}

PARCArrayList *
parcArrayList_RemoveAndDestroyAtIndex(PARCArrayList *array, size_t index)
{
    parcArrayList_OptionalAssertValid(array);

    assertTrue(index < array->numberOfElements, "Index must be ( 0 <= index < %zd). Actual=%zd", array->numberOfElements, index);

    if (index < array->numberOfElements) {
        // Destroy the element at the given index.
        if (array->destroyElement != NULL) {
            array->destroyElement(&array->array[index]);
        }

        // Adjust the list to elide the element.
        for (size_t i = index; i < array->numberOfElements - 1; i++) {
            array->array[i] = array->array[i + 1];
        }
        array->numberOfElements--;
    }

    return array;
}

PARCArrayList *
parcArrayList_InsertAtIndex(PARCArrayList *array, size_t index, const void *pointer)
{
    parcArrayList_OptionalAssertValid(array);
    size_t length = parcArrayList_Size(array);

    assertTrue(index <= array->numberOfElements, "You can't insert beyond the end of the list");

    // Create space and grow the array if needed
    _ensureRemaining(array, length + 1);
    for (size_t i = index; i < length; i++) {
        array->array[i + 1] = array->array[i];
    }
    array->numberOfElements++;

    array->array[index] = (void *) pointer;

    return array;
}

void *
parcArrayList_Pop(PARCArrayList *array)
{
    return parcArrayList_RemoveAtIndex(array, parcArrayList_Size(array) - 1);
}

void
parcArrayList_Clear(PARCArrayList *array)
{
    while (!parcArrayList_IsEmpty(array)) {
        parcArrayList_RemoveAndDestroyAtIndex(array, parcArrayList_Size(array) - 1);
    }
}
