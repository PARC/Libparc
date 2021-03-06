/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
 * @file parc_SymmetricKeySigner.h
 * @brief A symmetric-key signer that creates MAC tags.
 *
 * @author Christopher A. Wood, Computing Science Laboratory, PARC
 * @copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#ifndef Libparc_parc_SymmetricKeySigner
#define Libparc_parc_SymmetricKeySigner
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

#include <parc/security/parc_SymmetricKeyStore.h>

struct PARCSymmetricKeySigner;
typedef struct PARCSymmetricKeySigner PARCSymmetricKeySigner;

extern PARCSigningInterface *PARCSymmetricKeySignerAsSigner;

/**
 * Increase the number of references to a `PARCSymmetricKeySigner` instance.
 *
 * Note that new `PARCSymmetricKeySigner` is not created,
 * only that the given `PARCSymmetricKeySigner` reference count is incremented.
 * Discard the reference by invoking `parcSymmetricKeySigner_Release`.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     PARCSymmetricKeySigner *b = parcSymmetricKeySigner_Acquire();

 *     parcSymmetricKeySigner_Release(&a);
 *     parcSymmetricKeySigner_Release(&b);
 * }
 * @endcode
 */
PARCSymmetricKeySigner *parcSymmetricKeySigner_Acquire(const PARCSymmetricKeySigner *instance);

#ifdef Libparc_DISABLE_VALIDATION
#  define parcSymmetricKeySigner_OptionalAssertValid(_instance_)
#else
#  define parcSymmetricKeySigner_OptionalAssertValid(_instance_) parcSymmetricKeySigner_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCSymmetricKeySigner` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     parcSymmetricKeySigner_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcSymmetricKeySigner_Release(&b);
 * }
 * @endcode
 */
void parcSymmetricKeySigner_AssertValid(const PARCSymmetricKeySigner *instance);

/**
 * Create an instance of PARCSymmetricKeySigner
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCSymmetricKeySigner instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCSymmetricKeySigner *parcSymmetricKeySigner_Create(PARCSymmetricKeyStore *keyStore, PARCCryptoHashType hmacHashType);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 * @param [in] other A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *     PARCSymmetricKeySigner *b = parcSymmetricKeySigner_Create();
 *
 *     if (parcSymmetricKeySigner_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcSymmetricKeySigner_Release(&a);
 *     parcSymmetricKeySigner_Release(&b);
 * }
 * @endcode
 *
 * @see parcSymmetricKeySigner_Equals
 */
int parcSymmetricKeySigner_Compare(const PARCSymmetricKeySigner *instance, const PARCSymmetricKeySigner *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCSymmetricKeySigner` instance.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     PARCSymmetricKeySigner *copy = parcSymmetricKeySigner_Copy(&b);
 *
 *     parcSymmetricKeySigner_Release(&b);
 *     parcSymmetricKeySigner_Release(&copy);
 * }
 * @endcode
 */
PARCSymmetricKeySigner *parcSymmetricKeySigner_Copy(const PARCSymmetricKeySigner *original);

/**
 * Print a human readable representation of the given `PARCSymmetricKeySigner`.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     parcSymmetricKeySigner_Display(a, 0);
 *
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 */
void parcSymmetricKeySigner_Display(const PARCSymmetricKeySigner *instance, int indentation);

/**
 * Determine if two `PARCSymmetricKeySigner` instances are equal.
 *
 * The following equivalence relations on non-null `PARCSymmetricKeySigner` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcSymmetricKeySigner_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcSymmetricKeySigner_Equals(x, y)` must return true if and only if
 *        `parcSymmetricKeySigner_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcSymmetricKeySigner_Equals(x, y)` returns true and
 *        `parcSymmetricKeySigner_Equals(y, z)` returns true,
 *        then `parcSymmetricKeySigner_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcSymmetricKeySigner_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcSymmetricKeySigner_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCSymmetricKeySigner instance.
 * @param [in] y A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *     PARCSymmetricKeySigner *b = parcSymmetricKeySigner_Create();
 *
 *     if (parcSymmetricKeySigner_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcSymmetricKeySigner_Release(&a);
 *     parcSymmetricKeySigner_Release(&b);
 * }
 * @endcode
 * @see parcSymmetricKeySigner_HashCode
 */
bool parcSymmetricKeySigner_Equals(const PARCSymmetricKeySigner *x, const PARCSymmetricKeySigner *y);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcSymmetricKeySigner_Equals} method,
 * then calling the {@link parcSymmetricKeySigner_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcSymmetricKeySigner_Equals} function,
 * then calling the `parcSymmetricKeySigner_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     PARCHashCode hashValue = parcSymmetricKeySigner_HashCode(buffer);
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcSymmetricKeySigner_HashCode(const PARCSymmetricKeySigner *instance);

/**
 * Determine if an instance of `PARCSymmetricKeySigner` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     if (parcSymmetricKeySigner_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 *
 */
bool parcSymmetricKeySigner_IsValid(const PARCSymmetricKeySigner *instance);

/**
 * Release a previously acquired reference to the given `PARCSymmetricKeySigner` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 */
void parcSymmetricKeySigner_Release(PARCSymmetricKeySigner **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     PARCJSON *json = parcSymmetricKeySigner_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcSymmetricKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcSymmetricKeySigner_ToJSON(const PARCSymmetricKeySigner *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCSymmetricKeySigner`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCSymmetricKeySigner instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCSymmetricKeySigner *a = parcSymmetricKeySigner_Create();
 *
 *     char *string = parcSymmetricKeySigner_ToString(a);
 *
 *     parcSymmetricKeySigner_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcSymmetricKeySigner_Display
 */
char *parcSymmetricKeySigner_ToString(const PARCSymmetricKeySigner *instance);
#endif // Libparc_parc_SymmetricKeySigner
