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
#include <config.h>
#include <stdio.h>
#include <pthread.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventSignal.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventSignal.c"

LONGBOW_TEST_RUNNER(parc_EventSignal)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventSignal)
{
    parcEventSignal_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventSignal)
{
    parcEventSignal_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventSignal_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parc_EventSignal_Start_Stop);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static int _empty_event_called = 0;

static void
_empty_event(int fd, PARCEventType flags, void *data)
{
    _empty_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _empty_event, NULL);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    _parc_event_signal_callback(0, 0, (void *)parcEventSignal);
    assertTrue(_empty_event_called == 1, "Event handler never called.");
    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

#include <pthread.h>

static void *
_run_scheduler(void *data)
{
    PARCEventScheduler *parcEventScheduler = (PARCEventScheduler *) data;
    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    return NULL;
}

static int _test_event_called = 0;

static void
_signal_event(int fd, PARCEventType flags, void *data)
{
    PARCEventSignal **parcEventSignal = (PARCEventSignal **) data;
    _test_event_called++;
    parcEventSignal_Stop(*parcEventSignal);
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Start_Stop)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _signal_event, &parcEventSignal);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    parcEventSignal_Start(parcEventSignal);

    pthread_t thread;
    pthread_create(&thread, NULL, _run_scheduler, parcEventScheduler);

    kill(getpid(), SIGUSR1);
    pthread_join(thread, NULL);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

static void
_test_stop_event(int fd, PARCEventType flags, void *data)
{
    PARCEventSignal **parcEventSignal = (PARCEventSignal **) data;
    _test_event_called++;
    parcEventSignal_Stop(*parcEventSignal);
}

LONGBOW_TEST_CASE(Global, parc_EventSignal_Stop)
{
    _test_event_called = 0;
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");
    PARCEventSignal *parcEventSignal = parcEventSignal_Create(parcEventScheduler, SIGUSR1, PARCEventType_Signal | PARCEventType_Persist, _test_stop_event, &parcEventSignal);
    assertNotNull(parcEventSignal, "parcEventSignal_Create returned a null reference");

    parcEventSignal_Start(parcEventSignal);
    kill(getpid(), SIGUSR1);

    parcEventScheduler_Start(parcEventScheduler, PARCEventSchedulerDispatchType_Blocking);
    assertTrue(_test_event_called == 1, "Event never called.");

    parcEventSignal_Destroy(&parcEventSignal);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventSignal);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
