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
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_RingBuffer_1x1.c"

#include <sys/time.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_RingBuffer_1x1)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_RingBuffer_1x1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_RingBuffer_1x1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Create_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Create_NonPower2);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Get_Put);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Empty);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Half);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Full);
    LONGBOW_RUN_TEST_CASE(Global, parcRingBuffer1x1_Put_ToCapacity);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ------
typedef struct test_ringbuffer {
    unsigned itemsToWrite;
    volatile unsigned itemsWritten;
    volatile unsigned itemsRead;
    volatile bool blocked;

    PARCRingBuffer1x1 *producerBuffer;
    PARCRingBuffer1x1 *consumerBuffer;

    pthread_t producerThread;
    pthread_t consumerThread;
} TestRingBuffer;


void *
consumer(void *p)
{
    TestRingBuffer *trb = (TestRingBuffer *) p;

    while (trb->blocked) {
        // nothing to do.
    }

    while (trb->itemsRead < trb->itemsToWrite) {
        uint32_t *data;
        bool success = parcRingBuffer1x1_Get(trb->consumerBuffer, (void **) &data);
        if (success) {
            assertTrue(*data == trb->itemsRead, "Got out of order item %u expected %u\n", *data, trb->itemsRead);
            parcMemory_Deallocate((void **) &data);
            trb->itemsRead++;
        }
    }

    pthread_exit((void *) NULL);
}

void *
producer(void *p)
{
    TestRingBuffer *trb = (TestRingBuffer *) p;

    while (trb->blocked) {
        // nothing to do
    }

    while (trb->itemsWritten < trb->itemsToWrite) {
        uint32_t *data = parcMemory_Allocate(sizeof(uint32_t));
        assertNotNull(data, "parcMemory_Allocate(%zu) returned NULL", sizeof(uint32_t));
        *data = trb->itemsWritten;

        bool success = false;
        do {
            success = parcRingBuffer1x1_Put(trb->producerBuffer, data);
        } while (!success);
        trb->itemsWritten++;
    }

    pthread_exit((void *) NULL);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Acquire)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE_EXPECTS(Global, parcRingBuffer1x1_Create_NonPower2, .event = &LongBowAssertEvent)
{
    // this will assert because the number of elements is not a power of 2
    parcRingBuffer1x1_Create(3, NULL);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Create_Release)
{
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(1024, NULL);
    parcRingBuffer1x1_Release(&ring);
    assertTrue(parcMemory_Outstanding() == 0, "Non-zero memory balance: %u", parcMemory_Outstanding());

    printf("ring buffer entry size: %zu\n", sizeof(PARCRingBuffer1x1));
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Get_Put)
{
    TestRingBuffer *trb = parcMemory_AllocateAndClear(sizeof(TestRingBuffer));
    assertNotNull(trb, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestRingBuffer));
    trb->producerBuffer = parcRingBuffer1x1_Create(128, NULL);
    trb->consumerBuffer = parcRingBuffer1x1_Acquire(trb->producerBuffer);

    trb->itemsToWrite = 100000;
    trb->blocked = true;

    pthread_create(&trb->consumerThread, NULL, consumer, trb);
    pthread_create(&trb->producerThread, NULL, producer, trb);

    struct timeval t0, t1;

    gettimeofday(&t0, NULL);
    trb->blocked = false;

    // wait for them to exit
    pthread_join(trb->producerThread, NULL);
    pthread_join(trb->consumerThread, NULL);
    gettimeofday(&t1, NULL);

    timersub(&t1, &t0, &t1);

    assertTrue(trb->itemsWritten == trb->itemsToWrite,
               "Did not write all items got %u expected %u\n",
               trb->itemsWritten,
               trb->itemsToWrite);

    assertTrue(trb->itemsRead == trb->itemsToWrite,
               "Did not read all items got %u expected %u\n",
               trb->itemsRead,
               trb->itemsToWrite);

    double sec = t1.tv_sec + t1.tv_usec * 1E-6;

    printf("Passed %u items in %.6f seconds, %.2f items/sec\n",
           trb->itemsWritten,
           sec,
           trb->itemsWritten / sec);

    parcRingBuffer1x1_Release(&trb->consumerBuffer);
    parcRingBuffer1x1_Release(&trb->producerBuffer);
    parcMemory_Deallocate((void **) &trb);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Empty)
{
    uint32_t capacity = 128;
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(capacity, NULL);
    uint32_t remaining = parcRingBuffer1x1_Remaining(ring);
    parcRingBuffer1x1_Release(&ring);

    // -1 because the ring buffer is always -1
    assertTrue(remaining == capacity - 1, "Got wrong remaining, got %u expecting %u\n", remaining, capacity);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Half)
{
    uint32_t capacity = 128;
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(capacity, NULL);
    for (int i = 0; i < capacity / 2; i++) {
        parcRingBuffer1x1_Put(ring, &i);
    }

    uint32_t remaining = parcRingBuffer1x1_Remaining(ring);
    parcRingBuffer1x1_Release(&ring);

    // -1 because the ring buffer is always -1
    assertTrue(remaining == capacity / 2 - 1, "Got wrong remaining, got %u expecting %u\n", remaining, capacity / 2 - 1);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Remaining_Full)
{
    uint32_t capacity = 128;
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(capacity, NULL);
    for (int i = 0; i < capacity - 1; i++) {
        parcRingBuffer1x1_Put(ring, &i);
    }

    uint32_t remaining = parcRingBuffer1x1_Remaining(ring);
    parcRingBuffer1x1_Release(&ring);

    assertTrue(remaining == 0, "Got wrong remaining, got %u expecting %u\n", remaining, 0);
}

LONGBOW_TEST_CASE(Global, parcRingBuffer1x1_Put_ToCapacity)
{
    uint32_t capacity = 128;
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(capacity, NULL);
    for (int i = 0; i < capacity - 1; i++) {
        parcRingBuffer1x1_Put(ring, &i);
    }

    // this next put should fail
    bool success = parcRingBuffer1x1_Put(ring, &capacity);

    parcRingBuffer1x1_Release(&ring);

    assertFalse(success, "Should have failed on final put because data structure is full\n");
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _create);
    LONGBOW_RUN_TEST_CASE(Local, _destroy);
    LONGBOW_RUN_TEST_CASE(Local, _isPowerOfTwo);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _create)
{
    testUnimplemented("");
}

static void
_testDestoryer(void **ptr)
{
    parcBuffer_Release((PARCBuffer **) ptr);
}

LONGBOW_TEST_CASE(Local, _destroy)
{
    // put something in the ring and don't remove it.  Make sure the destroyer catches it.

    uint32_t capacity = 128;
    PARCRingBuffer1x1 *ring = parcRingBuffer1x1_Create(capacity, _testDestoryer);

    PARCBuffer *buffer = parcBuffer_Allocate(5);
    parcRingBuffer1x1_Put(ring, buffer);

    parcRingBuffer1x1_Release(&ring);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance, expected 0 got %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Local, _isPowerOfTwo)
{
    struct test_struct {
        uint32_t value;
        bool isPow2;
    } test_vector[] = { { 0, false }, { 1, true }, { 2, true }, { 15, false }, { 16, true }, { 32, true }, { UINT32_MAX, true } };

    for (int i = 0; test_vector[i].value != UINT32_MAX; i++) {
        bool test = _isPowerOfTwo(test_vector[i].value);
        assertTrue(test == test_vector[i].isPow2, "Got wrong result for value %u, got %d expected %d\n", test_vector[i].value, test, test_vector[i].isPow2);
    }
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_RingBuffer_1x1);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
