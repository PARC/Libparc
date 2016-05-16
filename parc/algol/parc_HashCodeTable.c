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
 * Implements an open-addressing hash table.  We use linear probing of +1 per step.
 *
 * Table is rehashed when we reach 75% utilization.
 * The table is rehashed if we go more than 10 linear probes without being able to insert.
 *
 * HashCodeTable is a wrapper that holds the key/data management functions.  It also
 * has LinearAddressingHashTable that is the actual hash table.
 *
 * This open-addressing table is inefficient for GET or DEL if the element does not exist.
 * The whole table needs to be
 *
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <string.h>

#include <parc/algol/parc_HashCodeTable.h>
#include <parc/algol/parc_Memory.h>

// minimum size if nothing specified
#define MIN_SIZE 256

// when we expand, use this factor
#define EXPAND_FACTOR   2

#define MAX_PROBE_LENGTH 20

typedef enum {
    ADD_OK,     // we added the key
    ADD_DUP,    // the key is a duplicate
    ADD_NOSPACE // ran out of space
} PARCHashCodeTable_AddResult;

typedef struct hashtable_entry {
    // A hashtable entry is in use if the key is non-null
    void *key;
    void *data;
    HashCodeType hashcode;
} HashTableEntry;

typedef struct linear_address_hash_table {
    HashTableEntry  *entries;

    // Number of elements allocated
    size_t tableLimit;

    // Number of elements in use
    size_t tableSize;

    // When the tableSize equals or exceeds this
    // threshold, we should expand and re-hash the table∫
    size_t expandThreshold;
} LinearAddressingHashTable;

struct parc_hashcode_table {
    LinearAddressingHashTable hashtable;

    PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc;
    PARCHashCodeTable_HashCodeFunc keyHashCodeFunc;
    PARCHashCodeTable_Destroyer keyDestroyer;
    PARCHashCodeTable_Destroyer dataDestroyer;

    unsigned expandCount;
};

static bool
_findIndex(PARCHashCodeTable *table, const void *key, size_t *outputIndexPtr)
{
    size_t index, start;
    HashCodeType hashcode;
    LinearAddressingHashTable *innerTable;

    innerTable = &table->hashtable;
    hashcode = table->keyHashCodeFunc(key);
    index = hashcode % innerTable->tableLimit;
    start = index;


    // check until we've gone MAX_PROBE_LENGTH
    unsigned steps = 0;
    do {
        if (innerTable->entries[index].key != NULL) {
            if ((innerTable->entries[index].hashcode == hashcode) && table->keyEqualsFunc(key, innerTable->entries[index].key)) {
                // the key already exists in the table
                *outputIndexPtr = index;
                return true;
            }
        }
        steps++;
        index = index + 1;
        if (index == innerTable->tableLimit) {
            index = 0;
        }
    } while (index != start && steps < MAX_PROBE_LENGTH);

    return false;
}

static PARCHashCodeTable_AddResult
_innerTableAdd(LinearAddressingHashTable *innerTable, PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
               HashCodeType hashcode, void *key, void *data)
{
    size_t index = hashcode % innerTable->tableLimit;

    unsigned steps = 0;

    // we know the size < limit, so it will fit eventually
    while (steps < MAX_PROBE_LENGTH) {
        if (innerTable->entries[index].key == NULL) {
            innerTable->entries[index].hashcode = hashcode;
            innerTable->entries[index].key = key;
            innerTable->entries[index].data = data;
            innerTable->tableSize++;
            return ADD_OK;
        }

        if ((innerTable->entries[index].hashcode == hashcode) && keyEqualsFunc(key, innerTable->entries[index].key)) {
            // the key already exists in the table
            return ADD_DUP;
        }

        steps++;
        index = index + 1;
        if (index == innerTable->tableLimit) {
            index = 0;
        }
    }

    return ADD_NOSPACE;
}

static PARCHashCodeTable_AddResult
_rehash(LinearAddressingHashTable *old_table, LinearAddressingHashTable *new_table, PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc)
{
    size_t i;
    for (i = 0; i < old_table->tableLimit; i++) {
        if (old_table->entries[i].key != NULL) {
            PARCHashCodeTable_AddResult result = _innerTableAdd(new_table, keyEqualsFunc, old_table->entries[i].hashcode,
                                                                old_table->entries[i].key, old_table->entries[i].data);
            if (result != ADD_OK) {
                return result;
            }
        }
    }
    return ADD_OK;
}

static void
_expand(PARCHashCodeTable *hashCodeTable)
{
    LinearAddressingHashTable temp_table;
    LinearAddressingHashTable *old_table = &hashCodeTable->hashtable;

    size_t expandby = EXPAND_FACTOR;

    // start with a copy of the current table
    PARCHashCodeTable_AddResult result = ADD_OK;
    do {
        hashCodeTable->expandCount++;

        temp_table.tableSize = 0;
        temp_table.tableLimit = old_table->tableLimit * expandby;
        temp_table.expandThreshold = temp_table.tableLimit - temp_table.tableLimit / 4;
        temp_table.entries = parcMemory_AllocateAndClear(temp_table.tableLimit * sizeof(HashTableEntry));
        assertNotNull(temp_table.entries, "parcMemory_AllocateAndClear(%zu) returned NULL", temp_table.tableLimit * sizeof(HashTableEntry));

        result = _rehash(old_table, &temp_table, hashCodeTable->keyEqualsFunc);
        if (result == ADD_NOSPACE) {
            // could not rehash, so expand by more and try again
            parcMemory_Deallocate((void **) &(temp_table.entries));
            expandby++;
        }
    } while (result == ADD_NOSPACE);

    parcMemory_Deallocate((void **) &old_table->entries);
    hashCodeTable->hashtable = temp_table;
}

PARCHashCodeTable *
parcHashCodeTable_Create_Size(PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
                              PARCHashCodeTable_HashCodeFunc keyHashCodeFunc,
                              PARCHashCodeTable_Destroyer keyDestroyer,
                              PARCHashCodeTable_Destroyer dataDestroyer,
                              size_t minimumSize)
{
    PARCHashCodeTable *table = parcMemory_AllocateAndClear(sizeof(PARCHashCodeTable));
    assertNotNull(table, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCHashCodeTable));

    assertNotNull(keyEqualsFunc, "keyEqualsFunc must be non-null");
    assertNotNull(keyHashCodeFunc, "keyHashCodeFunc must be non-null");
    assertTrue(minimumSize > 0, "minimumSize must be greater than zero");

    table->keyEqualsFunc = keyEqualsFunc;
    table->keyHashCodeFunc = keyHashCodeFunc;
    table->keyDestroyer = keyDestroyer;
    table->dataDestroyer = dataDestroyer;

    table->hashtable.entries = parcMemory_AllocateAndClear(minimumSize * sizeof(HashTableEntry));
    assertNotNull(table->hashtable.entries, "parcMemory_AllocateAndClear(%zu) returned NULL", minimumSize * sizeof(HashTableEntry));
    table->hashtable.tableLimit = minimumSize;
    table->hashtable.tableSize = 0;

    memset(table->hashtable.entries, 0, minimumSize * sizeof(HashTableEntry));

    // expand at 75% utilization
    table->hashtable.expandThreshold = minimumSize - minimumSize / 4;

    return table;
}

PARCHashCodeTable *
parcHashCodeTable_Create(PARCHashCodeTable_KeyEqualsFunc keyEqualsFunc,
                         PARCHashCodeTable_HashCodeFunc keyHashCodeFunc,
                         PARCHashCodeTable_Destroyer keyDestroyer,
                         PARCHashCodeTable_Destroyer dataDestroyer)
{
    return parcHashCodeTable_Create_Size(keyEqualsFunc, keyHashCodeFunc, keyDestroyer, dataDestroyer, MIN_SIZE);
}

void
parcHashCodeTable_Destroy(PARCHashCodeTable **tablePtr)
{
    assertNotNull(tablePtr, "Parameter must be non-null double pointer");
    assertNotNull(*tablePtr, "Parameter must dereference to non-null pointer");
    PARCHashCodeTable *table = *tablePtr;
    size_t i;

    for (i = 0; i < table->hashtable.tableLimit; i++) {
        if (table->hashtable.entries[i].key != NULL) {
            if (table->keyDestroyer) {
                table->keyDestroyer(&table->hashtable.entries[i].key);
            }

            if (table->dataDestroyer) {
                table->dataDestroyer(&table->hashtable.entries[i].data);
            }
        }
    }

    parcMemory_Deallocate((void **) &(table->hashtable.entries));
    parcMemory_Deallocate((void **) &table);
    *tablePtr = NULL;
}

bool
parcHashCodeTable_Add(PARCHashCodeTable *table, void *key, void *data)
{
    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(key, "Parameter key must be non-null");
    assertNotNull(data, "Parameter data must be non-null");

    if (table->hashtable.tableSize >= table->hashtable.expandThreshold) {
        _expand(table);
    }

    HashCodeType hashcode = table->keyHashCodeFunc(key);

    PARCHashCodeTable_AddResult result = ADD_OK;
    do {
        result = _innerTableAdd(&table->hashtable, table->keyEqualsFunc, hashcode, key, data);
        if (result == ADD_NOSPACE) {
            _expand(table);
        }
    } while (result == ADD_NOSPACE);

    return (result == ADD_OK);
}

void
parcHashCodeTable_Del(PARCHashCodeTable *table, const void *key)
{
    size_t index;
    bool found;

    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(key, "parameter key must be non-null");

    found = _findIndex(table, key, &index);

    if (found) {
        assertTrue(table->hashtable.tableSize > 0, "Illegal state: found entry in a hash table with 0 size");

        if (table->keyDestroyer) {
            table->keyDestroyer(&table->hashtable.entries[index].key);
        }

        if (table->dataDestroyer) {
            table->dataDestroyer(&table->hashtable.entries[index].data);
        }

        memset(&table->hashtable.entries[index], 0, sizeof(HashTableEntry));

        table->hashtable.tableSize--;
    }
}

void *
parcHashCodeTable_Get(PARCHashCodeTable *table, const void *key)
{
    size_t index;

    assertNotNull(table, "Parameter table must be non-null");
    assertNotNull(key, "parameter key must be non-null");

    bool found = _findIndex(table, key, &index);

    if (found) {
        return table->hashtable.entries[index].data;
    }

    return NULL;
}

size_t
parcHashCodeTable_Length(const PARCHashCodeTable *table)
{
    assertNotNull(table, "Parameter table must be non-null");
    return table->hashtable.tableSize;
}
