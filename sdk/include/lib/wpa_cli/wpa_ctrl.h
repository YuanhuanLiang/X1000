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
#ifndef _WPA_CTRL_H
#define _WPA_CTRL_H
#include <sys/un.h>

#define CONFIG_CTRL_IFACE_DIR             "/var/run/wpa_supplicant"
#define CONFIG_CTRL_IFACE_CLIENT_DIR      "/tmp"
#define CONFIG_CTRL_IFACE_CLIENT_PREFIX   "wpa_ctrl_"


/** Interactive request for identity/password/pin */
#define WPA_CTRL_REQ "CTRL-REQ-"

/** Response to identity/password/pin request */
#define WPA_CTRL_RSP "CTRL-RSP-"


/**
 * The control interface data, initialized by wpa_ctrl_open().
 */
struct wpa_ctrl {
    int sockfd;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};


/**
 * wpa_ctrl_attach - Send "ATTACH" command to wpa_supplicant and judge the reply message
 * @ctrl: Control inferface data from wpa_ctrl_open().
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 */
int wpa_ctrl_attach(struct wpa_ctrl *ctrl);

/**
 * wpa_ctrl_detach - Send "DETACH" command to wpa_supplicant and judge the reply message
 * @ctrl: Control inferface data from wpa_ctrl_open().
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 */
int wpa_ctrl_detach(struct wpa_ctrl *ctrl);

/**
 * wpa_ctrl_recv - Receive message from wpa_supplicant through domain socket.
 * @ctrl: Control inferface data from wpa_ctrl_open().
 * @reply: Pointer of reply message buffer.
 * @reply_len: The lenght of reply message from wpa_supplicant.
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 */
int wpa_ctrl_recv(struct wpa_ctrl *ctrl, char *reply, size_t *reply_len);

/**
 * wpa_ctrl_get_fd - Get the socket file descriptor of struct wpa_ctrl.
 * @ctrl: Control inferface data from wpa_ctrl_open().
 *
 * Returns: Return the value of ctrl->sockfd.
 */
int wpa_ctrl_get_fd(struct wpa_ctrl *ctrl);

/**
 * wpa_ctrl_command - Send a request command to wpa_supplicant and get the reply message
 * @ctrl: Control inferface data from wpa_ctrl_open().
 * @cmd: The command send to wpa_supplicant.
 * @reply: Pointer of reply message buffer.
 * @reply_len: The lenght of reply message from wpa_supplicant.
 *
 * Returns: The value equal to 0 indicates success, or less than 0 indicates failure.
 *
 * This function is used to send a request command to wpa_supplicant and get the reply
 * message. You should judge the return value of this function, and then processs the
 * reply message.
 */
int wpa_ctrl_command(struct wpa_ctrl *ctrl, char *cmd, char *reply, size_t *reply_len);

/**
 * wpa_ctrl_open - Open a control interface to wpa_supplicant/hostapd.
 * @ctrl_path: Path for UNIX domain sockets; ignored if UDP sockets are used.
 *
 * Returns: Pointer to abstract control interface data or NULL on failure.
 *
 * This function is used to open a control interface to wpa_supplicant/hostapd
 * when the sockets path for client need to be specified explicitly. Default
 * ctrl_path is usually /var/run/wpa_supplicant or /var/run/hostapd and client
 * socket path is /tmp.
 */
struct wpa_ctrl *wpa_ctrl_open(const char *ctrl_path);

/**
 * wpa_ctrl_close - Close a control interface to wpa_supplicant/hostapd.
 * @ctrl: Control interface data from wpa_ctrl_open().
 *
 * This function is used to close a control interface.
 */
void wpa_ctrl_close(struct wpa_ctrl *ctrl);

#endif /* _WPA_CTRL_H */
