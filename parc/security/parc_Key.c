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
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#include <config.h>

#include <string.h>
#include <stdio.h>

#include <parc/security/parc_Key.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

struct parc_key {
    PARCKeyId *keyid;
    PARCSigningAlgorithm signingAlg;
    PARCBuffer *key;
};

static void
_parcKey_FinalRelease(PARCKey **keyP)
{
    if ((*keyP)->keyid != NULL) {
        parcKeyId_Release(&(*keyP)->keyid);
    }
    if ((*keyP)->key != NULL) {
        parcBuffer_Release(&(*keyP)->key);
    }
}

parcObject_ExtendPARCObject(PARCKey, _parcKey_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCKey *
_parcKey_Create()
{
    PARCKey *key = parcObject_CreateInstance(PARCKey);
    return key;
}

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method support Public Key alogirhtms
 *
 * For Public Key algorithms, the buffer should be a DER encoded key.
 */
PARCKey *
parcKey_CreateFromDerEncodedPublicKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *derEncodedKey)
{
    assertNotNull(keyid, "Parameter keyid must be non-null");
    assertNotNull(derEncodedKey, "Parameter derEncodedKey must be non-null");

    // Exclude the symmetric key algorithms
    switch (signingAlg) {
        case PARCSigningAlgorithm_RSA: // fallthrough
        case PARCSigningAlgorithm_DSA:
            break;

        default:
            trapIllegalValueIf(true, "Unknown key algorithm or symmetric key algorithm: %s\n", parcSigningAlgorithm_ToString(signingAlg));
    }

    PARCKey *key = _parcKey_Create();
    assertNotNull(key, "Unable to allocate memory for PARCKey");

    key->key = parcBuffer_Acquire(derEncodedKey);
    key->signingAlg = signingAlg;
    key->keyid = parcKeyId_Acquire(keyid);
    return key;
}

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method support HMAC with symmetric keys.
 *
 * The secretkey is a set of random bytes.
 */
PARCKey *
parcKey_CreateFromSymmetricKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *secretkey)
{
    assertNotNull(keyid, "Parameter keyid must be non-null");
    assertNotNull(secretkey, "Parameter derEncodedKey must be non-null");

    // Exclude the symmetric key algorithms
    switch (signingAlg) {
        case PARCSigningAlgorithm_HMAC:
            break;

        default:
            trapIllegalValueIf(true, "Unknown key algorithm or symmetric key algorithm: %s\n", parcSigningAlgorithm_ToString(signingAlg));
    }

    PARCKey *key = _parcKey_Create();
    assertNotNull(key, "Unable to allocate memory for PARCKey");

    key->key = parcBuffer_Acquire(secretkey);
    key->signingAlg = signingAlg;
    key->keyid = parcKeyId_Acquire(keyid);
    return key;
}

/**
 * Destroys the key, keyid, and key byte buffer
 */

parcObject_ImplementAcquire(parcKey, PARCKey);

parcObject_ImplementRelease(parcKey, PARCKey);

void
parcKey_AssertValid(PARCKey *keyPtr)
{
    assertNotNull(keyPtr, "Parameter must be non-null double pointer");
    assertNotNull(keyPtr->key, "Parameter key must not be null");
    assertNotNull(keyPtr->keyid, "Parameter keyId must not be null");
}

PARCKeyId *
parcKey_GetKeyId(const PARCKey *key)
{
    assertNotNull(key, "Parameter must be non-null");
    return key->keyid;
}

PARCSigningAlgorithm
parcKey_GetSigningAlgorithm(const PARCKey *key)
{
    assertNotNull(key, "Parameter must be non-null");
    return key->signingAlg;
}

PARCBuffer *
parcKey_GetKey(const PARCKey *key)
{
    assertNotNull(key, "Parameter must be non-null");
    return key->key;
}

/**
 * keyA equals keyB iff the KeyIds are equal, the SigningAlgs are equal, and the keys are equal.
 * NULL equals NULL, but NULL does not equal any non-NULL
 */
bool
parcKey_Equals(const PARCKey *keyA, const PARCKey *keyB)
{
    if (keyA == keyB) {
        return true;
    }

    if (keyA == NULL || keyB == NULL) {
        return false;
    }

    if (keyA->signingAlg == keyB->signingAlg) {
        if (parcKeyId_Equals(keyA->keyid, keyB->keyid)) {
            if (parcBuffer_Equals(keyA->key, keyB->key)) {
                return true;
            }
        }
    }

    return false;
}

PARCKey *
parcKey_Copy(const PARCKey *original)
{
    PARCKey *newkey = _parcKey_Create();
    assertNotNull(newkey, "Unable to allocate memory for a new key");
    newkey->key = parcBuffer_Copy(original->key);
    newkey->keyid = parcKeyId_Copy(original->keyid);
    newkey->signingAlg = original->signingAlg;
    return newkey;
}

char *
parcKey_ToString(const PARCKey *key)
{
    char *string;
    int failure = asprintf(&string, "PARCKey {.KeyID=\"%s\", .SigningAlgorithm=\"%s\" }",
                           parcKeyId_ToString(key->keyid),
                           parcSigningAlgorithm_ToString(key->signingAlg));
    assertTrue(failure > -1, "Error asprintf");

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);

    return result;
}
