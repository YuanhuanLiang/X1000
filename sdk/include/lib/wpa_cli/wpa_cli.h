/*
 *  Copyright (C) 2017, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 * This software may be distributed, used, and modified under the terms of
 * BSD license:

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 *  You should have received a copy of the BSD License along with this program.
 *
 */
#ifndef _WPA_CLI_H
#define _WPA_CLI_H

#define WPA_CLI_MAX_ARGS         10
#define WPA_CLI_MAX_REPLY_LEN    1024 * 4
#define WPA_CLI_PRINT_REPLY      0
#define WPA_CLI_MAX_CMD_LEN      256


/**
 * wpa_cli_print_help - Print the command help message.
 * @cmd: A specified command or be NULL will print all commands message.
 *
 * This function is used to print a specified command or all commands help message.
 */
void wpa_cli_print_help(const char *cmd);

/**
 * wpa_cli_get_default_ifname - Get the default interface name.
 * @ifname: Point to the interface name buffer.
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 *
 * This function is used to get then default interface name. Default ifname is usually
 * wlan0.
 */
int wpa_cli_get_default_ifname(char *ifname);

/**
 * wpa_cli_open_connection - Open and connect to wpa_supplicant/hostapd.
 * @ifname: The default interface name.
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 *
 * This function is used to control interface to wpa_supplicant/hostapd, and create a
 * UNIX domain socket to connect the wpa_supplicant/hostapd.
 */
int wpa_cli_open_connection(const char *ifname);

/**
 * wpa_cli_cmd_request - Send a request command to wpa_supplicant/hostapd.
 * @cmd: Point to the command string.
 * @p_msg: This parameter is used to return the reply message buffer pointer.
 * @msg_len: This parameter is used to return the reply message lenght.
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 *
 * This function is used to send a request command to wpa_supplicant/hostapd, and get the
 * command reply message from wpa_supplicant/hostapd by p_msg and msg_len parameters. If
 * you don't care the reply message, set p_msg and msg_len parameters to NULL. You should
 * judge the return value of this function, and then processsing the reply message.
 */
int wpa_cli_cmd_request(char *cmd, char **p_msg, size_t *msg_len);

/**
 * wpa_cli_cleanup - Close control interface and free resources before the processs exit.
 *
 * This function is used to close control inferface, unlink the UNIX domain socket and
 * free the system resources occupied by processs, You must call it before processs exit.
 */
void wpa_cli_cleanup(void);

#endif /* _WPA_CLI_H */
