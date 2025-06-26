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
 *                          Copyright i3 Product Development 2023-2025
 * *
 ********************************************************************************************/

/**
 * @file      i3_log.c
 * @brief     Provides a weak definition of the printf-style 
 *            logging functions as used by the Cygnus Reach
 *            firmware stack. The useful implementations are
 *            provided in the chip specific integration
 *            packages.
 * @author    Chuck Peplinski
 * @date      2025-06-26
 * @copyright (c) Copyright 2023-2025 i3 Product Development. 
 * All Rights Reserved. The Cygngus Reach firmware stack is 
 * shared under an MIT license. 
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crcb_weak.h"
#include "i3_log.h"
#include "text_colors.h"

void __attribute__((weak)) i3_log_set_mask(uint32_t mask)
{
}

uint32_t __attribute__((weak)) i3_log_get_mask(void)
{
    return 0;
}

int __attribute__((weak)) i3_log_set_remote_cli_enable(bool enable)
{
    (void)enable;
    return cr_ErrorCodes_NOT_IMPLEMENTED;
}

bool __attribute__((weak)) i3_log_get_remote_cli_enable()
{
    return false;
}

int __attribute__((weak)) i3_log_get_remote_buffer(char **pRcli)
{
    *pRcli = NULL;
    return 0;
}

void __attribute__((weak)) i3_log(const uint32_t mask, const char *fmt, ...)
{
}

void __attribute__((weak)) i3_log_dump_buffer(const uint32_t mask,
                            const char *banner,
                            const uint8_t *ptr,
                            const size_t len)
{
    (void)mask;
    (void)banner;
    (void)ptr;
    (void)len;
}

