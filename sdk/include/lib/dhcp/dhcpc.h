/* dhcpc.h */
#ifndef _DHCPC_H
#define _DHCPC_H

#define VERSION           "0.9.8"

#define INIT_SELECTING    0
#define REQUESTING        1
#define BOUND             2
#define RENEWING          3
#define REBINDING         4
#define INIT_REBOOT       5
#define RENEW_REQUESTED   6
#define RELEASED          7
#define REQUESTED         8

#define LISTEN_NONE       0
#define LISTEN_KERNEL     1
#define LISTEN_RAW        2

#define DEFAULT_SCRIPT    "/usr/share/udhcpc/default.script"

struct client_config_t {
    char foreground;                /* Do not fork */
    char quit_after_lease;          /* Quit after obtaining lease */
    char abort_if_no_lease;         /* Abort if no lease */
    char background_if_no_lease;    /* Fork to background if no lease */
    char *interface;                /* The name of the interface to use */
    char *pidfile;                  /* Optionally store the process ID */
    char *script;                   /* User script to run at dhcp events */
    unsigned char *clientid;        /* Optional client id to use */
    unsigned char *hostname;        /* Optional hostname to use */
    int ifindex;                    /* Index number of the interface to use */
    unsigned char arp[6];           /* Our arp address */
};

struct dhcpc_config_t {
    char quit_after_lease;          /* Quit after obtaining lease */
    char *interface;                /* The name of the interface to use */
    char *script;                   /* User script to run at dhcp events */
    unsigned char *clientid;        /* Optional client id to use */
    unsigned char *hostname;        /* Optional hostname to use */
    int ifindex;                    /* Index number of the interface to use */
    unsigned char arp[6];           /* Our arp address */
};

struct dhcpc_t {
    int fd;
    unsigned char state;
    unsigned char cancel;
    unsigned char listen_mode;
    unsigned long server_addr;
    unsigned long requested_ip;
    struct dhcpc_config_t dhcpc_config;
};

int udhcpc_main(struct dhcpc_t *dhcpc);
void udhcpc_release(struct dhcpc_t *dhcpc);

#endif
