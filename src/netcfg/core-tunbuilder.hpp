//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C) 2018-  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C) 2018-  Arne Schwabe <arne@openvpn.net>
//

/**
 * @file   core-tunbuilder.hpp
 *
 * @brief This class will reuse the OpenVPN 3 Core library TunBuilder
 *        to create and manage a tun/dco devices
 */

#pragma once

#include <string>

#include <openvpn/common/rc.hpp>

#include "netcfg-signals.hpp"

class NetCfgDevice;


namespace openvpn {
class CoreTunbuilder : public RC<thread_safe_refcount>
{
  public:
    virtual int establish(NetCfgDevice &netCfgDevice) = 0;
    virtual void teardown(const NetCfgDevice &netCfgDevice, bool disconnect) = 0;
};


// Forward declaration to allow friend
class CoreTunbuilderImpl;


/**
 * Function that binds the the fd to the device of the best route to remote
 * @param fd Socket to bind
 * @param remote remote host to lookup
 * @param ipv6 is remote ipv6
 */
void protect_socket_binddev(int fd, const std::string &remote, bool ipv6);


/**
 * Function that set the so_mark for socket
 * @param fd Socket for the SO_MARK to set
 * @param remote remote host to connect to (for logging only)
 * @param somark the value to set SO_MARK to
 */
void protect_socket_somark(int fd, const std::string &remote, int somark);


/**
 * Function that adds a host route
 * @param tun_intf Name of the tun interface, will be ignore for calculating the host route
 * @param fd Socket for the SO_MARK to set
 * @param remote remote host to connect to (for logging only)
 * @param remove_cmds Return the commands to undo the protect command
 *
 * The remove_cmds is a parameter instead of a return value since the move only
 * semantics of ActionList prevent returning it without wrapping it in a smartpointer
 */
void protect_socket_hostroute(const std::string &tun_intf,
                              const std::string &remote,
                              bool ipv6,
                              pid_t pid);

/**
 * Remove all protected sockets that belong to a certain pid
 * @param pid the pid for which the socket protection to remove for
 */
void cleanup_protected_sockets(pid_t pid, NetCfgSignals::Ptr signals);


// Workaround to avoid circular dependencies
CoreTunbuilder *getCoreBuilderInstance();

} // namespace openvpn
