//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C)  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C)  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   syslog.cpp
 *
 * @brief  Implementation of SyslogWriter
 */

#include <iostream>
#include <sstream>
#include <string>

#include "../logwriter.hpp"
#include "syslog.hpp"


//
//  SyslogWriter - implementation
//
SyslogWriter::SyslogWriter(const std::string &prgname,
                           const int log_facility)
    : LogWriter()
{
    progname = strdup(prgname.c_str());
    openlog(progname, LOG_NDELAY | LOG_PID, log_facility);
}

SyslogWriter::~SyslogWriter()
{
    closelog();
    free(progname);
}


const std::string SyslogWriter::GetLogWriterInfo() const
{
    return std::string("syslog");
}


bool SyslogWriter::TimestampEnabled()
{
    return true;
}


void SyslogWriter::WriteLogLine(LogTag::Ptr logtag,
                                const std::string &data,
                                const std::string &colour_init,
                                const std::string &colour_reset)
{
    // This is a very simple log implementation.  We do not
    // care about timestamps, as we trust the syslog takes
    // care of that.  We also do not do anything about
    // colours, as that can mess up the log files.

    std::ostringstream logtag_str;
    if (logtag)
    {
        logtag_str << logtag->str(true) << " ";
    }

    if (log_meta && metadata && !metadata->empty())
    {
        std::ostringstream meta_str;
        meta_str << metadata;

        syslog(LOG_INFO, "%s%s", logtag_str.str().c_str(), meta_str.str().c_str());
    }

    syslog(LOG_INFO, "%s%s", logtag_str.str().c_str(), data.c_str());
    if (metadata)
    {
        metadata->clear();
    }
}


void SyslogWriter::WriteLogLine(LogTag::Ptr logtag,
                                const LogGroup grp,
                                const LogCategory ctg,
                                const std::string &data,
                                const std::string &colour_init,
                                const std::string &colour_reset)
{
    // Equally simple to the other Write() method, but here
    // we have access to LogGroup and LogCategory, so we
    // include that information.

    std::ostringstream logtag_str;
    if (logtag)
    {
        logtag_str << logtag->str(true) << " ";
    }

    if (log_meta && metadata && !metadata->empty())
    {
        std::ostringstream meta_str;
        meta_str << metadata;

        syslog(logcatg2syslog(ctg),
               "%s%s",
               logtag_str.str().c_str(),
               meta_str.str().c_str());
    }

    syslog(logcatg2syslog(ctg),
           "%s%s%s",
           logtag_str.str().c_str(),
           LogPrefix(grp, ctg).c_str(),
           data.c_str());

    if (metadata)
    {
        metadata->clear();
    }
}
