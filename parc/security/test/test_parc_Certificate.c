/*
 * Copyright (c) 2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
#include <LongBow/unit-test.h>

#include "../parc_Certificate.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_Key.h>
#include <parc/algol/parc_Buffer.h>

PARCCryptoHash *
_mockGetPublicKeyDigest(void *instance)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
    parcBuffer_Release(&buffer);
    return hash;
}

PARCCryptoHash *
_mockGetCertificateDigest(void *instance)
{
    PARCBuffer *buffer = parcBuffer_Allocate(20);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, buffer);
    parcBuffer_Release(&buffer);
    return hash;
}

PARCBuffer *
_mockGetDEREncodedCertificate(void *instance)
{
    return parcBuffer_Allocate(30);
}

PARCBuffer *
_mockGetDEREncodedPublicKey(void *instance)
{
    return parcBuffer_Allocate(40);
}

PARCCertificateType
_mockGetCertificateType(const void *instance)
{
    return PARCCertificateType_X509;
}

PARCContainerEncoding
_mockGetContainerEncoding(const void *instance)
{
    return PARCContainerEncoding_PEM;
}

PARCKey *
_mockGetPublicKey(void *instance)
{
    PARCBuffer *realKey = parcBuffer_Allocate(256);
    PARCBuffer *keyId = parcBuffer_Allocate(256);
    PARCKeyId *id = parcKeyId_Create(keyId);

    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(id, PARCSigningAlgorithm_RSA, realKey);

    parcBuffer_Release(&keyId);
    parcKeyId_Release(&id);
    parcBuffer_Release(&realKey);

    return key;
}

PARCCertificateInterface *_mockCertificate = &(PARCCertificateInterface) {
    .GetPublicKeyDigest       = _mockGetPublicKeyDigest,
    .GetCertificateDigest     = _mockGetCertificateDigest,
    .GetDEREncodedCertificate = _mockGetDEREncodedCertificate,
    .GetDEREncodedPublicKey   = _mockGetDEREncodedPublicKey,
    .GetCertificateType       = _mockGetCertificateType,
    .GetContainerEncoding     = _mockGetContainerEncoding
};

LONGBOW_TEST_RUNNER(parc_Certificate)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_Certificate)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Certificate)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_Create);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetContainerEncoding);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetCertificateType);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetPublicKeyDigest);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetCertificateDigest);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetDEREncodedCertificate);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetDEREncodedPublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parc_Certificate_GetPublicKey);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parc_Certificate_AcquireRelease)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);
    assertNotNull(certificate, "Expected non-NULL certificate");

    PARCReferenceCount firstCount = parcObject_GetReferenceCount(certificate);
    PARCCertificate *copy = parcCertificate_Acquire(certificate);
    PARCReferenceCount secondCount = parcObject_GetReferenceCount(copy);

    assertTrue(firstCount == (secondCount - 1), "Expected incremented reference count after Acquire");

    parcCertificate_Release(&copy);
    PARCReferenceCount thirdCount = parcObject_GetReferenceCount(certificate);

    assertTrue(firstCount == thirdCount, "Expected equal reference counts after Release");

    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_Create)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    assertNotNull(certificate, "Expected non-NULL certificate");

    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetContainerEncoding)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);
    PARCContainerEncoding encoding = parcCertificate_GetContainerEncoding(certificate);
    assertTrue(encoding == PARCContainerEncoding_PEM, "Expected %d, got %d", PARCContainerEncoding_PEM, encoding);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetCertificateType)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);
    PARCCertificateType type = parcCertificate_GetCertificateType(certificate);
    assertTrue(type == PARCCertificateType_X509, "Expected %d, got %d", PARCCertificateType_X509, type);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetPublicKeyDigest)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    PARCCryptoHash *hash = parcCertificate_GetPublicKeyDigest(certificate);
    size_t length = parcBuffer_Remaining(parcCryptoHash_GetDigest(hash));
    size_t expected = 10;
    assertTrue(length == expected, "Expected length of %zu, got %zu", expected, length);

    parcCryptoHash_Release(&hash);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetCertificateDigest)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    PARCCryptoHash *hash = parcCertificate_GetCertificateDigest(certificate);
    size_t length = parcBuffer_Remaining(parcCryptoHash_GetDigest(hash));
    size_t expected = 20;
    assertTrue(length == expected, "Expected length of %zu, got %zu", expected, length);

    parcCryptoHash_Release(&hash);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetDEREncodedCertificate)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    PARCBuffer *buffer = parcCertificate_GetDEREncodedCertificate(certificate);
    size_t length = parcBuffer_Remaining(buffer);
    size_t expected = 30;
    assertTrue(length == expected, "Expected length of %zu, got %zu", expected, length);

    parcBuffer_Release(&buffer);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetDEREncodedPublicKey)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    PARCBuffer *buffer = parcCertificate_GetDEREncodedPublicKey(certificate);
    size_t length = parcBuffer_Remaining(buffer);
    size_t expected = 40;
    assertTrue(length == expected, "Expected length of %zu, got %zu", expected, length);

    parcBuffer_Release(&buffer);
    parcCertificate_Release(&certificate);
}

LONGBOW_TEST_CASE(Global, parc_Certificate_GetPublicKey)
{
    PARCCertificate *certificate = parcCertificate_CreateFromInstance(_mockCertificate, NULL);

    PARCKey *actual = parcCertificate_GetPublicKey(certificate);
    assertTrue(parcBuffer_Remaining(parcKey_GetKey(actual)) == 40,
               "Expected PARCKey size to be 40, got %zu",
               parcBuffer_Remaining(parcKey_GetKey(actual)));
    assertTrue(parcBuffer_Remaining(parcKeyId_GetKeyId(parcKey_GetKeyId(actual))) == 10,
               "Expected PARCKey keyId size to be 10, got %zu",
               parcBuffer_Remaining(parcKeyId_GetKeyId(parcKey_GetKeyId(actual))));

    parcKey_Release(&actual);
    parcCertificate_Release(&certificate);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Certificate);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
