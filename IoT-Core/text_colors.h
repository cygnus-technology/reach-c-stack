/***********************************************************************************************************************
 * @brief   Serial Handler
 * @author  i3 Product Development (C. Peplinski)
 * @date    2021
 **********************************************************************************************************************/
#ifndef _TEXT_COLORS_H
#define _TEXT_COLORS_H
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
    
    #define TEXT_BOLD_BRIGHT_BLACK      TEXT_BOLD TEXT_BRIGHT_BLACK   // Bold Bright Black
    #define TEXT_BOLD_BRIGHT_RED        TEXT_BOLD TEXT_BRIGHT_RED     // Bold Bright Red
    #define TEXT_BOLD_BRIGHT_GREEN      TEXT_BOLD TEXT_BRIGHT_GREEN   // Bold Bright Green
    #define TEXT_BOLD_BRIGHT_YELLOW     TEXT_BOLD TEXT_BRIGHT_YELLOW  // Bold Bright Yellow
    #define TEXT_BOLD_BRIGHT_BLUE       TEXT_BOLD TEXT_BRIGHT_BLUE    // Bold Bright Blue
    #define TEXT_BOLD_BRIGHT_MAGENTA    TEXT_BOLD TEXT_BRIGHT_MAGENTA // Bold Bright Magenta
    #define TEXT_BOLD_BRIGHT_CYAN       TEXT_BOLD TEXT_BRIGHT_CYAN    // Bold Bright Cyan
    #define TEXT_BOLD_BRIGHT_WHITE      TEXT_BOLD TEXT_BRIGHT_WHITE   // Bold Bright White

#endif  // ndef TEXT_RESET
#endif  // def _TEXT_COLORS_H
