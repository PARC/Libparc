/*
 * Copyright (c) 2014, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_KeyStore.c"
#include <parc/testing/parc_ObjectTesting.h>

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_KeyStore)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_KeyStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_KeyStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_Acquire);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_CreateFile);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_GetFileName);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_GetPassWord);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_Release);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

//LONGBOW_TEST_CASE(Global, parcKeyStore_Acquire)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcObjectTesting_AssertAcquireReleaseContract(parcKeyStore_Acquire, keyStoreFile);
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_CreateFile)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_GetFileName)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    assertEqualStrings(keystoreName, parcKeyStore_GetFileName(keyStoreFile));
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_GetPassWord)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    assertEqualStrings(keystorePassword, parcKeyStore_GetPassWord(keyStoreFile));
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_Release)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcKeyStore_Release(&keyStoreFile);
//    assertNull(keyStoreFile, "Key store File was not nulled out after Release()");
//}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_KeyStore);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
