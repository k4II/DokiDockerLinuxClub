/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Daniel Lezcano <daniel.lezcano at free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>
int netdev_set_flag(const char *name, int flag);
int lxc_netdev_up(const char *name);
int lxc_netdev_down(const char *name);
int lxc_veth_create(const char *name1, const char *name2);
int lxc_netdev_move_by_index(int ifindex, pid_t pid, const char* ifname);
int lxc_netdev_move_by_name(const char *ifname, pid_t pid, const char* newname);
int setup_private_host_hw_addr(char *veth1);
int lxc_bridge_attach(const char *bridge, const char *ifname);
int lxc_convert_mac(char *macaddr, struct sockaddr *sockaddr);
static int ip_addr_add(int family, int ifindex,
                       void *addr, void *bcast, void *acast, int prefix);
int lxc_ipv4_addr_add(int ifindex, struct in_addr *addr,
                      struct in_addr *bcast, int prefix);
static int ip_gateway_add(int family, int ifindex, void *gw);
int lxc_ipv4_gateway_add(int ifindex, struct in_addr *gw);
static int ip_route_dest_add(int family, int ifindex, void *dest);
int lxc_ipv4_dest_add(int ifindex, struct in_addr *dest);
int setup_hw_addr(char *hwaddr, const char *ifname);
char *lxc_mkifname(char *);
int lxc_netdev_delete_by_index(int ifindex);
int lxc_netdev_delete_by_name(const char *name);
void new_hwaddr(char *hwaddr);
#ifdef __cplusplus
}
#endif