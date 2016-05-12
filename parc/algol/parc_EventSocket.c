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
/**
 * @author Kevin Fox, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_EventScheduler.h>
#include <parc/algol/parc_EventSocket.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

static int _parc_event_socket_debug_enabled = 0;

#define parcEventSocket_LogDebug(parcEventSocket, ...) \
    if (_parc_event_socket_debug_enabled) \
        parcLog_Debug(parcEventScheduler_GetLogger(parcEventSocket->eventScheduler), __VA_ARGS__)

/**
 * Current implementation based on top of libevent2
 */

#include <sys/errno.h>
#include <event2/listener.h>

/**
 * @typedef PARCEventSocket
 * @brief A structure containing private event state
 */
struct PARCEventSocket {
    struct evconnlistener *listener;

    // Event scheduler we have been queued with
    PARCEventScheduler *eventScheduler;

    // Interpose on EventSocket callbacks
    PARCEventSocket_Callback *socketCallback;
    void *socketUserData;
    PARCEventSocket_ErrorCallback *socketErrorCallback;
    void *socketErrorUserData;
};

static void
_parc_evconn_callback(struct evconnlistener *listener, evutil_socket_t fd,
                      struct sockaddr *address, int socklen, void *ctx)
{
    PARCEventSocket *parcEventSocket = (PARCEventSocket *) ctx;
    parcEventSocket_LogDebug(parcEventSocket, "_parc_evconn_callback(fd=%d,,parcEventSocket=%p)\n", fd, parcEventSocket);

    parcEventSocket->socketCallback((int) fd, address, socklen, parcEventSocket->socketUserData);
}

static void
_parc_evconn_error_callback(struct evconnlistener *listener, void *ctx)
{
    PARCEventSocket *parcEventSocket = (PARCEventSocket *) ctx;

    int error = EVUTIL_SOCKET_ERROR();
    char *errorString = evutil_socket_error_to_string(error);
    parcEventSocket_LogDebug(parcEventSocket,
                             "_parc_evconn_error_callback(error=%d,errorString=%s,parcEventSocket=%p)\n",
                             error, errorString, parcEventSocket);

    parcEventSocket->socketErrorCallback(parcEventSocket->eventScheduler,
                                         error, errorString, parcEventSocket->socketErrorUserData);
}

PARCEventSocket *
parcEventSocket_Create(PARCEventScheduler *eventScheduler,
                       PARCEventSocket_Callback *callback,
                       PARCEventSocket_ErrorCallback *errorCallback,
                       void *userData, const struct sockaddr *sa, int socklen)
{
    PARCEventSocket *parcEventSocket = parcMemory_AllocateAndClear(sizeof(PARCEventSocket));
    assertNotNull(parcEventSocket, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCEventSocket));

    parcEventSocket->eventScheduler = eventScheduler;
    parcEventSocket->socketCallback = callback;
    parcEventSocket->socketErrorCallback = errorCallback;
    parcEventSocket->socketUserData = userData;
    parcEventSocket->socketErrorUserData = userData;
    parcEventSocket->listener = evconnlistener_new_bind(parcEventScheduler_GetEvBase(eventScheduler),
                                                        _parc_evconn_callback, parcEventSocket,
                                                        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                                        sa, socklen);
    if (parcEventSocket->listener == NULL) {
        parcLog_Error(parcEventScheduler_GetLogger(eventScheduler),
                      "Libevent evconnlistener_new_bind error (%d): %s",
                      errno, strerror(errno));
        parcEventSocket_Destroy(&parcEventSocket);
        return NULL;
    }

    if (errorCallback) {
        evconnlistener_set_error_cb(parcEventSocket->listener, _parc_evconn_error_callback);
    }
    parcEventSocket_LogDebug(parcEventSocket,
                             "parcEventSocket_Create(cb=%p,args=%p) = %p\n",
                             callback, userData, parcEventSocket);
    return parcEventSocket;
}

void
parcEventSocket_Destroy(PARCEventSocket **socketEvent)
{
    assertNotNull(*socketEvent, "parcEventSocket_Destroy must be passed a valid socketEvent!");

    if ((*socketEvent)->listener) {
        evconnlistener_free((*socketEvent)->listener);
    }
    parcEventSocket_LogDebug((*socketEvent), "parcEventSocket_Destroy(%p)\n", *socketEvent);
    parcMemory_Deallocate((void **) socketEvent);
}

void
parcEventSocket_EnableDebug(void)
{
    _parc_event_socket_debug_enabled = 1;
}

void
parcEventSocket_DisableDebug(void)
{
    _parc_event_socket_debug_enabled = 0;
}
