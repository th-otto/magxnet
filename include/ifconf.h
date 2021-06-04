#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "stsocket.h"

#if defined(__PUREC__) && !defined(__MINT__)
# include <tos.h>
#else
# include <osbind.h>
# include <mintbind.h>
# include <sys/stat.h>
#endif

#include <sys/ioctl.h>
#include "sockdev.h"
#include <net/route.h>

extern struct magxnet_cookie *sockets_dev;
extern long cookie;
extern int sock_fd;
extern char ifname_link[IFNAMSIZ];

int open_socket(int flag);
int get_if_flags(const char *ifname);
void set_if_flags(const char *ifname, int flags);
int get_link_flags(const char *ifname, int *flags);
int set_link_flags(const char *ifname, int flags);
int set_addr(const char *ifname, short which, in_addr_t addr);
int set_mtu_metric(const char *ifname, short which, long val);
int if_link(const char *device, const char *ifname);
struct in_addr resolve(const char *name);
int is_host(struct in_addr ina);
int add_route(int argc, char *argv[]);
