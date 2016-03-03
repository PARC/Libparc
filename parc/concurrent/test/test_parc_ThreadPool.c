/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * Copyright 2015 Palo Alto Research Center, Inc. (PARC), a Xerox company.  All Rights Reserved.
 * The content of this file, whole or in part, is subject to licensing terms.
 * If distributing this software, include this License Header Notice in each
 * file and provide the accompanying LICENSE file.
 */
/**
 * @author <#gscott#>, Computing Science Laboratory, PARC
 * @copyright 2015 Palo Alto Research Center, Inc. (PARC), A Xerox Company.  All Rights Reserved.
 */
#include "../parc_ThreadPool.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_ThreadPool)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Object);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_ThreadPool)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_ThreadPool)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    assertNotNull(instance, "Expected non-null result from parcThreadPool_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcThreadPool_Acquire, instance);
    
    parcThreadPool_Release(&instance);
    assertNull(instance, "Expected null result from parcThreadPool_Release();");
}

LONGBOW_TEST_FIXTURE(Object)
{
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Compare);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Copy);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Display);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_Equals);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_HashCode);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_IsValid);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_ToJSON);
    LONGBOW_RUN_TEST_CASE(Object, parcThreadPool_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Object)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Object)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Object,  parcThreadPool_Compare)
{
    testUnimplemented("");

}

LONGBOW_TEST_CASE(Object, parcThreadPool_Copy)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    PARCThreadPool *copy = parcThreadPool_Copy(instance);
    assertTrue(parcThreadPool_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcThreadPool_Release(&instance);
    parcThreadPool_Release(&copy);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Display)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    parcThreadPool_Display(instance, 0);
    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_Equals)
{
    PARCThreadPool *x = parcThreadPool_Create(6);
    PARCThreadPool *y = parcThreadPool_Create(6);
    PARCThreadPool *z = parcThreadPool_Create(6);

    parcObjectTesting_AssertEquals(x, y, z, NULL);

    parcThreadPool_Release(&x);
    parcThreadPool_Release(&y);
    parcThreadPool_Release(&z);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_HashCode)
{
    PARCThreadPool *x = parcThreadPool_Create(6);
    PARCThreadPool *y = parcThreadPool_Create(6);
    
    parcObjectTesting_AssertHashCode(x, y);
    
    parcThreadPool_Release(&x);
    parcThreadPool_Release(&y);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_IsValid)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    assertTrue(parcThreadPool_IsValid(instance), "Expected parcThreadPool_Create to result in a valid instance.");
    
    parcThreadPool_Release(&instance);
    assertFalse(parcThreadPool_IsValid(instance), "Expected parcThreadPool_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Object, parcThreadPool_ToJSON)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    
    PARCJSON *json = parcThreadPool_ToJSON(instance);

    parcJSON_Release(&json);

    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_CASE(Object, parcThreadPool_ToString)
{
    PARCThreadPool *instance = parcThreadPool_Create(6);
    
    char *string = parcThreadPool_ToString(instance);
    
    assertNotNull(string, "Expected non-NULL result from parcThreadPool_ToString");
    
    parcMemory_Deallocate((void **) &string);
    parcThreadPool_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Specialization)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_ThreadPool);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}

