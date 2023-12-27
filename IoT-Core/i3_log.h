/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                          Copyright i3 Product Development 2023
 *
 * \brief "i3_log.h" specifies the interface to a basic logging function.
 *   The log must be easily retargeted to the console IO of the target.
 *   The log must be easily connected to the Reach CLI facility
 *   The log must easily be removed for very small targets.
 *   The "mask" facility allows users to turn on and off logging on a per function basis.
 *
 * Original Author: Chuck.Peplinski
 *
 ********************************************************************************************/

#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "i3_error.h"
#include "text_colors.h"

#ifdef __cplusplus
extern "C"
{
#endif

// The "mask" concept here is chosen over the "level" concept so that masks
// can be assigned to specific features.  Turning on all of the masks is
// likely to be too much information.  You can turn on the logging from
// only the modules that you want to debug.
// Reach uses a few bits.  You can assign your own bits.

//  The lowest nibble is reserved to system things.
#define LOG_MASK_ALWAYS     0x01    // Cannot be supressed
#define LOG_MASK_ERROR      0x02    // Prints red, cannot be supressed
#define LOG_MASK_WARN       0x04    // Prints yellow, cannot be supressed
#define LOG_MASK_BARE       0x08    // trailing \n is omitted
#define LOG_MASK_REMOTE     0x10    // Set this to indicate that a message should be shared remotely.

// Thes used by Reach features.  Enable them to debug and understand.
#define LOG_MASK_WEAK       0x20    // print in weak functions
#define LOG_MASK_WIRE       0x40    // show what is on the wire
#define LOG_MASK_REACH      0x80    // show reach protocol exchanges
#define LOG_MASK_PARAMS     0x100
#define LOG_MASK_FILES      0x200
#define LOG_MASK_BLE        0x400
#define LOG_MASK_ACME      0x4000
#define LOG_MASK_DEBUG     0x8000
#define LOG_MASK_TIMEOUT  0x10000

// Higher bits can be defined and used by the application
// #include "app_log_masks.h"

//#define NO_LOGGING
#ifdef NO_LOGGING
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

// Set and get the mask that control output per module
void i3_log_set_mask(uint32_t mask);
uint32_t i3_log_get_mask(void);

// if remote CLI is enabled at build, this can disable it at run time.
int i3_log_set_remote_cli_enable(bool enable);
bool i3_log_get_remote_cli_enable();

// if remote CLI support is enabled this gets a pointer to the remote buffer.
// Otherwise pRcli is set to NULL.
// Returns the current (valid) size of the buffer.
int i3_log_get_remote_buffer(char **pRcli);

// The basic log function, like printf.
// The mask passed here is compared with the "log mask" for the system.
// The string prints if the mask bit is enabled.
void i3_log(const uint32_t mask, const char *fmt, ...);

// A utility used to display raw binary data.
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
