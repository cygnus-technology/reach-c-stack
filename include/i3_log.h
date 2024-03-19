/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023-24
 *
 ********************************************************************************************/

#ifndef LOG_H_
#define LOG_H_

/**
 * @file      i3_log.h
 * @brief     Contains printf style logging functions as used by the Cygnus 
 *            Reach firmware stack. The log module is designed
 *            to be easily retargeted to the console IO of the
 *            target. It is designed to be easily removed for
 *            very small targets.  The "mask" facility allows
 *            users to turn on and off logging on a per function
 *            basis.
 * @copyright (c) Copyright 2023 i3 Product Development. All Rights Reserved.
 * The Cygngus Reach firmware stack is shared under an MIT license.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "reach-server.h"  // excludes logging
#include "i3_error.h"
#include "text_colors.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * The logging system used by Reach relies on a "mask" to control the amount of 
 * logging. The "mask" concept here is chosen over the "level" concept so that 
 * masks can be assigned to specific features.  Turning on all of the masks is 
 * likely to be too much information.  You can turn on the logging from only the 
 * modules that you want to debug. Reach uses a few bits.  You can assign your 
 * own bits. 
 * The lowest nibble is reserved to system things. 
 * Higher bits can be defined and used by the application. 
 */

///  The lowest nibble is reserved to system things.
#define LOG_MASK_ALWAYS     0x01    ///< Cannot be supressed
#define LOG_MASK_ERROR      0x02    ///< Prints red, cannot be supressed
#define LOG_MASK_WARN       0x04    ///< Prints yellow, cannot be supressed
#define LOG_MASK_BARE       0x08    ///< trailing \n is omitted
#define LOG_MASK_REMOTE     0x10    ///< Set this to indicate that a message should be shared remotely.

/// These used by Reach features.  Enable them to debug and understand.
#define LOG_MASK_WEAK       0x20    ///< print in weak functions
#define LOG_MASK_WIRE       0x40    ///< show what is on the wire
#define LOG_MASK_REACH      0x80    ///< show reach protocol exchanges
#define LOG_MASK_PARAMS     0x100   ///< show parameter handling
#define LOG_MASK_FILES      0x200   ///< show file handling
#define LOG_MASK_BLE        0x400   ///< show BLE handling
#define LOG_MASK_DEBUG      0x800   ///< show other reach features

/// Logging can be completely excluded from the build by
/// defining NO_REACH_LOGGING in reach-server.h.
#ifdef NO_REACH_LOGGING
    #define I3_LOG(m, f, ...)
    #define LOG_REACH(format, ...)
    #define LOG_ERROR(format, ...)
    #define LOG_DUMP_WIRE(banner, buf, len)
    #define LOG_DUMP_MASK(mask, banner, buf, len)
#else
    #define I3_LOG(m, f, ...)     i3_log(m, f, ##__VA_ARGS__)
    #define LOG_REACH(format, ...)                                                 \
        I3_LOG(LOG_MASK_REACH, "[%s][%s] " format, __FILE__, __func__, ##__VA_ARGS__)
    #define LOG_ERROR(format, ...)                                                 \
        i3_log(LOG_MASK_ERROR, "[%s][%s] " format, __FILE__, __func__, ##__VA_ARGS__)
    #define LOG_DUMP_WIRE(banner, buf, len)                                        \
        i3_log_dump_buffer(LOG_MASK_WIRE, banner, buf, len)
    #define LOG_DUMP_MASK(mask, banner, buf, len)                                  \
        i3_log_dump_buffer(mask, banner, buf, len)
#endif  // def NO_LOGGING

/**
* @brief   i3_log_set_mask
* @details Sets the mask which determines whether or not a log statement 
*              generates output. See defines starting with LOG_MASK_.
*/
void i3_log_set_mask(uint32_t mask);


/**
* @brief   i3_log_set_mask
* @details Gets the mask which determines whether or not a log statement 
*              generates output. See defines starting with LOG_MASK_.
*/
uint32_t i3_log_get_mask(void);

/**
* @brief   i3_log_set_remote_cli_enable
* @details Enabling the remote CLI can generate significant BLE traffic. This 
*          can slow down speed related things like file transfer.  Hence this
*          API allows the remote command line to be easily
*          supressed.  The initial state can be set in
*          reach-server.h
* @return  cr_ErrorCodes_NO_ERROR on success.
*/
int i3_log_set_remote_cli_enable(bool enable);

/**
* @brief   i3_log_get_remote_cli_enable
* @return  true if enabled.
*/
bool i3_log_get_remote_cli_enable();

#if 0
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
int i3_log_get_remote_buffer(char **pRcli);
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
void i3_log(const uint32_t mask, const char *fmt, ...);

/**
* @brief   i3_log_dump_buffer
* @details Logs a hex dump of a buffer. Used to view the 
*          contents of coded buffers sent and received. The
*          buffer is dumped locally, and not remotely. The width
*          of the dump is set by the DUMP_WIDTH defined just
*          above this function.
* @param   mask Enable or disable using the log module's mask feature..
* @param   banner A header to be displayed describing the hex dump.
* @param   ptr A buffer of bytes to be displayed.
* @param   len The number of bytes to be displayed..
*/
void i3_log_dump_buffer(const uint32_t mask,
                        const char *banner,
                        const uint8_t *ptr,
                        const size_t len);

#ifdef __cplusplus
}
#endif


#ifdef NO_LOGGING
#else
#endif


// Text color escape codes
#ifndef TEXT_RESET
// escape codes so traces (printf) can have colors:
/// CSI - Control Sequence Introducer
    #define CSI                 "\033["
// Text Reset
    #define TEXT_RESET          CSI"0m"
// Bold define
    #define TEXT_BOLD           CSI"1m"           // Bold
// Colors
    #define TEXT_BLACK          CSI"30m"          // Black
    #define TEXT_RED            CSI"31m"          // Red
    #define TEXT_GREEN          CSI"32m"          // Green
    #define TEXT_YELLOW         CSI"33m"          // Yellow
    #define TEXT_BLUE           CSI"34m"          // Blue
    #define TEXT_MAGENTA        CSI"35m"          // Magenta
    #define TEXT_CYAN           CSI"36m"          // Cyan
    #define TEXT_WHITE          CSI"37m"          // White

// Bright Colors
    #define TEXT_BRIGHT_BLACK   CSI"90m"          // Bright Black
    #define TEXT_BRIGHT_RED     CSI"91m"          // Bright Red
    #define TEXT_BRIGHT_GREEN   CSI"92m"          // Bright Green
    #define TEXT_BRIGHT_YELLOW  CSI"93m"          // Bright Yellow
    #define TEXT_BRIGHT_BLUE    CSI"94m"          // Bright Blue
    #define TEXT_BRIGHT_MAGENTA CSI"95m"          // Bright Magenta
    #define TEXT_BRIGHT_CYAN    CSI"96m"          // Bright Cyan
    #define TEXT_BRIGHT_WHITE   CSI"97m"          // Bright White

// Bold (
    #define TEXT_BOLDBLACK      TEXT_BOLD TEXT_BLACK   // Bold Black
    #define TEXT_BOLDRED        TEXT_BOLD TEXT_RED     // Bold Red
    #define TEXT_BOLDGREEN      TEXT_BOLD TEXT_GREEN   // Bold Green
    #define TEXT_BOLDYELLOW     TEXT_BOLD TEXT_YELLOW  // Bold Yellow
    #define TEXT_BOLDBLUE       TEXT_BOLD TEXT_BLUE    // Bold Blue
    #define TEXT_BOLDMAGENTA    TEXT_BOLD TEXT_MAGENTA // Bold Magenta
    #define TEXT_BOLDCYAN       TEXT_BOLD TEXT_CYAN    // Bold Cyan
    #define TEXT_BOLDWHITE      TEXT_BOLD TEXT_WHITE   // Bold White

// Bright Colors
    #define TEXT_BOLD_BRIGHT_BLACK      TEXT_BOLD TEXT_BRIGHT_BLACK   // Bold Bright Black
    #define TEXT_BOLD_BRIGHT_RED        TEXT_BOLD TEXT_BRIGHT_RED     // Bold Bright Red
    #define TEXT_BOLD_BRIGHT_GREEN      TEXT_BOLD TEXT_BRIGHT_GREEN   // Bold Bright Green
    #define TEXT_BOLD_BRIGHT_YELLOW     TEXT_BOLD TEXT_BRIGHT_YELLOW  // Bold Bright Yellow
    #define TEXT_BOLD_BRIGHT_BLUE       TEXT_BOLD TEXT_BRIGHT_BLUE    // Bold Bright Blue
    #define TEXT_BOLD_BRIGHT_MAGENTA    TEXT_BOLD TEXT_BRIGHT_MAGENTA // Bold Bright Magenta
    #define TEXT_BOLD_BRIGHT_CYAN       TEXT_BOLD TEXT_BRIGHT_CYAN    // Bold Bright Cyan
    #define TEXT_BOLD_BRIGHT_WHITE      TEXT_BOLD TEXT_BRIGHT_WHITE   // Bold Bright White

    #define EMPTY_LINE          CSI"1K"
    #define CLR_TO_END          CSI"0K"
    #define CURSOR_LEFT         CSI"1D"
    #define CURSOR_RIGHT        CSI"1C"


#endif  // ndef TEXT_RESET


#endif /* LOG_H_ */
