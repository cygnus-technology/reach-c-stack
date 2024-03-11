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
 *                          Copyright i3 Product Development 2023-2024
 * *
 ********************************************************************************************/

/**
 * @file      i3_log.c
 * @brief     Contains printf style logging functions as used by the Cygnus 
 *            Reach firmware stack. The log module is designed
 *            to be easily retargeted to the console IO of the
 *            target. It is designed to be easily removed for
 *            very small targets.  The "mask" facility allows
 *            users to turn on and off logging on a per function
 *            basis.
 * @author    Chuck Peplinski
 * @date      2024-02-21
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crcb_weak.h"
#include "i3_log.h"
#include "text_colors.h"

// reach-server.h defines the basic size of structures appropriate for the
// current transport means.  For example, BLE.
#include "reach-server.h"

#ifndef DEFAULT_LOG_MASK
  #define DEFAULT_LOG_MASK 0xFC7
#endif
static uint32_t sLogMask = DEFAULT_LOG_MASK;

/**
* @brief   i3_log_set_mask
* @details Sets the mask which determines whether or not a log statement 
*              generates output. See defines starting with LOG_MASK_.
*/
void i3_log_set_mask(uint32_t mask)
{
    sLogMask = mask;
}

/**
* @brief   i3_log_set_mask
* @details Gets the mask which determines whether or not a log statement 
*              generates output. See defines starting with LOG_MASK_.
*/
uint32_t i3_log_get_mask(void)
{
    return sLogMask;
}

// defined in reach-server.h
#ifndef INCLUDE_CLI_SERVICE
    /**
     * i3_log_set_remote_cli_enable() is a stub when the the CLI 
     *  service is not included.
     */
    int i3_log_set_remote_cli_enable(bool enable)
    {
        (void)enable;
        return cr_ErrorCodes_NO_SERVICE;
    }

    /**
     * i3_log_get_remote_cli_enable() is a stub when the the CLI 
     *  service is not included.
     */
    bool i3_log_get_remote_cli_enable()
    {
        return false;
    }

    // uses no memory for a log buffer.  Just printf().

    // There is no buffer

    /**
    * @brief   i3_log_get_remote_buffer
    * @details Retrieve the pointer and size of the remote buffer.  Set the size to 
    *          zero before returning so that the next string can overwrite the
    *          buffer.
    *          In case there is no CLI service there is no buffer.
    * @param   pRcli   pointer to char pointer of buffer.
    * @return  number of bytes currently in use by the remote cli buffer.
    */
    int i3_log_get_remote_buffer(char **pRcli)
    {
        *pRcli = NULL;
        return 0;
    }

    /**
    * @brief   i3_log
    * @details A printf style logging function conditioned on a mask. The mask is 
    *          and'ed with the control set by i3_log_set_mask(). The string is
    *          printed if the result is non-zero. See LOG_MASK_.
    *          In case there is no CLI service, simply printf.
    * @param   mask See LOG_MASK_.
    * @param   fmt As in printf.
    */
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

    /**
    * @brief   i3_log_set_remote_cli_enable
    * @details Enabling the remote CLI can generate significant BLE traffic. This 
    *          can slow down speed related things like file transfer.  Hence this
    *          API allows the remote command line to be easily supressed.
    *          The initial state can be set in reach-server.h 
    * @return  cr_ErrorCodes_NO_ERROR on success.
    */
    static bool sUseRemoteCLI = REMOTE_CLI_ECHO_ON_DEFAULT;  
    int i3_log_set_remote_cli_enable(bool enable)
    {
        sUseRemoteCLI = enable;
        return cr_ErrorCodes_NO_ERROR;
    }

    /**
    * @brief   i3_log_get_remote_cli_enable
    * @return  true if enabled.
    */
    bool i3_log_get_remote_cli_enable()
    {
        if (!crcb_enable_remote_cli())
            return false;
        return sUseRemoteCLI;
    }

    /**
     * When remote logging is enabled the strings for output are generated into a 
     * local buffer with length REACH_ERROR_BUFFER_LEN. 
     */
    static char sLog_rcliBuf[REACH_ERROR_BUFFER_LEN];
    static size_t sLog_rcliPtr = 0;

    #if 0  // obsolete
    /**
    * @brief   i3_log_get_remote_buffer
    * @details Retrieve the pointer and size of the remote buffer.  Set the size to 
    *          zero before returning so that the next string can overwrite the
    *          buffer.
    *          The assumption is that the contents of the buffer will be quickly 
    *          transmitted before anyone else writes into the buffer.
    * @param   pRcli   pointer to char pointer of buffer.
    * @return  number of bytes currently in use by the remote cli buffer.
    */
    int i3_log_get_remote_buffer(char **pRcli)
    {
        *pRcli = sLog_rcliBuf;
        size_t rval = sLog_rcliPtr;
        sLog_rcliPtr = 0;
        return rval;
    }
    #endif

    /**
    * @brief   i3_log
    * @details A printf style logging function conditioned on a mask. The mask is 
    *          and'ed with the control set by i3_log_set_mask(). The string is
    *          printed if the result is non-zero. See LOG_MASK_.
    *          ANSI color codes are inserted for errors (red), warnings (yellow) and
    *          Reach logging (cyan).  The color reset code and a \r\n are appended
    *          to all strings except for LOG_MASK_BARE. When the remote CLI is
    *          enabled the string is copied to the remote buffer and sent via
    *          crcb_cli_respond().
    * @param   mask See LOG_MASK_.
    */
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

        if (!i3_log_get_remote_cli_enable()) 
            return;

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

        // The i3_log module should not depend on the Reach stack.
        // Yet it is very convenient to emit the remote buffer right here.
        // This might want to be changed in some systems.
        extern int crcb_cli_respond(char *cli);

        sLog_rcliBuf[sLog_rcliPtr] = 0;
        crcb_cli_respond(sLog_rcliBuf);
        sLog_rcliPtr = 0;
    }

#endif  // def LOCAL_CLI_ONLY

#ifdef NO_REACH_LOGGING
    void i3_log_dump_buffer(const uint32_t mask,
                            const char *banner,
                            const uint8_t *ptr,
                            const size_t len)
    {
        (void)mask;
        (void)banner;
        (void)ptr;
        (void)len;
    }
#else
#define DUMP_WIDTH  25

    /**
    * @brief   i3_log_dump_buffer
    * @details Directly uses printf to produce a hex dump of a buffer. Used to view 
    *          the contents of coded buffers sent and received.
    *          The buffer is dumped locally, and not remotely. The width of the dump 
    *          is set by the DUMP_WIDTH defined just above this function.
    * @param   mask Enable or disable using the log module's mask feature..
    * @param   banner A header to be displayed describing the hex dump.
    * @param   ptr A buffer of bytes to be displayed.
    * @param   len The number of bytes to be displayed..
    */
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
#endif  // def NO_REACH_LOGGING


