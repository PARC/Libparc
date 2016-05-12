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
 * @author Glenn Scott, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>

#include <parc/algol/parc_URI.h>

#include <parc/algol/parc_URIPath.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_BufferComposer.h>

char *sub_delims = "!$&'()*+,;=";
char *gen_delims = ":/?#[]@";

#define isSubDelims(c) (c != 0 && index(sub_delims, c) != NULL)
#define isGenDelims(c) (c != 0 && index(gen_delims, c) != NULL)
#define isDigit(c) (c >= '0' && c <= '9')
#define isAlpha(c) (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')

#define isUnreserved(c) (isAlpha(c) || isDigit(c) || c == '-' || c == '.' || c == '_' || c == '~')
#define isReserved(c) (isGenDelims(c) || isSubDelims(c))
#define isPchar(c) (isUnreserved(c) || isSubDelims(c) || c == ':' || c == '@')

struct parc_uri {
    char *scheme;
    char *authority;
    PARCURIPath *path;
    char *query;
    char *fragment;
};

static void
_parcURI_Finalize(PARCURI **uriPtr)
{
    assertNotNull(uriPtr, "Parameter must be a non-null pointer to a pointer to a PARCURI instance.");

    PARCURI *uri = *uriPtr;

    if (uri->scheme != NULL) {
        parcMemory_Deallocate((void **) &uri->scheme);
    }

    if (uri->authority != NULL) {
        parcMemory_Deallocate((void **) &(uri->authority));
    }

    if (uri->path) {
        parcURIPath_Release(&uri->path);
    }

    if (uri->query != NULL) {
        parcMemory_Deallocate((void **) &(uri->query));
    }

    if (uri->fragment != NULL) {
        parcMemory_Deallocate((void **) &(uri->fragment));
    }
}

parcObject_ExtendPARCObject(PARCURI, _parcURI_Finalize, parcURI_Copy, parcURI_ToString, parcURI_Equals, NULL, NULL, NULL);

PARCURI *
parcURI_Create(void)
{
    PARCURI *result = parcObject_CreateAndClearInstance(PARCURI);
    return result;
}


PARCURI *
parcURI_CreateFromValist(const char * restrict format, va_list argList)
{
    PARCURI *result = NULL;

    char *string;
    if (vasprintf(&string, format, argList) != -1) {
        result = parcURI_Parse(string);
        free(string);
    }
    return result;
}

PARCURI *
parcURI_CreateFromFormatString(const char * restrict format, ...)
{
    va_list argList;
    va_start(argList, format);

    PARCURI *result = parcURI_CreateFromValist(format, argList);

    va_end(argList);

    return result;
}

parcObject_ImplementAcquire(parcURI, PARCURI);

parcObject_ImplementRelease(parcURI, PARCURI);

static bool
_parcURI_SchemeEquals(const char *schemeA, const char *schemeB)
{
    if (schemeA == schemeB) {
        return true;
    }
    if (schemeA == NULL || schemeB == NULL) {
        return false;
    }
    return strcmp(schemeA, schemeB) == 0;
}

static bool
_parcURI_AuthorityEquals(const char *authorityA, const char *authorityB)
{
    if (authorityA == authorityB) {
        return true;
    }
    if (authorityA == NULL || authorityB == NULL) {
        return false;
    }
    return strcmp(authorityA, authorityB) == 0;
}

static bool
_parcURI_QueryEquals(const char *queryA, const char *queryB)
{
    if (queryA == queryB) {
        return true;
    }
    if (queryA == NULL || queryB == NULL) {
        return false;
    }
    return strcmp(queryA, queryB) == 0;
}

static bool
_parcURI_FragmentEquals(const char *fragmentA, const char *fragmentB)
{
    if (fragmentA == fragmentB) {
        return true;
    }
    if (fragmentA == NULL || fragmentB == NULL) {
        return false;
    }
    return strcmp(fragmentA, fragmentB) == 0;
}

bool
parcURI_Equals(const PARCURI *uriA, const PARCURI *uriB)
{
    if (uriA == uriB) {
        return true;
    }
    if (uriA == NULL || uriB == NULL) {
        return false;
    }

    if (_parcURI_SchemeEquals(uriA->scheme, uriB->scheme)) {
        if (_parcURI_AuthorityEquals(uriA->authority, uriB->authority)) {
            if (parcURIPath_Equals(uriA->path, uriB->path)) {
                if (_parcURI_QueryEquals(uriA->query, uriB->query)) {
                    if (_parcURI_FragmentEquals(uriA->fragment, uriB->fragment)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/*
 * Parse and return a copy of the scheme portion of the URI.
 *
 * If this function returns successfully,
 * the input parameter @p pointer will point to either a null-byte or the first character
 * after the ':' separating the scheme from the rest of the URI.
 *
 * @return non-NULL An allocated string copy of the string which must be freed by the caller via <code>parcMemory_Deallocate</code>.
 * @return NULL The scheme is malformed.
 */
static char *
_parseScheme(const char *uri, const char **pointer)
{
    size_t length = 0;

    const char *p = uri;
    while (*p != 0 && *p != ':') {
        length++;
        p++;
    }
    if (*p == 0) {
        return NULL;
    }

    if (length == 0) {
        return NULL;
    }

    char *result = parcMemory_StringDuplicate(uri, length);

    *pointer = (char *) &uri[length + 1];
    return result;
}

/**
 * @function _parseAuthority
 * @abstract Parse the authority portion of a URI, if present.
 * @discussion
 *   A URI may have an optional authority component.
 *   If the given string begins with a double forward slash ("//"),
 *   then it is followed by an authority part and a path.
 *   If the string doesn't begin with ("//") it contains only a path and this
 *    function simply returns NULL and setting the give pointer to the first
 *   character of the (expected) path.
 *
 * @param string A pointer to the start of the (potential) authority component.
 * @param pointer A pointer to a character pointer that will be assigned point to the first character that begins the path.
 * @return An allocated string, to be freed via parcMemory_Deallocate, if the authority portion is present or NULL if otherwise.
 */
static char *
_parseAuthority(const char *string, const char **pointer)
{
    if (string[0] == '/' && string[1] == '/') {
        size_t length = 0;
        for (const char *p = &string[2]; *p != '/'; p++) {
            if (*p == 0) {
                *pointer = p;
                break;
            }
            length++;
        }

        char *result = parcMemory_StringDuplicate(&string[2], length);
        // The pointer must point to the first character *after* the '/' character as the '/' is not part of the path.
        *pointer = &(&string[2])[length];
        return result;
    }
    *pointer = string;
    return NULL;
}

static char *
_parseQuery(const char *string, const char **pointer)
{
    if (*string != '?') {
        return NULL;
    }

    string++;
    size_t length = 0;
    for (const char *p = string; *p != 0 && *p != '#'; p++) {
        length++;
    }

    char *result = parcMemory_StringDuplicate(string, length);
    *pointer = &string[length];
    return result;
}

static char *
_parseFragment(const char *string, const char **pointer)
{
    if (*string != '#') {
        return NULL;
    }
    string++;
    size_t length = 0;
    for (const char *p = string; *p != 0; p++) {
        length++;
    }
    char *result = parcMemory_StringDuplicate(string, length);

    *pointer = &string[length];
    return result;
}

static void
_parcURI_SetScheme(PARCURI *uri, const char *scheme)
{
    if (uri->scheme != NULL) {
        parcMemory_Deallocate((void **) &(uri->scheme));
    }
    if (scheme == NULL) {
        uri->scheme = NULL;
    } else {
        uri->scheme = parcMemory_StringDuplicate(scheme, strlen(scheme));
    }
}

static void
_parcURI_SetAuthority(PARCURI *uri, const char *authority)
{
    if (uri->authority != NULL) {
        parcMemory_Deallocate((void **) &(uri->authority));
    }
    if (authority == NULL) {
        uri->authority = NULL;
    } else {
        uri->authority = parcMemory_StringDuplicate(authority, strlen(authority));
    }
}

static void
_parcURI_SetQuery(PARCURI *uri, const char *query)
{
    if (uri->query != NULL) {
        parcMemory_Deallocate((void **) &(uri->query));
    }
    if (query == NULL) {
        uri->query = NULL;
    } else {
        uri->query = parcMemory_StringDuplicate(query, strlen(query));
    }
}

static void
_parcURI_SetFragment(PARCURI *uri, const char *fragment)
{
    if (uri->fragment != NULL) {
        parcMemory_Deallocate((void **) &(uri->fragment));
    }
    if (fragment == NULL) {
        uri->fragment = NULL;
    } else {
        uri->fragment = parcMemory_StringDuplicate(fragment, strlen(fragment));
    }
}

PARCURI *
parcURI_Parse(const char *string)
{
    const char *pointer = string;

    PARCURI *result = parcURI_Create();

    if (result != NULL) {
        result->scheme = _parseScheme(pointer, &pointer);
        if (result->scheme != NULL) {
            result->authority = _parseAuthority(pointer, &pointer);
            result->path = parcURIPath_Parse(pointer, &pointer);
            result->query = _parseQuery(pointer, &pointer);
            result->fragment = _parseFragment(pointer, &pointer);
        } else {
            parcURI_Release(&result);
            result = NULL;
        }
    }

    return result;
}

PARCURI *
parcURI_Copy(const PARCURI *uri)
{
    PARCURI *result = parcURI_Create();

    if (result != NULL) {
        _parcURI_SetScheme(result, parcURI_GetScheme(uri));
        _parcURI_SetAuthority(result, parcURI_GetAuthority(uri));
        result->path = parcURIPath_Copy(parcURI_GetPath(uri));
        _parcURI_SetQuery(result, parcURI_GetQuery(uri));
        _parcURI_SetFragment(result, parcURI_GetFragment(uri));
    }

    return result;
}

PARCBufferComposer *
parcURI_BuildString(const PARCURI *uri, PARCBufferComposer *composer)
{
    parcBufferComposer_PutStrings(composer, parcURI_GetScheme(uri), ":", NULL);

    if (parcURI_GetAuthority(uri)) {
        parcBufferComposer_PutString(composer, "//");
        parcBufferComposer_PutString(composer, parcURI_GetAuthority(uri));
    }

    parcBufferComposer_PutString(composer, "/");
    parcURIPath_BuildString(parcURI_GetPath(uri), composer);

    if (parcURI_GetQuery(uri)) {
        parcBufferComposer_PutStrings(composer, "?", parcURI_GetQuery(uri), NULL);
    }

    if (parcURI_GetFragment(uri)) {
        parcBufferComposer_PutStrings(composer, "#", parcURI_GetFragment(uri), NULL);
    }

    return composer;
}

char *
parcURI_ToString(const PARCURI *uri)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        if (parcURI_BuildString(uri, composer) != NULL) {
            PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
            result = parcBuffer_ToString(tempBuffer);
            parcBuffer_Release(&tempBuffer);
        }
        parcBufferComposer_Release(&composer);
    }

    return result;
}

const char *
parcURI_GetScheme(const PARCURI *uri)
{
    return uri->scheme;
}

const char *
parcURI_GetAuthority(const PARCURI *uri)
{
    return uri->authority;
}

PARCURIPath *
parcURI_GetPath(const PARCURI *uri)
{
    return uri->path;
}

const char *
parcURI_GetQuery(const PARCURI *uri)
{
    return uri->query;
}

const char *
parcURI_GetFragment(const PARCURI *uri)
{
    return uri->fragment;
}
