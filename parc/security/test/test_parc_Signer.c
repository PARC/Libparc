/*
 * Copyright (c) 2013-2015, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
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
#include <config.h>
#include <LongBow/unit-test.h>
#include <sys/param.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Signer.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <errno.h>

//#include <parc/security/parc_PublicKeySignerPkcs12Store.h>
//#include <parc/security/parc_SymmetricSignerFileStore.h>

#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>
#include <parc/security/parc_SymmetricKeySigner.h>

#define FAKE_SIGNATURE "signature"

typedef struct {
    PARCCryptoHasher *hasher;
    PARCKeyStore *keyStore;
    PARCSymmetricKeyStore *privateKeyStore;
} _MockSigner;

static PARCSignature *
_SignDigest(PARCSigner *interfaceContext)
{
    PARCBuffer *buffer = parcBuffer_WrapCString(FAKE_SIGNATURE);
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARC_HASH_SHA256, buffer);
    parcBuffer_Release(&buffer);
    return signature;
}

static PARCSigningAlgorithm
_GetSigningAlgorithm(PARCSigner *interfaceContext)
{
    return PARCSigningAlgorithm_RSA;
}

static PARCCryptoHashType
_GetCryptoHashType(PARCSigner  *signer)
{
    return PARC_HASH_SHA256;
}

static PARCCryptoHasher *
_GetCryptoHasher(_MockSigner  *signer)
{
    return signer->hasher;
}

static PARCKeyStore *
_GetKeyStore(_MockSigner *signer)
{
    return signer->keyStore;
}

static bool
_releaseSigner(_MockSigner **signer)
{
    parcCryptoHasher_Release(&((*signer)->hasher));
    parcKeyStore_Release(&((*signer)->keyStore));
    return true;
}

parcObject_ImplementAcquire(_mockSigner, _MockSigner);
parcObject_ImplementRelease(_mockSigner, _MockSigner);

parcObject_Override(_MockSigner, PARCObject,
    .destructor = (PARCObjectDestructor *) _releaseSigner);

static _MockSigner *
_createSigner()
{
    _MockSigner *signer = parcObject_CreateInstance(_MockSigner);

    signer->hasher = parcCryptoHasher_Create(PARC_HASH_SHA256);

    PARCPkcs12KeyStore *publicKeyStore = parcPkcs12KeyStore_Open("test_rsa.p12", "blueberry", PARC_HASH_SHA256);
    assertNotNull(publicKeyStore, "Got null result from opening openssl pkcs12 file");
    signer->keyStore = parcKeyStore_Create(publicKeyStore, PARCPkcs12KeyStoreAsKeyStore);

    return signer;
}

static PARCSigningInterface *_MockSignerInterface = &(PARCSigningInterface) {
        .GetCryptoHasher = (PARCCryptoHasher *(*)(void *)) _GetCryptoHasher,
        .SignDigest = (PARCSignature *(*)(void *, const PARCCryptoHash *)) _SignDigest,
        .GetSigningAlgorithm = (PARCSigningAlgorithm (*)(void *)) _GetSigningAlgorithm,
        .GetCryptoHashType = (PARCCryptoHashType (*)(void *)) _GetCryptoHashType,
        .GetKeyStore = (PARCKeyStore *(*)(void *)) _GetKeyStore,
};

LONGBOW_TEST_RUNNER(parc_Signer)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Signer)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Signer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_CreateKeyId);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_CreatePublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetCryptoHasher);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_SignDigest);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetSigningAlgorithm);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetCryptoHashType);
    LONGBOW_RUN_TEST_CASE(Global, parcSigner_GetKeyStore);
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

LONGBOW_TEST_CASE(Global, parcSigner_Create)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    assertNotNull(signer, "Expected non-null signer");

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_AcquireRelease)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    assertNotNull(signer, "Expected non-null signer");

    parcObjectTesting_AssertAcquireReleaseContract(parcSigner_Acquire, signer);

    parcSigner_Release(&signer);
    assertNull(signer, "Expected null result from parcSigner_Release();");
}

LONGBOW_TEST_CASE(Global, parcSigner_CreateKeyId)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCKeyId *keyId = parcSigner_CreateKeyId(signer);

    assertNotNull(keyId, "Expected non-NULL PARCKeyId");

    parcKeyId_Release(&keyId);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_CreatePublicKey)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCKey *key = parcSigner_CreatePublicKey(signer);

    // Compute the real value
    PARCCryptoHash *hash = parcKeyStore_GetVerifierKeyDigest(mock->keyStore);
    PARCKeyId *keyid = parcKeyId_Create(parcCryptoHash_GetDigest(hash));
    PARCBuffer *derEncodedKey = parcKeyStore_GetDEREncodedPublicKey(mock->keyStore);

    PARCKey *expectedKey = parcKey_CreateFromDerEncodedPublicKey(keyid,
                                                         parcSigner_GetSigningAlgorithm(signer),
                                                         derEncodedKey);

    parcBuffer_Release(&derEncodedKey);
    parcKeyId_Release(&keyid);

    parcCryptoHash_Release(&hash);

    assertTrue(parcKey_Equals(key, expectedKey), "Expected public keys to be computed equally.");

    parcKey_Release(&key);
    parcKey_Release(&expectedKey);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetCryptoHasher)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);

    assertNotNull(hasher, "Expected non-NULL PARCCryptoHasher");

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_SignDigest)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCBuffer *buffer = parcBuffer_Allocate(10);
    PARCCryptoHash *hash = parcCryptoHash_Create(PARC_HASH_SHA256, buffer);
    PARCSignature *signature = parcSigner_SignDigest(signer, hash);

    assertNotNull(signature, "Expected non-NULL PARCSignature");

    PARCBuffer *bits = parcSignature_GetSignature(signature);
    char *bitstring = parcBuffer_ToString(bits);
    char *expectedString = FAKE_SIGNATURE;
    assertTrue(strcmp(bitstring, expectedString) == 0, "Expected the forced signature as output %s, got %s", expectedString, bitstring);
    parcMemory_Deallocate(&bitstring);

    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);
    parcSignature_Release(&signature);
    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetSigningAlgorithm)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCSigningAlgorithm alg = parcSigner_GetSigningAlgorithm(signer);
    assertTrue(PARCSigningAlgorithm_RSA == alg, "Expected PARCSigningAlgorithm_RSA algorithm, got %d", alg);

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetCryptoHashType)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCCryptoHashType type = parcSigner_GetCryptoHashType(signer);
    assertTrue(PARC_HASH_SHA256 == type, "Expected PARC_HASH_SHA256 algorithm, got %d", type);

    parcSigner_Release(&signer);
}

LONGBOW_TEST_CASE(Global, parcSigner_GetKeyStore)
{
    _MockSigner *mock = _createSigner();
    PARCSigner *signer = parcSigner_Create(mock, _MockSignerInterface);

    PARCKeyStore *keyStore = parcSigner_GetKeyStore(signer);
    assertNotNull(keyStore, "Expected non-NULL PARCKeyStore");

    parcSigner_Release(&signer);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Signer);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
