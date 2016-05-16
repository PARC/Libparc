/*
 * Copyright (c) 2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Iterator.c"

#include <inttypes.h>
#include <stdio.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_Iterator)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Iterator)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Iterator)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, parcIterator_CreateAcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s", longBowTestCase_GetName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static uint64_t _state;

static void *
init(PARCObject *object __attribute__((unused)))
{
    _state = 0;
    return &_state;
}

static bool
hasNext(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;
    return (*value < 5);
}

static void *
next(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;

    (*value)++;
    return state;
}

static void
removex(PARCObject *object __attribute__((unused)), void **state)
{
}

static void *
getElement(PARCObject *object __attribute__((unused)), void *state)
{
    uint64_t *value = (uint64_t *) state;
    return (void *) *value;
}

static void
fini(PARCObject *object __attribute__((unused)), void *state __attribute__((unused)))
{

}

static void
assertValid(const void *state __attribute__((unused)))
{

}

LONGBOW_TEST_CASE(CreateAcquireRelease, parcIterator_CreateAcquireRelease)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    parcObjectTesting_AssertAcquireReleaseContract(parcIterator_Acquire, iterator);
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcIterator_HasNext);
    LONGBOW_RUN_TEST_CASE(Global, parcIterator_Next);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    bool leaked = parcMemoryTesting_ExpectedOutstanding(0, "%s leaks memory \n", longBowTestCase_GetName(testCase)) != true;
    if (leaked) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcIterator_HasNext)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    while (parcIterator_HasNext(iterator)) {
        uint64_t value = (uint64_t) parcIterator_Next(iterator);
        printf("%" PRIu64 "\n", value);
    }
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_CASE(Global, parcIterator_Next)
{
    PARCBuffer *buffer = parcBuffer_Allocate(1);

    PARCIterator *iterator = parcIterator_Create(buffer, init, hasNext, next, removex, getElement, fini, assertValid);

    while (parcIterator_HasNext(iterator)) {
        uint64_t value = (uint64_t) parcIterator_Next(iterator);
        printf("%" PRIu64 "\n", value);
    }
    parcBuffer_Release(&buffer);
    parcIterator_Release(&iterator);
}

LONGBOW_TEST_CASE(Local, _finalize)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Iterator);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
