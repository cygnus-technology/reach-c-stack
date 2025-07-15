/*
 * Copyright (c) 2023-2025 i3 Product Development
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
 *                          Copyright i3 Product Development 2023-2025
 * *
 ********************************************************************************************/

/**
 * @file      i3_log.c
 * @brief     Includes a weak definition of the 
 *            printf-style logging functions as used by the
 *            Cygnus Reach firmware stack in a bare metal
 *            situation. An RTOS based system must override this
 *            in its integration package.
 * @author    Chuck Peplinski
 * @date      2025-06-26
 */

#include "reach-server.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crcb_weak.h"
#include "i3_log.h"
#include "text_colors.h"

#ifndef DEFAULT_LOG_MASK
  #define DEFAULT_LOG_MASK 0
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
#ifdef INCLUDE_CLI_SERVICE

    /**
     * When remote logging is enabled the strings for output are generated into a 
     * local buffer with length REACH_ERROR_BUFFER_LEN. 
     * This is static here to work with the weak implementation. 
     * A function is provided to obtain it for derived 
     * implementations. 
     */
    static char sLog_rcliBuf[REACH_ERROR_BUFFER_LEN];
    /**
    * @brief   i3_log_get_remote_buffer
    * @details Retrieve the pointer and size of the remote buffer. 
    *           Intended to be used by implementations that override
    *           the weak i3_log() given here.
    * @param   pRcli    pointer to char pointer of buffer.
    * @param   pBufSize  pointer to the size of the buffer.
    * @return  zero.
    */
    int i3_log_get_remote_buffer(char **pRcli, size_t *bufSize)
    {
        *pRcli = sLog_rcliBuf;
        *bufSize = sizeof(sLog_rcliBuf) ;
        return 0;
    }

  #ifdef DISABLE_REMOTE_CLI
    static bool sUseRemoteCLI = false;
  #else
    static bool sUseRemoteCLI = true;
  #endif

    /**
     * i3_log_set_remote_cli_enable() is a stub when the the CLI 
     *  service is not included.
     */
    int i3_log_set_remote_cli_enable(bool enable)
    {
        sUseRemoteCLI = enable;
        return 0;
    }

    /**
     * i3_log_get_remote_cli_enable() is a stub when the the CLI 
     *  service is not included.
     */
    bool i3_log_get_remote_cli_enable()
    {
        return sUseRemoteCLI;
    }

#else
    int i3_log_get_remote_buffer(char **pRcli, size_t *bufSize)
    {
        *pRcli = NULL;
        *bufSize = 0;
        return 0;
    }

    int i3_log_set_remote_cli_enable(bool enable)
    {
        (void)enable;
        return cr_ErrorCodes_NO_SERVICE;
    }

    bool i3_log_get_remote_cli_enable()
    {
        return false;
    }
#endif // def INCLUDE_CLI_SERVICE


#ifdef NO_REACH_LOGGING
    void __attribute__((weak)) i3_log(const uint32_t mask, const char *fmt, ...)
    {
    }
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
    /**
    * @brief   i3_log
    * @details A printf style logging function conditioned on a mask. The mask is 
    *          and'ed with the control set by i3_log_set_mask(). The string is
    *          printed if the result is non-zero. See LOG_MASK_.
    *          ANSI color codes are inserted for errors (red), warnings (yellow) and
    *          Reach logging (cyan).  The color reset code and a \r\n are appended
    *          to all strings except for LOG_MASK_BARE. When the remote CLI is
    *          enabled the string is copied to the remote buffer and sent via
    *          pvtCr_cli_respond().
    * @param   mask See LOG_MASK_.
    */
    void __attribute__((weak)) i3_log(const uint32_t mask, const char *fmt, ...)
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

    #ifdef INCLUDE_CLI_SERVICE

    /**
     * When remote logging is enabled the strings for output are generated into a 
     * local buffer with length REACH_ERROR_BUFFER_LEN. 
     */
    static char sLog_rcliBuf[REACH_ERROR_BUFFER_LEN];
    static size_t sLog_rcliPtr = 0;



        if (!i3_log_get_remote_cli_enable()) 
            return;

        // Then record any remote messages.
        localMask = LOG_MASK_ALWAYS | LOG_MASK_ERROR | LOG_MASK_WARN | LOG_MASK_REMOTE;

        if (0 == (mask & localMask)) return;

        sLog_rcliPtr = 0;
      #ifdef COLORS_REMOTE
        if (mask & LOG_MASK_ERROR)
        {
            sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_RED);
        }
        else if (mask & LOG_MASK_WARN)
        {
            sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_YELLOW);
        }
        else if (mask & LOG_MASK_REACH)
        {
            sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_CYAN);
        }
      #endif  // def COLORS_REMOTE

        va_start(args, fmt);
        sLog_rcliPtr += vsnprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN-8, fmt, args);
        va_end(args);

      #ifdef COLORS_REMOTE
        sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, TEXT_RESET);
      #endif  // def COLORS_REMOTE

        if (0 == (mask & LOG_MASK_BARE))
        {
            sLog_rcliPtr += snprintf(&sLog_rcliBuf[sLog_rcliPtr], REACH_ERROR_BUFFER_LEN, "\r\n");
        }

        // The i3_log module should not depend on the Reach stack.
        // Yet it is very convenient to emit the remote buffer right here.
        // This might want to be changed in some systems.
        extern int pvtCr_cli_respond(char *cli);

        sLog_rcliBuf[sLog_rcliPtr] = 0;
        pvtCr_cli_respond(sLog_rcliBuf);
        sLog_rcliPtr = 0;
    #endif  // def INCLUDE_CLI_SERVICE
    }


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
    void __attribute__((weak)) i3_log_dump_buffer(const uint32_t mask,
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
#endif // def NO_REACH_LOGGING

#ifdef DEV_BUILD
  char gFatalErrorBuffer[256];
#endif
  
void __attribute__((weak)) i3_log_fatal_error(char * msg)
{
    printf(msg);
  #ifdef DEV_BUILD
    __asm__("bkpt");
    exit(1); 
  #else
    // reboot
    exit(1); 
  #endif

}

