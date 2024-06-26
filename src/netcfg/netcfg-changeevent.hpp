//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C) 2018 - 2023  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C) 2018 - 2023  David Sommerseth <davids@openvpn.net>
//  Copyright (C) 2019 - 2023  Lev Stipakov <lev@openvpn.net>
//

/**
 * @file   netcfg-changeevent.hpp
 *
 * @brief  Defines constants and provides structs/object capable of handling
 *         network change events from net.openvpn.v3.netcfg
 */

#pragma once

#include <vector>
#include <sstream>
#include <glib.h>
#include <gdbuspp/glib2/utils.hpp>
#include <gdbuspp/signals/group.hpp>

#include "netcfg-exception.hpp"
#include "netcfg-changetype.hpp"


struct NetCfgChangeEvent
{
    NetCfgChangeEvent(const NetCfgChangeType &t,
                      const std::string &dev,
                      const NetCfgChangeDetails &d) noexcept;
    NetCfgChangeEvent(GVariant *params);
    NetCfgChangeEvent() noexcept;

    void reset() noexcept;
    bool empty() const noexcept;


    static const DBus::Signals::SignalArgList SignalDeclaration() noexcept
    {
        return {{"type", glib2::DataType::DBus<NetCfgChangeType>()},
                {"device", glib2::DataType::DBus<std::string>()},
                {"details", "a{ss}"}};
    }


    static const std::string TypeStr(const NetCfgChangeType &type,
                                     bool tech_form = false) noexcept
    {
        switch (type)
        {
        case NetCfgChangeType::UNSET:
            return "[UNSET]";
        case NetCfgChangeType::DEVICE_ADDED:
            return (tech_form ? "DEVICE_ADDED" : "Device Added");
        case NetCfgChangeType::DEVICE_REMOVED:
            return (tech_form ? "DEVICE_REMOVED" : "Device Removed");
        case NetCfgChangeType::IPADDR_ADDED:
            return (tech_form ? "IPADDR_ADDED" : "IP Address Added");
        case NetCfgChangeType::IPADDR_REMOVED:
            return (tech_form ? "IPADDR_REMOVED" : "IP Address Removed");
        case NetCfgChangeType::ROUTE_ADDED:
            return (tech_form ? "ROUTE_ADDED" : "Route Added");
        case NetCfgChangeType::ROUTE_REMOVED:
            return (tech_form ? "ROUTE_REMOVED" : "Route Removed");
        case NetCfgChangeType::ROUTE_EXCLUDED:
            return (tech_form ? "ROUTE_EXCLUDED" : "Route Excluded");
        case NetCfgChangeType::DNS_SERVER_ADDED:
            return (tech_form ? "DNS_SERVER_ADDED" : "DNS Server Added");
        case NetCfgChangeType::DNS_SERVER_REMOVED:
            return (tech_form ? "DNS_SERVER_REMOVED" : "DNS Server Removed");
        case NetCfgChangeType::DNS_SEARCH_ADDED:
            return (tech_form ? "DNS_SEARCH_ADDED" : "DNS Search domain Added");
        case NetCfgChangeType::DNS_SEARCH_REMOVED:
            return (tech_form ? "DNS_SEARCH_REMOVED" : "DNS Search domain Removed");
        default:
            return "[UNKNOWN: " + std::to_string((uint8_t)type) + "]";
        }
    }


    static const std::vector<std::string> FilterMaskList(const uint32_t mask,
                                                         bool tech_form = false)
    {
        std::vector<std::string> ret;

        for (uint32_t i = 0; i < 16; ++i)
        {
            uint32_t flag = 1 << i;
            NetCfgChangeType t = (NetCfgChangeType)(flag);
            if (flag & mask)
            {
                ret.push_back(TypeStr(t, tech_form));
            }
        }

        return ret;
    }


    static const std::string FilterMaskStr(const uint16_t mask,
                                           bool tech_form = false,
                                           const std::string &separator = ", ")
    {
        std::stringstream buf;
        bool first_done = false;
        for (const auto &t : FilterMaskList(mask, tech_form))
        {
            buf << (first_done ? separator : "") << t;
            first_done = true;
        }
        std::string ret(buf.str());
        return ret;
    }


    GVariant *GetGVariant() const;


    /**
     *  Makes it possible to write NetCfgStateEvent in a readable format
     *  via iostreams, such as 'std::cout << state', where state is a
     *  NetCfgStateEvent object.
     *
     * @param os  std::ostream where to write the data
     * @param ev  NetCfgStateEvent to write to the stream
     *
     * @return  Returns the provided std::ostream together with the
     *          decoded NetCfgStateEvent information
     */
    friend std::ostream &operator<<(std::ostream &os, const NetCfgChangeEvent &s)
    {
        if (s.empty())
        {
            return os << "(Empty Network Change Event)";
        }

        std::stringstream detstr;
        bool beginning = true;
        for (const auto &kv : s.details)
        {
            detstr << (beginning ? ": " : ", ")
                   << kv.first << "='" << kv.second << "'";
            beginning = false;
        }
        return os << "Device " << s.device
                  << " - " << TypeStr(s.type)
                  << detstr.str();
    }

    bool operator==(const NetCfgChangeEvent &compare) const;
    bool operator!=(const NetCfgChangeEvent &compare) const;


    NetCfgChangeType type;
    std::string device;
    NetCfgChangeDetails details;
};
