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
 * @file parc_PriorityQueue.h
 * @ingroup datastructures
 * @brief A priority queue (heap), where the top item is the minimum by the sort function.
 *
 * The user provides a sort function and the top item will be the minimum
 * as per the < relation.
 *
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2014, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#ifndef libparc_parc_PriorityQueue_h
#define libparc_parc_PriorityQueue_h

#include <stdlib.h>
#include <stdbool.h>

struct parc_priority_queue;
typedef struct parc_priority_queue PARCPriorityQueue;

typedef int (PARCPriorityQueueCompareTo)(const void *a, const void *b);
typedef void (PARCPriorityQueueDestroyer)(void **elementPtr);

/**
 * Calls {@link parcMemory_Deallocate} to free the element
 *
 * A simple destroyer that only uses {@link parcMemory_Deallocate}.
 *
 * @param [in,out] elementPtr Double pointer to data item, will be NULL'd
 *
 * Example:
 * @code
 *
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
void parcPriorityQueue_ParcFreeDestroyer(void **elementPtr);

/**
 * Treats the parameters as `uint64_t` pointers and compares them via natural sort order.
 *
 * Treats the parameters as `uint64_t` pointers and compares them via natural sort order.
 * Obeys standared CompareTo semantics.
 *
 * @param [in] a uint64_t pointer
 * @param [in] b uint64_t pointer
 *
 * @return -1 if a < b
 * @return  0 if a == b
 * @return +1 if a > b
 *
 * Example:
 * @code
 *
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
int parcPriorityQueue_Uint64CompareTo(const void *a, const void *b);


/**
 * Creates a priority queue with a given sort function.
 *
 * The sort function defines the ordering of the Priorty Queue.  The minimum element
 * will always be the head of the queue.
 *
 * The destroyer is called on data elements from {@link parcPriorityQueue_Clear()} and
 * {@link parcPriorityQueue_Destroy()}.  You may use {@linkparcPriorityQueue_ParcFreeDestroyer()} for
 * elements that can be freed by only calling {@link parcMemory_Deallocate}.
 *
 * @param [in] compare Defines the sort order of the priority queue
 * @param [in] destroyer Called for Clear and Destroy operations, may be NULL.
 *
 * @return non-null A pointer to a `PARCPriorityQueue`
 *
 * Example:
 * @code
 * PARCPriorityQueue *q = parcPriorityQueue_Create(parcPriorityQueue_Uint64CompareTo, parcPriorityQueue_ParcFreeDestroyer);
 * uint64_t *objectid = parcMemory_Allocate(sizeof(uint64_t));
 * objectid = 100;
 *
 * // this will use parcPriorityQueue_Uint64CompareTo sort order
 * parcPriorityQueue_Add(q, objectid);
 *
 * // this will use the ParcFreeDestroyer
 * parcPriorityQueue_Destroy(&q);
 * @endcode
 */
PARCPriorityQueue *parcPriorityQueue_Create(PARCPriorityQueueCompareTo *compare, PARCPriorityQueueDestroyer *destroyer);


/**
 * Destroy the queue and free remaining elements.
 *
 * Destroys the queue.  If the destroyer was set in Create, then it will be called
 * on all the remaining elements.
 *
 * @param [in,out] queuePtr Double pointer to allocated queue.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcPriorityQueue_Destroy(PARCPriorityQueue **queuePtr);

/**
 * Add an element to the priority queue, returning true if changed
 *
 * A "duplicate" is a data item that compares as equal to another item.  The priority
 * queue supports duplicates.  It is not stable in regard to the ordering of duplicates.
 * Because it supports duplicates, Add will always return true.
 *
 * The priority queue is unbounded.
 *
 * @param [in,out] queue The queue to modify
 * @param [in] data The data to add to the queue, which must be comparable and not NULL
 *
 * @return true The data structure was modified by adding the new value
 * @return false The data structure was not modified
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcPriorityQueue_Add(PARCPriorityQueue *queue, void *data);

/**
 * Removes all elements, calling the data structure's destroyer on each
 *
 * Remvoes all elements.  If the data structure's destroyer is non-NULL, it will be called
 * on each element.
 *
 * @param [in,out] queue The queue to modify
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcPriorityQueue_Clear(PARCPriorityQueue *queue);

/**
 * Returns the head element, but does not remove it.
 *
 * Returns the head element.  The data structure is not modified.  If the
 * priority queue is empty, will return NULL.
 *
 * @param [in] queue The `PARCPriorityQueue` to query.
 *
 * @return non-null The head element
 * @return null The queue is empty
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcPriorityQueue_Peek(PARCPriorityQueue *queue);

/**
 * Removes the head element from the queue and returns it.
 *
 * Removes the head element from the queue and returns it.  If the queue is empty,
 * it returns NULL.
 *
 * @param [in,out] queue The queue to query and modify.
 *
 * @return non-null The head element
 * @return null The queue is empty
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcPriorityQueue_Poll(PARCPriorityQueue *queue);

/**
 * Returns the number of elements in the queue.
 *
 * @param [in] queue The `PARCPriorityQueue` to query.
 *
 * @return number The number of elements in the queue.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcPriorityQueue_Size(const PARCPriorityQueue *queue);
#endif // libparc_parc_PriorityQueue_h
