//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C)  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C)  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   netcfg-signals.cpp
 *
 * @brief  Implementation of the NetCfgSignals class
 */

#include <signal.h>
#include <sstream>
#include <gdbuspp/connection.hpp>
#include <gdbuspp/signals/group.hpp>

#include "log/dbus-log.hpp"
#include "netcfg-changeevent.hpp"
#include "netcfg-signals.hpp"
#include "netcfg-subscriptions.hpp"



NetCfgSignals::NetCfgSignals(DBus::Connection::Ptr conn,
                             LogGroup lgroup,
                             std::string object_path_,
                             LogWriter *logwr)
    : LogSender(conn,
                lgroup,
                object_path_,
                Constants::GenInterface("netcfg"),
                false,
                logwr),
      object_path(object_path_),
      object_interface(Constants::GenInterface("netcfg"))
{
    auto creds = DBus::Credentials::Query::Create(conn);
    AddTarget(creds->GetUniqueBusName(Constants::GenServiceName("log")));

    RegisterSignal("NetworkChange", NetCfgChangeEvent::SignalDeclaration());

    SetLogLevel(default_log_level);
    GroupCreate(object_path);
}


/**
 * Sends a FATAL log messages and kills itself
 *
 * @param Log message to send to the log subscribers
 */
void NetCfgSignals::LogFATAL(const std::string &msg)
{
    Log(Events::Log(log_group, LogCategory::FATAL, msg));
    kill(getpid(), SIGTERM);
}


void NetCfgSignals::Debug(const std::string &msg, bool duplicate_check)
{
    try
    {
        Log(Events::Log(log_group, LogCategory::DEBUG, msg));
    }
    catch (const DBus::Signals::Exception &excp)
    {
        std::cerr << "NetCfgSignals::Debug EXCEPTION: " << excp.what()
                  << std::endl
                  << "Message: " << msg << std::endl;
    }
}


void NetCfgSignals::DebugDevice(const std::string &dev, const std::string &msg)
{
    std::stringstream m;
    m << "[" << dev << "] " << msg;
    Debug(m.str());
}


void NetCfgSignals::AddSubscriptionList(NetCfgSubscriptions::Ptr subs)
{
    subscriptions = subs;
}


void NetCfgSignals::NetworkChange(const NetCfgChangeEvent &ev)
{
    try
    {
        GVariant *e = ev.GetGVariant();
        if (subscriptions)
        {
            auto subscr_list = subscriptions->GetSubscribersList(ev);
            if (subscr_list.size() < 1)
            {
                // If there are no subscribers, we just bail out quickly
                return;
            }
            GroupAddTargetList(object_path, subscr_list);
            GroupSendGVariant(object_path, "NetworkChange", e);
            GroupClearTargets(object_path);
        }
        else
        {
            // If no subscription manager is configured, we switch
            // to broadcasting NetworkChange signals.
            SendGVariant("NetworkChange", e);
        }
    }
    catch (const DBus::Signals::Exception &excp)
    {
        std::cerr << "NetCfgSignals::NetworkChange EXCEPTION: " << excp.what()
                  << std::endl
                  << "Event: " << ev << std::endl;
    }
}
