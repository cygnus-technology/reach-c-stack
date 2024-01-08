/*
 * Copyright (c) 2023-2024 i3 Product Development
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023
 *
 * \brief "i3_log.c" implments the i3_log interface using printf().
 *   The log must be easily retargeted to the console IO of the target.
 *   The log must be easily connected to the Reach CLI facility
 *   The log must easily be removed for very small targets.
 *   The "mask" facility allows users to turn on and off logging on a per function basis.
 *   See further comments in the H file.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "i3_log.h"
#include "text_colors.h"

// reach-server.h defines the basic size of structures appropriate for the
// current transport means.  For example, BLE.
#include "reach-server.h"

#define DUMP_WIDTH  25

static uint32_t sLogMask = 0x107C7; // 0x387;
//LOG_MASK_ALWAYS | LOG_MASK_ERROR | LOG_MASK_WARN |
// /* LOG_MASK_REACH |*/ LOG_MASK_PARAMS | LOG_MASK_FILES;

void i3_log_set_mask(uint32_t mask)
{
    sLogMask = mask;
}
uint32_t i3_log_get_mask(void)
{
    return sLogMask;
}

// defined in reach-server.h
#ifndef INCLUDE_CLI_SERVICE
int i3_log_set_remote_cli_enable(bool enable)
{
    (void)enable;
    return cr_ErrorCodes_NO_SERVICE;
}

bool i3_log_get_remote_cli_enable()
{
    return false;
}

// uses no memory for a log buffer.  Just printf().

// There is no buffer
int i3_log_get_remote_buffer(char **pRcli)
{
    *pRcli = NULL;
    return 0;
}

void i3_log(const uint32_t mask, const char *fmt, ...)
{
    va_list args;
    // you can't turn off ALWAYS ERROR and WARN.
    uint32_t localMask = sLogMask | LOG_MASK_ALWAYS | LOG_MASK_ERROR | LOG_MASK_WARN;

    if (0 == (mask & localMask)) return;

    if (mask & LOG_MASK_ERROR)
    {
        printf(TEXT_RED);
    }
    else if (mask & LOG_MASK_WARN)
    {
        printf(TEXT_YELLOW);
    }
    else if (mask & LOG_MASK_REACH)
    {
        printf(TEXT_CYAN);
    }
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf(TEXT_RESET);  // this could be prefixed by if (color mask)

    if (0 == (mask & LOG_MASK_BARE)) printf("\r\n");
}

#else

#include "reach-server.h"  // for remote buffer size

// If false, you have to turn it on explicitly.  See reach-server.h
static bool sUseRemoteCLI = REMOTE_CLI_ECHO_ON_DEFAULT;  
int i3_log_set_remote_cli_enable(bool enable)
{
    sUseRemoteCLI = enable;
    return 0;
}

bool i3_log_get_remote_cli_enable()
{
    return sUseRemoteCLI;
}

// Queue up logs into a local buffer which can be forwarded to the remote client.
static char sLog_rcliBuf[REACH_ERROR_BUFFER_LEN];
static size_t sLog_rcliPtr = 0;

// fetch the pointer and return the size.
// clear the buffer for next time.
int i3_log_get_remote_buffer(char **pRcli)
{
    *pRcli = sLog_rcliBuf;
    size_t rval = sLog_rcliPtr;
    sLog_rcliPtr = 0;
    return rval;
}

void i3_log(const uint32_t mask, const char *fmt, ...)
{
    va_list args;

    // first print it locally.
    // you can't turn off ALWAYS ERROR and WARN.
    uint32_t localMask = sLogMask | LOG_MASK_ALWAYS | LOG_MASK_ERROR | LOG_MASK_WARN;

    if (0 == (mask & localMask)) return;

    if (mask & LOG_MASK_ERROR)
    {
        printf(TEXT_RED);
    }
    else if (mask & LOG_MASK_WARN)
    {
        printf(TEXT_YELLOW);
    }
    else if (mask & LOG_MASK_REACH)
    {
        printf(TEXT_CYAN);
    }
    // printf("0x%x ", mask);  

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf(TEXT_RESET);  // this could be prefixed by if (color mask)

    if (0 == (mask & LOG_MASK_BARE)) printf("\r\n");

    if (!sUseRemoteCLI) return;

    // Then record any remote messages.
    localMask = LOG_MASK_ALWAYS | LOG_MASK_ERROR | LOG_MASK_WARN | LOG_MASK_REMOTE;

    if (0 == (mask & localMask)) return;

    // printf("remote: mask 0x%x, localMask 0x%x\n", mask, localMask);  

    sLog_rcliPtr = 0;
  #ifdef COLORS_REMOTE
    if (mask & LOG_MASK_ERROR)
    {
        // printf(TEXT_RED);
        sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_RED);
    }
    else if (mask & LOG_MASK_WARN)
    {
        // printf(TEXT_YELLOW);
        sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_YELLOW);
    }
    else if (mask & LOG_MASK_REACH)
    {
        // printf(TEXT_CYAN);
        sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_CYAN);
    }
  #endif  // def COLORS_REMOTE

    va_start(args, fmt);
    // vprintf(fmt, args);
    sLog_rcliPtr += vsnprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN-8, fmt, args);
    va_end(args);

  #ifdef COLORS_REMOTE
    // printf(TEXT_RESET);  // this could be prefixed by if (color mask)
    sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_RESET);
  #endif  // def COLORS_REMOTE

    if (0 == (mask & LOG_MASK_BARE))
    {
        // printf("\r\n");
        sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, "\r\n");
    }
    // printf("sLog_rcliPtr %ld\n", sLog_rcliPtr);

    extern int crcb_cli_respond(char *cli);

    sLog_rcliBuf[sLog_rcliPtr] = 0;
    crcb_cli_respond(sLog_rcliBuf);
    sLog_rcliPtr = 0;
}

#endif  // def LOCAL_CLI_ONLY

// this is never remote
void i3_log_dump_buffer(const uint32_t mask,
                        const char *banner,
                        const uint8_t *ptr,
                        const size_t len)
{
    if (0 == (mask & sLogMask)) return;
    printf("%s: %d bytes.\r\n  ", banner, (int)len);

    for (size_t i = 0; i < len; i++)
    {
        printf("%02X ", (unsigned char)ptr[i]);

        if ((i > 0) && (((i + 1) % DUMP_WIDTH) == 0)) printf("\r\n  ");
    }
    printf("\r\n");
}




