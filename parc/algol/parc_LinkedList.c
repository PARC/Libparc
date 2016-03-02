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
/**
 * @author Glenn Scott, Palo Alto Research Center (Xerox PARC)
 * @copyright 2013-2015, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <sys/queue.h>

#include "parc_LinkedList.h"

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

PARCListInterface *PARCLinkedListAsPARCList = &(PARCListInterface) {
    .Add                    = (bool      (*)(void *, void *))                       parcLinkedList_Append,
    .AddAtIndex             = (void      (*)(void *, int index, void *))            NULL,
    .AddCollection          = (bool      (*)(void *, PARCCollection *))             NULL,
    .AddCollectionAtIndex   = (bool      (*)(void *, int index, PARCCollection *))  NULL,
    .Clear                  = (void      (*)(void *))                               NULL,
    .Contains               = (bool      (*)(const void *, void *))                 NULL,
    .ContainsCollection     = (bool      (*)(void *, PARCCollection *))             NULL,
    .Copy                   = (void *    (*)(const PARCList *))                     parcLinkedList_Copy,
    .Destroy                = (void      (*)(void **))                              parcLinkedList_Release,
    .Equals                 = (bool      (*)(const void *, const void *))           parcLinkedList_Equals,
    .GetAtIndex             = (void *    (*)(const void *, size_t))                 parcLinkedList_GetAtIndex,
    .HashCode               = (int       (*)(void *))                               NULL,
    .IndexOf                = (size_t    (*)(const void *, void *element))          NULL,
    .IsEmpty                = (bool      (*)(const void *))                         parcLinkedList_IsEmpty,
    .LastIndexOf            = (size_t    (*)(void *, void *element))                NULL,
    .Remove                 = (bool      (*)(void *, void *element))                parcLinkedList_Remove,
    .RemoveAtIndex          = (void *    (*)(PARCList *, size_t))                   NULL,
    .RemoveCollection       = (bool      (*)(void *, PARCCollection *))             NULL,
    .RetainCollection       = (bool      (*)(void *, PARCCollection *))             NULL,
    .SetAtIndex             = (void *    (*)(void *, size_t index, void *))         NULL,
    .Size                   = (size_t    (*)(const void *))                         parcLinkedList_Size,
    .SubList                = (PARCList *(*)(void *, size_t, size_t))               NULL,
    .ToArray                = (void**    (*)(void *))                               NULL,
};

typedef struct parc_linkedlist_node {
    PARCObject *object;
    struct parc_linkedlist_node *previous;
    struct parc_linkedlist_node *next;
} _PARCLinkedListNode;

struct parc_linkedlist {
    _PARCLinkedListNode *head;
    _PARCLinkedListNode *tail;
    size_t size;
};

static bool
_parcLinkedListNode_IsValid(const _PARCLinkedListNode *node)
{
    bool result = false;

    if (node != NULL) {
        if (node->object != NULL) {
            if (parcObject_IsValid(node->object)) {
                if (node->previous) {
                    if (node->previous->next == node) {
                        if (parcObject_IsValid(node->previous->object)) {
                            result = true;
                        }
                    }
                } else {
                    result = true;
                }
                if (node->next != NULL) {
                    if (node->next->previous == node) {
                        if (parcObject_IsValid(node->next->object)) {
                            result = true;
                        }
                    }
                } else {
                    result = true;
                }
            }
        }
    }

    return result;
}

static inline _PARCLinkedListNode *
_parcLinkedListNode_Create(const PARCObject *object, _PARCLinkedListNode *previous, _PARCLinkedListNode *next)
{
    parcObject_OptionalAssertValid(object);

    _PARCLinkedListNode *result = parcMemory_Allocate(sizeof(_PARCLinkedListNode));
    if (result != NULL) {
        result->object = parcObject_Acquire(object);
        result->next = next;
        result->previous = previous;
    }

    return result;
}

static void
_parcLinkedListIterator_IsValid(const _PARCLinkedListNode *node)
{
    if (node != NULL) {
        assertTrue(_parcLinkedListNode_IsValid(node), "node is invalid");
    }
}

static void
_parcLinkedListNode_Destroy(PARCLinkedList *list __attribute__((unused)), _PARCLinkedListNode **nodePtr, bool releaseObj)
{
    _PARCLinkedListNode *node = *nodePtr;

    if (releaseObj) {
        parcObject_Release(&node->object);
    }
    parcMemory_Deallocate((void **) nodePtr);
}

static void
_parcLinkedList_Destroy(PARCLinkedList **listPtr)
{
    PARCLinkedList *list = *listPtr;

    _PARCLinkedListNode *next = NULL;

    for (_PARCLinkedListNode *node = list->head; node != NULL; node = next) {
        next = node->next;
        _parcLinkedListNode_Destroy(list, &node, true);
    }
}

static _PARCLinkedListNode *
_parcLinkedIterator_Init(PARCLinkedList *list __attribute__((unused)))
{
    return NULL;
}

static bool
_parcLinkedListNode_Fini(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node __attribute__((unused)))
{
    return true;
}

static struct parc_linkedlist_node *
_parcLinkedListNode_Next(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node)
{
    struct parc_linkedlist_node *result = NULL;

    if (node == NULL) {
        result = list->head;
    } else {
        assertTrue(_parcLinkedListNode_IsValid(node), "node is invalid");
        trapOutOfBoundsIf(node->next == NULL, "No more elements.");
        result = node->next;
    }

    assertTrue(_parcLinkedListNode_IsValid(result), "result is invalid");
    parcObject_OptionalAssertValid(result->object);

    return result;
}

static void
_parcLinkedListNode_RemoveNoChecks(PARCLinkedList *list __attribute__((unused)), _PARCLinkedListNode *node, bool releaseObj) 
{
    list->size--;

    if (node == list->head) {
        list->head = node->next;
    }
    if (node == list->tail) {
        list->tail = node->previous;
    }
    if (node->previous) {
        node->previous->next = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    }

   _parcLinkedListNode_Destroy(list, &node, releaseObj);
}

static void
_parcLinkedListNode_Remove(PARCLinkedList *list __attribute__((unused)), _PARCLinkedListNode **nodePtr)
{
    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");

    _PARCLinkedListNode *node = *nodePtr;

    if (node != NULL) {

        *nodePtr = node->previous;

        _parcLinkedListNode_RemoveNoChecks(list, node, true);

        assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");
    }
}

static bool
_parcLinkedListNode_HasNext(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node)
{
    bool result = false;

    if (node == NULL) {
        result = (list->head != NULL);
        if (result) {
            assertTrue(_parcLinkedListNode_IsValid(list->head), "node is invalid");
        }
    } else {
        result = node->next != NULL;
        if (result) {
            assertTrue(_parcLinkedListNode_IsValid(node->next), "node is invalid");
        }
    }


    return result;
}

static void *
_parcLinkedListNode_Element(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node)
{
    return node->object;
}

parcObject_ExtendPARCObject(PARCLinkedList, _parcLinkedList_Destroy, parcLinkedList_Copy, NULL, parcLinkedList_Equals, NULL, NULL, NULL);

PARCIterator *
parcLinkedList_CreateIterator(PARCLinkedList *list)
{
    PARCIterator *iterator = parcIterator_Create(list,
                                                 (void *(*)(PARCObject *)) _parcLinkedIterator_Init,
                                                 (bool  (*)(PARCObject *, void *)) _parcLinkedListNode_HasNext,
                                                 (void *(*)(PARCObject *, void *)) _parcLinkedListNode_Next,
                                                 (void  (*)(PARCObject *, void **)) _parcLinkedListNode_Remove,
                                                 (void *(*)(PARCObject *, void *)) _parcLinkedListNode_Element,
                                                 (void  (*)(PARCObject *, void *)) _parcLinkedListNode_Fini,
                                                 (void  (*)(const void *))         _parcLinkedListIterator_IsValid);

    return iterator;
}

PARCLinkedList *
parcLinkedList_Create(void)
{
    PARCLinkedList *result = parcObject_CreateInstance(PARCLinkedList);

    if (result != NULL) {
        result->head = NULL;
        result->tail = NULL;
        result->size = 0;
    }
    return result;
}

bool
parcLinkedList_IsValid(const PARCLinkedList *list)
{
    bool result = false;

    if (list != NULL) {
        if (parcObject_IsValid(list)) {
            if (list->size > 0) {
                if (list->head != NULL) {
                    if (list->tail != NULL) {
                        result = true;
                        for (_PARCLinkedListNode *node = list->head; node != NULL; node = node->next) {
                            if (_parcLinkedListNode_IsValid(node) == false) {
                                result = false;
                                break;
                            }
                        }
                    }
                }
            } else {
                if (list->head == NULL) {
                    if (list->tail == NULL) {
                        result = true;
                    }
                }

            }
        }
    }

    return result;
}

void
parcLinkedList_AssertValid(const PARCLinkedList *instance)
{
    assertTrue(parcLinkedList_IsValid(instance),
               "PARCLinkedList is not valid.");
}

parcObject_ImplementAcquire(parcLinkedList, PARCLinkedList);

parcObject_ImplementRelease(parcLinkedList, PARCLinkedList);

PARCLinkedList *
parcLinkedList_Copy(const PARCLinkedList *list)
{
    PARCLinkedList *result = parcLinkedList_Create();

    struct parc_linkedlist_node *node = list->head;

    while (node != NULL) {
        parcLinkedList_Append(result, node->object);
        node = node->next;
    }

    return result;
}

bool
parcLinkedList_Contains(const PARCLinkedList *list, const PARCObject *element)
{
    bool result = false;

    struct parc_linkedlist_node *node = list->head;

    while (node != NULL) {
        if (parcObject_Equals(node->object, element)) {
            result = true;
            break;
        }
        node = node->next;
    }

    return result;
}

PARCLinkedList *
parcLinkedList_Append(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *node = _parcLinkedListNode_Create(element, list->tail, NULL);

    if (list->tail == NULL) {
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    if (list->head == NULL) {
        list->head = list->tail;
    }

    list->size++;

    return list;
}

PARCLinkedList *
parcLinkedList_AppendAll(PARCLinkedList *list, const PARCLinkedList *other)
{
    size_t length = parcLinkedList_Size(other);
    for (size_t index = 0; index < length; index++) {
        parcLinkedList_Append(list, parcLinkedList_GetAtIndex(other, index));
    }
    return list;
}

PARCLinkedList *
parcLinkedList_Prepend(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *node = _parcLinkedListNode_Create(element, NULL, list->head);

    if (list->head == NULL) {
        list->head = node;
    } else {
        list->head->previous = node;
        list->head = node;
    }

    if (list->tail == NULL) {
        list->tail = list->head;
    }
    list->size++;

    assertTrue(_parcLinkedListNode_IsValid(node), "_PARCLinkedListNode is invalid");
    parcLinkedList_OptionalAssertValid(list);

    return list;
}

bool
parcLinkedList_Remove(PARCLinkedList *list, const PARCObject *element) 
{
    assertTrue(element != NULL, "Element should not be NULL");
    bool result = false;

    _PARCLinkedListNode *node = list->head;
    if (node != NULL) {
        while (node != NULL) {
            if (parcObject_Equals(node->object, element)) {
                _parcLinkedListNode_RemoveNoChecks(list, node, true);
                result = true;
                break;
            }
            node = node->next;
        }
    }

    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");
    parcLinkedList_OptionalAssertValid(list);

    return result;
}

PARCObject *
parcLinkedList_RemoveFirst(PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->head != NULL) {
        _PARCLinkedListNode *node = list->head;

        result = node->object;
        _parcLinkedListNode_RemoveNoChecks(list, node, false);
    }

    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");

    parcLinkedList_OptionalAssertValid(list);

    return result;
}

PARCObject *
parcLinkedList_RemoveLast(PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->tail != NULL) {
        _PARCLinkedListNode *node = list->tail;

        result = node->object;
        _parcLinkedListNode_RemoveNoChecks(list, node, false);
    }

    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");
    parcLinkedList_OptionalAssertValid(list);
    return result;
}

PARCObject *
parcLinkedList_PeekFirst(const PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->head != NULL) {
        _PARCLinkedListNode *node = list->head;
        result = node->object;
    }
    return result;
}

PARCObject *
parcLinkedList_PeekLast(const PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->tail != NULL) {
        _PARCLinkedListNode *node = list->tail;
        result = node->object;
    }
    return result;
}

PARCHashCode
parcLinkedList_HashCode(const PARCLinkedList *list)
{
    PARCHashCode result = 0;

    _PARCLinkedListNode *node = list->head;
    if (node != NULL) {
        while (node != NULL) {
            result += parcObject_HashCode(node->object);
            node = node->next;
        }
    }

    return result;
}

size_t
parcLinkedList_Size(const PARCLinkedList *list)
{
    return list->size;
}

bool
parcLinkedList_IsEmpty(const PARCLinkedList *list)
{
    return (parcLinkedList_Size(list) == 0);
}

static void
_parcLinkedList_InsertInitialNode(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, NULL, NULL);
    list->head = newNode;
    list->tail = newNode;
}

PARCLinkedList *
parcLinkedList_InsertAtIndex(PARCLinkedList *list, size_t index, const PARCObject *element)
{
    if (index == 0) {
        if (list->head == NULL) {
            _parcLinkedList_InsertInitialNode(list, element);
        } else {
            _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, NULL, list->head);

            list->head->previous = newNode;
            list->tail = list->head;
            list->head = newNode;
        }

        list->size++;
    } else if (index == list->size) {
        _PARCLinkedListNode *node = list->tail;
        node->next = _parcLinkedListNode_Create(element, node, NULL);
        list->tail = node->next;
        list->size++;
    } else {
        _PARCLinkedListNode *node = list->head;
        while (index-- && node->next != NULL) {
            node = node->next;
        }
         _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, node->previous, node);

        node->previous->next = newNode;
        node->previous = newNode;
        list->size++;
    }

    assertTrue(parcLinkedList_IsValid(list), "PARCLinkedList is invalid.");
    return list;
}

PARCObject *
parcLinkedList_GetAtIndex(const PARCLinkedList *list, size_t index)
{
    if (index > (parcLinkedList_Size(list) - 1)) {
        trapOutOfBounds(index, "[0, %zd]", parcLinkedList_Size(list) - 1);
    }
    _PARCLinkedListNode *node = list->head;
    while (index--) {
        node = node->next;
    }

    return node->object;
}

bool
parcLinkedList_Equals(const PARCLinkedList *x, const PARCLinkedList *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->size == y->size) {
        _PARCLinkedListNode *xNode = x->head;
        _PARCLinkedListNode *yNode = y->head;

        while (xNode != NULL) {
            if (parcObject_Equals(xNode->object, yNode->object) == false) {
                return false;
            }
            xNode = xNode->next;
            yNode = yNode->next;
        }
        return true;
    }
    return false;
}

void
parcLinkedList_Display(const PARCLinkedList *list, const int indentation)
{
    if (list == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCLinkedList@NULL");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCLinkedList@%p { .size=%zd, .head=%p, .tail=%p", (void *) list, list->size, list->head, list->tail);

        _PARCLinkedListNode *node = list->head;

        while (node != NULL) {

            parcDisplayIndented_PrintLine(indentation + 1,
                                          "%4s %11p { .previous=%11p, %11p=%11p, .next=%11p } %4s",
                                          (list->head == node) ? "head" : "    ",
                                          node, node->previous, node, node->object, node->next,
                                          (list->tail == node) ? "tail" : "    ");
            node = node->next;
        }

        parcDisplayIndented_PrintLine(indentation, "}\n");
    }
}

bool
parcLinkedList_SetEquals(const PARCLinkedList *x, const PARCLinkedList *y)
{
    bool result = false;

    if (x->size == y->size) {
        for (size_t i = 0; i < x->size; i++) {
            PARCObject *xObject = parcLinkedList_GetAtIndex(x, i);
            for (size_t j = 0; j < x->size; j++) {
                PARCObject *yObject = parcLinkedList_GetAtIndex(y, j);
                if (parcObject_Equals(xObject, yObject) == false) {
                    break;
                }
            }
        }
        result = true;
    }

    return result;
}
