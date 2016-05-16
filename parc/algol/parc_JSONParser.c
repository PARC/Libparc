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

#include <ctype.h>
#include <string.h>

#include <parc/algol/parc_JSONParser.h>

#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Object.h>

struct parc_buffer_parser {
    char *ignore;
    PARCBuffer *buffer;
};

static PARCBuffer *
_getBuffer(const PARCJSONParser *parser)
{
    return parser->buffer;
}

static void
_destroyPARCBufferParser(PARCJSONParser **instancePtr)
{
    PARCJSONParser *parser = *instancePtr;
    parcBuffer_Release(&parser->buffer);
}

parcObject_ExtendPARCObject(PARCJSONParser, _destroyPARCBufferParser, NULL, NULL, NULL, NULL, NULL, NULL);

PARCJSONParser *
parcJSONParser_Create(PARCBuffer *buffer)
{
    PARCJSONParser *result = parcObject_CreateInstance(PARCJSONParser);
    result->ignore = " \t\n";
    result->buffer = parcBuffer_Acquire(buffer);
    return result;
}

void
parcJSONParser_AssertValid(const PARCJSONParser *parser)
{
    assertNotNull(parser, "PARCJSONParser cannot be NULL");
    parcBuffer_OptionalAssertValid(parser->buffer);
}

parcObject_ImplementAcquire(parcJSONParser, PARCJSONParser);

parcObject_ImplementRelease(parcJSONParser, PARCJSONParser);

void
parcJSONParser_SkipIgnored(PARCJSONParser *parser)
{
    parcJSONParser_OptionalAssertValid(parser);

    parcBuffer_SkipOver(parser->buffer, strlen(parser->ignore), (uint8_t *) parser->ignore);
}

char
parcJSONParser_NextChar(PARCJSONParser *parser)
{
    parcJSONParser_SkipIgnored(parser);
    return (char) parcBuffer_GetUint8(parser->buffer);
}

bool
parcJSONParser_Next(PARCJSONParser *parser, char *value)
{
    bool result = false;
    parcJSONParser_SkipIgnored(parser);
    if (parcJSONParser_Remaining(parser) > 0) {
        *value = (char) parcBuffer_GetUint8(parser->buffer);
        result = true;
    }
    return result;
}

char
parcJSONParser_PeekNextChar(PARCJSONParser *parser)
{
    parcJSONParser_SkipIgnored(parser);
    return (char) parcBuffer_PeekByte(parser->buffer);
}

void
parcJSONParser_Advance(PARCJSONParser *parser, long bytes)
{
    parcJSONParser_OptionalAssertValid(parser);

    parcBuffer_SetPosition(parser->buffer, parcBuffer_Position(parser->buffer) + bytes);
}

size_t
parcJSONParser_Remaining(const PARCJSONParser *parser)
{
    parcJSONParser_OptionalAssertValid(parser);

    return parcBuffer_Remaining(parser->buffer);
}

bool
parcJSONParser_RequireString(PARCJSONParser *parser, const char *string)
{
    PARCBuffer *buffer = _getBuffer(parser);

    for (const char *requiredCharacter = string; *requiredCharacter != 0; requiredCharacter++) {
        uint8_t actualCharacter = parcBuffer_GetUint8(buffer);
        if (actualCharacter != *requiredCharacter) {
            return false;
        }
    }
    return true;
}

PARCBuffer *
parcJSONParser_ParseString(PARCJSONParser *parser)
{
    PARCBuffer *result = NULL;

    PARCBuffer *buffer = _getBuffer(parser);
    if (parcBuffer_GetUint8(buffer) == '"') { // skip the initial '"' character starting the string.
        PARCBufferComposer *composer = parcBufferComposer_Create();

        while (parcBuffer_Remaining(buffer)) {
            uint8_t c = parcBuffer_GetUint8(buffer);
            if (c == '"') {
                // This is the only successful way to exit this while loop.
                result = parcBufferComposer_ProduceBuffer(composer);
                break;
            } else if (c == '\\') {
                c = parcBuffer_GetUint8(buffer);
                if (c == '"') {
                    // this special character passes directly into the composed string.
                } else if (c == '\\') {
                    // this special character passes directly into the composed string.
                } else if (c == '/') {
                    // this special character passes directly into the composed string.
                } else if (c == 'b') {
                    c = '\b';
                } else if (c == 'f') {
                    c = '\f';
                } else if (c == 'n') {
                    c = '\n';
                } else if (c == 'r') {
                    c = '\r';
                } else if (c == 't') {
                    c = '\t';
                } else if (c == 'u') {
                    // Not supporting unicode at this point.
                    trapNotImplemented("Unicode is not supported.");
                }
            } else if (iscntrl(c)) {
                // !! Syntax Error.
                break;
            }
            parcBufferComposer_PutChar(composer, c);
        }

        parcBufferComposer_Release(&composer);
    }
    return result;
}
