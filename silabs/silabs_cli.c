/********************************************************************************************
 *    _ ____  ___             _         _     ___              _                        _
 *   (_)__ / | _ \_ _ ___  __| |_  _ __| |_  |   \ _____ _____| |___ _ __ _ __  ___ _ _| |_
 *   | ||_ \ |  _/ '_/ _ \/ _` | || / _|  _| | |) / -_) V / -_) / _ \ '_ \ '  \/ -_) ' \  _|
 *   |_|___/ |_| |_| \___/\__,_|\_,_\__|\__| |___/\___|\_/\___|_\___/ .__/_|_|_\___|_||_\__|
 *                                                                  |_|
 *                           -----------------------------------
 *                      (c) Copyright 2022-2023, i3 Product Development
 *
 * \brief silabs_cli.h/.c provides 'standard' Reach command line handling for
 *      a SiLabs based Reach product.
 *
 * Original Authors: Chuck.Peplinski, Joseph Peplinski and Sushanth Rao
 *
 ********************************************************************************************/

#include <string.h>

#include "sl_cli.h"
#include "sl_cli_handles.h"
#include "sl_cli_command.h"
#include "sl_bluetooth.h"
#include "nvm3_default.h"

#include "app.h"
#include "i3_log.h"
#include "version.h"
#include "cr_stack.h"
#include "reach_silabs.h"


/******************************************************************************
 *************************** CLI callback functions ***************************
 ******************************************************************************/

void slash()
{
  extern int rsl_stats();
  // Logs useful info about system state
  i3_log(LOG_MASK_ALWAYS, "System Status:");
  rsl_stats();

  uint16_t mtu_size;
  sl_bt_gatt_server_get_mtu(1, &mtu_size);
  i3_log(LOG_MASK_ALWAYS, "  Max ATT_MTU used by connection 1 is %u", mtu_size);
  i3_log(LOG_MASK_ALWAYS, "  Log mask 0x%x", i3_log_get_mask());

}

// void cli_lm(sl_cli_command_arg_t *arguments)
void cli_lm(sl_cli_command_arg_t *args)
{
  sl_cli_command_arg_t *arguments;
  const char *argStr;
  uint32_t logMask = i3_log_get_mask();
  int numRead;

  if (args)
  {
    arguments = (sl_cli_command_arg_t *)args;
    argStr = sl_cli_get_argument_string(arguments, 0);
    numRead = sscanf(argStr, "%x", (unsigned int*)&logMask);
  }
  else
  {
      argStr = crcb_get_command_line();
      numRead = sscanf(argStr, "lm %x", (unsigned int*)&logMask);
  }
  if (numRead != 1)
  {
    i3_log(LOG_MASK_ALWAYS, "Log mask not commanded.");
    i3_log(LOG_MASK_ALWAYS, "Log mask/Trace level is 0x%x", logMask);
    i3_log(LOG_MASK_ALWAYS, "  ALWAYS, ERROR and WARN are enabled");
    if (logMask & LOG_MASK_WEAK) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: WEAK is enabled", LOG_MASK_WEAK);
    else
        i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: WEAK is disabled", LOG_MASK_WEAK);
    if (logMask & LOG_MASK_WIRE) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: WIRE is enabled", LOG_MASK_WIRE);
    else
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: WIRE is disabled", LOG_MASK_WIRE);
    if (logMask & LOG_MASK_REACH) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: REACH is enabled", LOG_MASK_REACH);
    else
        i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: REACH is disabled", LOG_MASK_REACH);
    if (logMask & LOG_MASK_PARAMS) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: PARAMS is enabled", LOG_MASK_PARAMS);
    else
        i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: PARAMS is disabled", LOG_MASK_PARAMS);
    if (logMask & LOG_MASK_FILES) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: FILES is enabled", LOG_MASK_FILES);
    else
        i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: FILES is disabled", LOG_MASK_FILES);
    if (logMask & LOG_MASK_BLE) 
        i3_log(LOG_MASK_ALWAYS, "  mask 0x%x: BLE is enabled", LOG_MASK_BLE);
    else
        i3_log(LOG_MASK_ALWAYS, "    mask 0x%x: BLE is disabled", LOG_MASK_BLE);
    i3_log(LOG_MASK_ALWAYS, "  Other Valid log masks:");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_ACME      0x4000");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_DEBUG     0x8000");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_TIMEOUT  0x10000");
    return;
  }
  i3_log_set_mask(logMask);
  i3_log(LOG_MASK_ALWAYS, "The log mask is set to  0x%x", logMask);
}

void cli_rcli(sl_cli_command_arg_t *args)
{
   #ifndef ENABLE_REMOTE_CLI
    (void*)arguments;
    i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI is not enabled at compile time.");
    return;
  #else
    sl_cli_command_arg_t *arguments;
    const char *argStr;
    int enable = i3_log_get_remote_cli_enable();

    if (args)
    {
      arguments = (sl_cli_command_arg_t *)args;
      argStr = sl_cli_get_argument_string(arguments, 0);
      if (!strncmp(argStr, " on", 2))
          enable = true;
      else if (!strncmp(argStr, " off", 3))
          enable = false;
      else {
          i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI echo is %s.",
                 i3_log_get_remote_cli_enable() ? "ENABLED" : "DISABLED");
          return;
      }
    }
    else
    {
        argStr = crcb_get_command_line();
        if (!strncmp(argStr, "rcli on", 7))
            enable = true;
        else if (!strncmp(argStr, "rcli off", 8))
            enable = false;
        else {
            i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI echo is %s.",
                   i3_log_get_remote_cli_enable() ? "ENABLED" : "DISABLED");
            return;
        }
    }

    if (enable)
    {
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Enabled remote CLI echo.");
        i3_log_set_remote_cli_enable(true);
        return;
    }
    i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Disabled remote CLI echo.");
    i3_log_set_remote_cli_enable(false);
  #endif
}


// In recent tests the 1M Phy is faster than the 2M Phy.
// Going faster probably requires more tuning.
char gPhy = 1;
void cli_phy(sl_cli_command_arg_t *args)
{
    sl_cli_command_arg_t *arguments;
    const char *argStr;
    if (args)
    {
      arguments = (sl_cli_command_arg_t *)args;
      argStr = sl_cli_get_argument_string(arguments, 0);
      gPhy = sl_cli_get_argument_uint32(arguments, 0);
    }
    else
    {
        argStr = crcb_get_command_line();
        if (argStr[1] == '2') 
            gPhy = 2;
        else 
            gPhy = 1;
    }

    if (gPhy == 2)
        i3_log(LOG_MASK_ALWAYS, "Requesting 2M PHY");
    else
    {
        i3_log(LOG_MASK_ALWAYS, "Requesting 1M PHY");
        gPhy = 1;
    }
}

void cli_nvm(sl_cli_command_arg_t *args)
{
  int action = 0;  // nvm dump.  1 for clear.
  // nvm dump
  // nvm clear
  sl_cli_command_arg_t *arguments;
  const char *argStr;

  if (args)
  {
    arguments = (sl_cli_command_arg_t *)args;
    argStr = sl_cli_get_argument_string(arguments, 0);
    if (!strncmp(argStr, "clear", 6))
      action = 1;
    if (!strncmp(argStr, "init", 6))
      action = 2;
  }
  else
  {
      argStr = crcb_get_command_line();
      if (!strncmp(argStr, "nvm clear", 9))
        action = 1;
      if (!strncmp(argStr, "nvm init", 9))
        action = 2;
  }
  if (action == 1)
  {
      Ecode_t eCode = nvm3_eraseAll(nvm3_defaultHandle);
      i3_log(LOG_MASK_ALWAYS, "nvm3_eraseAll() returned 0x%x", eCode);
      return;
  }
  if (action == 2)
  {
      Ecode_t eCode = nvm3_eraseAll(nvm3_defaultHandle);
      i3_log(LOG_MASK_ALWAYS, "nvm3_eraseAll() returned 0x%x", eCode);
      init_param_repo();
      return;
  }

  // dump what is there
  size_t numObj = nvm3_enumObjects(nvm3_defaultHandle, NULL, 0, NVM3_KEY_MIN, NVM3_KEY_MAX);
  i3_log(LOG_MASK_ALWAYS, "Found %d NVM3 objects.", numObj);
  numObj = nvm3_enumDeletedObjects(nvm3_defaultHandle, NULL, 0, NVM3_KEY_MIN, NVM3_KEY_MAX);
  i3_log(LOG_MASK_ALWAYS, "Found %d deleted NVM3 objects.", numObj);

  #define NUM_KEYS  32
  nvm3_ObjectKey_t keyList[NUM_KEYS];  // uint32's
  int found = 0;
  numObj = nvm3_enumObjects(nvm3_defaultHandle, keyList, NUM_KEYS, NVM3_KEY_MIN, 128);
  for (int i=NVM3_KEY_MIN; i<NUM_KEYS; i++) {
      size_t dataLen;
      uint32_t objectType;

      nvm3_getObjectInfo(nvm3_defaultHandle, keyList[i], &objectType, &dataLen);
      if ( (objectType == NVM3_OBJECTTYPE_DATA) && (dataLen>0) ) {
          i3_log(LOG_MASK_ALWAYS, " Key %d is present, size %d.", keyList[i], dataLen);
          found++;
          if (found>= numObj) {
              break;
          }
      }
  }
  return;
}



/******************************************************************************
 *********************** CLI command info definitions *************************
 ******************************************************************************/
static const sl_cli_command_info_t cmd__ver =
SL_CLI_COMMAND(print_versions,
    "Gets information about the system version",
    "None",
      { SL_CLI_ARG_END,});

static const sl_cli_command_info_t cmd__slash =
SL_CLI_COMMAND(slash,
    "Gets information about the system status",
    "None",
      { SL_CLI_ARG_END,});

static const sl_cli_command_info_t cmd__lm =
SL_CLI_COMMAND(
    cli_lm,
    "Set log mask,",
    "log mask, hex number",
    { SL_CLI_ARG_STRING, SL_CLI_ARG_END });

static const sl_cli_command_info_t cmd__rcli =
SL_CLI_COMMAND(
    cli_rcli,
    "Enable or disable remote CLI echo,",
    "1 or 0",
    { SL_CLI_ARG_STRING, SL_CLI_ARG_END });

static const sl_cli_command_info_t cmd__phy =
SL_CLI_COMMAND(
    cli_phy,
    "Set PHY to 1 or 2 Mbps,",
    "1 (default) or 2 (fast)",
    { SL_CLI_ARG_UINT32, SL_CLI_ARG_END });

static const sl_cli_command_info_t cmd__nvm =
SL_CLI_COMMAND(
    cli_nvm,
    "Manage non volatile memory",
    "nvm ?: (summarize), nvm clear (erase all)",
    { SL_CLI_ARG_STRING, SL_CLI_ARG_END });

/******************************************************************************
 ***************************** CLI command table ******************************
 ******************************************************************************/
static sl_cli_command_entry_t command_table[] = {
  { "ver",          &cmd__ver,                    false},
  { "/",            &cmd__slash,                  false},
  { "lm",           &cmd__lm,                     false},
  { "rcli",         &cmd__rcli,                   false},
  { "phy",          &cmd__phy,                    false},
  { "nvm",          &cmd__nvm,                    false},
  { NULL,           NULL,                         false }, };

// Create the command group at the top level
static sl_cli_command_group_t main_group =
{
    { NULL },
    false, command_table 
};


/******************************************************************************
 ****************************** Local functions *******************************
 ******************************************************************************/
/*
 * Call this function to initialize the CLI arguments
 */
void
init_cygnus_cli ()
{
  sl_cli_command_add_command_group (sl_cli_cli_handle, &main_group);
}

