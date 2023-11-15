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

#include "app.h"
#include "i3_log.h"
#include "version.h"
#include "cr_stack.h"

/******************************************************************************
 *************************** CLI callback functions ***************************
 ******************************************************************************/

// the user application is expected to implement this.
extern void print_versions(void);


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


void cli_mtu(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  // this reports the size used so far, which is only 23 here.
  uint16_t mtu_size;
  sl_bt_gatt_server_get_mtu(1, &mtu_size);
  i3_log(LOG_MASK_ALWAYS, "The maximum ATT_MTU used by connection 1 is %u", mtu_size);

}

void cli_lm(sl_cli_command_arg_t *arguments)
{
  char *argStr = sl_cli_get_argument_string(arguments, 0);
  uint32_t logMask = i3_log_get_mask();
  int numRead = sscanf(argStr, "%x", (unsigned int*)&logMask);
  if (numRead != 1)
  {
    i3_log(LOG_MASK_ALWAYS, "Log mask not read.");
    i3_log(LOG_MASK_ALWAYS, "  Current log mask: 0x%x:", logMask);
    i3_log(LOG_MASK_ALWAYS, "  Valid log masks:");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_WEAK       0x20 ");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_WIRE       0x40 ");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_REACH      0x80 ");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_PARAMS     0x100");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_FILES      0x200");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_BLE        0x400");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_ACME      0x4000");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_DEBUG     0x8000");
    i3_log(LOG_MASK_ALWAYS, "    LOG_MASK_TIMEOUT  0x10000");
    i3_log(LOG_MASK_ALWAYS, "Log mask NOT set.");
    return;
  }
  i3_log_set_mask(logMask);
  i3_log(LOG_MASK_ALWAYS, "The log mask is set to  0x%x", logMask);
}

void cli_rcli(sl_cli_command_arg_t *arguments)
{
    char *argStr = sl_cli_get_argument_string(arguments, 0);
    int enable = i3_log_get_remote_cli_enable();
    int numRead = sscanf(argStr, "%d", &enable);

   #ifndef ENABLE_REMOTE_CLI
    i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI is not enabled at compile time.");
  #else
    if (numRead != 1)
    {
        i3_log(LOG_MASK_ALWAYS, TEXT_CLI "rcli: Remote CLI echo is %s.",
               i3_log_get_remote_cli_enable() ? "ENABLED" : "DISABLED");
        return;
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
void cli_phy(sl_cli_command_arg_t *arguments)
{
  gPhy = sl_cli_get_argument_uint32(arguments, 0);
  if (gPhy == 2)
      i3_log(LOG_MASK_ALWAYS, "Requesting 2M PHY");
  else
  {
      i3_log(LOG_MASK_ALWAYS, "Requesting 1M PHY");
      gPhy = 1;
  }
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

static const sl_cli_command_info_t cmd__mtu =
SL_CLI_COMMAND(
    cli_mtu,
    "Show current MTU usage",
    "None",
    { SL_CLI_ARG_END });

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



/******************************************************************************
 ***************************** CLI command table ******************************
 ******************************************************************************/
static sl_cli_command_entry_t command_table[] = {
  { "ver",          &cmd__ver,                    false},
  { "/",            &cmd__slash,                  false},
  { "mtu",          &cmd__mtu,                    false},
  { "lm",           &cmd__lm,                     false},
  { "rcli",         &cmd__rcli,                   false},
  { "phy",          &cmd__phy,                    false},
  { NULL,           NULL,                         false }, };

// Create the command group at the top level
static sl_cli_command_group_t main_group =
  {
    { NULL },
  false, command_table };

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

