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
 * The dictionary is implemented with the parc_HashCodeTable backend.  This implementation
 * is inefficient for additions with duplicate keys, because the semantics of parc_HashCodeTable
 * are not the same as parc_BufferDictionary in returning values for Put and Remove.
 *
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_BufferDictionary.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashCodeTable.h>

struct parc_buffer_dictionary {
    PARCHashCodeTable *hashtable;
};

// Wrapper to go from void * to PARCBuffer *
static bool
_parcBufferEquals(const void *a, const void *b)
{
    return parcBuffer_Equals((const PARCBuffer *) a, (const PARCBuffer *) b);
}

// Wrapper to go from void * to PARCBuffer *
static PARCHashCode
_parcBufferHashCode(const void *a)
{
    return parcBuffer_HashCode((const PARCBuffer *) a);
}

// Wrapper to go from void ** to PARCBuffer **
static void
_parcBufferRelease(void **bufferVoidPtr)
{
    parcBuffer_Release((PARCBuffer **) bufferVoidPtr);
}

/*
 * Initialise a parcBufferDictionary instance.
 * @return The same pointer as `result`.
 */
static PARCBufferDictionary *
_init(PARCBufferDictionary *result)
{
    result->hashtable = parcHashCodeTable_Create(_parcBufferEquals, _parcBufferHashCode, _parcBufferRelease, _parcBufferRelease);
    return result;
}

/**
 * Cleans up the internal memory of a PARCBufferDictionary
 *
 * @param [in] dictionaryPtr Double pointer to the dictionary to finalize
 */
static void
_destroy(PARCBufferDictionary **dictionaryPtr)
{
    assertNotNull(dictionaryPtr, "Double pointer dictionaryPtr must be non-NULL");
    assertNotNull(*dictionaryPtr, "Double pointer dictionaryPtr must dereference to non-NULL");

    PARCBufferDictionary *dict = *dictionaryPtr;

    parcHashCodeTable_Destroy(&dict->hashtable);
}


parcObject_ExtendPARCObject(PARCBufferDictionary, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCBufferDictionary *
parcBufferDictionary_Create(void)
{
    PARCBufferDictionary *result = parcObject_CreateInstance(PARCBufferDictionary);
    if (result != NULL) {
        return _init(result);
    }

    return NULL;
}

parcObject_ImplementAcquire(parcBufferDictionary, PARCBufferDictionary);

parcObject_ImplementRelease(parcBufferDictionary, PARCBufferDictionary);

PARCBuffer *
parcBufferDictionary_Put(PARCBufferDictionary *dictionary, PARCBuffer *key, PARCBuffer *value)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    assertNotNull(key, "Parameter key must be non-NULL");
    assertNotNull(value, "Parameter value must be non-NULL");

    PARCBuffer *oldValue = NULL;

    // We use reference counted copyies of the supplied parameters
    PARCBuffer *key_copy = parcBuffer_Acquire(key);
    PARCBuffer *value_copy = parcBuffer_Acquire(value);

    if (!parcHashCodeTable_Add(dictionary->hashtable, key_copy, value_copy)) {
        // parcHashCodeTable_Del will free the referece, to make a copy of it
        PARCBuffer *original = (PARCBuffer *) parcHashCodeTable_Get(dictionary->hashtable, key_copy);
        oldValue = parcBuffer_Acquire(original);
        parcHashCodeTable_Del(dictionary->hashtable, key_copy);
        parcHashCodeTable_Add(dictionary->hashtable, key_copy, value_copy);
    }

    return oldValue;
}

PARCBuffer *
parcBufferDictionary_Get(PARCBufferDictionary *dictionary, PARCBuffer *key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    assertNotNull(key, "Parameter key must be non-NULL");

    return parcHashCodeTable_Get(dictionary->hashtable, key);
}

PARCBuffer *
parcBufferDictionary_Remove(PARCBufferDictionary *dictionary, PARCBuffer *key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    assertNotNull(key, "Parameter key must be non-NULL");

    // parcHashCodeTable_Del will free the referece, to make a copy of it
    PARCBuffer *original = (PARCBuffer *) parcHashCodeTable_Get(dictionary->hashtable, key);
    PARCBuffer *oldValue = parcBuffer_Acquire(original);
    parcHashCodeTable_Del(dictionary->hashtable, key);
    return oldValue;
}
