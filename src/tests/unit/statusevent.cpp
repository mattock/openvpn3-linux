//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C)  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C)  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   statusevent.cpp
 *
 * @brief  Unit test for struct StatusEvent
 */

#include <iostream>
#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "events/status.hpp"


namespace unittest {

std::string test_empty(const Events::Status &ev, const bool expect)
{
    bool r = ev.empty();
    if (expect != r)
    {
        return std::string("test_empty():  ")
               + "ev.empty() = " + (r ? "true" : "false")
               + " [expected: " + (expect ? "true" : "false") + "]";
    }

    r = (StatusMajor::UNSET == ev.major
         && StatusMinor::UNSET == ev.minor
         && ev.message.empty());
    if (expect != r)
    {
        return std::string("test_empty() - Member check:  ")
               + "(" + std::to_string((unsigned)ev.major) + ", "
               + std::to_string((unsigned)ev.minor) + "', "
               + "message.size=" + std::to_string(ev.message.size()) + ") ..."
               + " is " + (r ? "EMPTY" : "NON-EMPTY")
               + " [expected: " + (expect ? "EMPTY" : "NON-EMPTY") + "]";
    }
    return "";
};


TEST(StatusEvent, init_empty)
{
    Events::Status empty;
    std::string res = test_empty(empty, true);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, init_with_values)
{
    Events::Status populated1(StatusMajor::PROCESS, StatusMinor::PROC_STARTED);
    std::string res = test_empty(populated1, false);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, reset)
{
    Events::Status populated1(StatusMajor::PROCESS, StatusMinor::PROC_STARTED);
    populated1.reset();
    std::string res = test_empty(populated1, true);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, init_with_values_2)
{
    Events::Status populated2(StatusMajor::PROCESS, StatusMinor::PROC_STOPPED, "Just testing");
    std::string res = test_empty(populated2, false);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, reset_2)
{
    Events::Status populated2(StatusMajor::PROCESS, StatusMinor::PROC_STOPPED, "Just testing");
    populated2.reset();
    std::string res = test_empty(populated2, true);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, parse_gvariant_invalid_data)
{
    GVariant *data = nullptr;
    data = g_variant_new("(uuss)",
                         (guint)StatusMajor::CONFIG,
                         (guint)StatusMinor::CFG_OK,
                         "Test status",
                         "Invalid data");

    ASSERT_THROW(Events::Status parsed(data),
                 DBus::Exception);
    if (nullptr != data)
    {
        g_variant_unref(data);
    }
}


TEST(StatusEvent, parse_gvariant_valid_dict)
{
    GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(b, "{sv}", "major", g_variant_new_uint32((guint)StatusMajor::CONFIG));
    g_variant_builder_add(b, "{sv}", "minor", g_variant_new_uint32((guint)StatusMinor::CFG_OK));
    g_variant_builder_add(b, "{sv}", "status_message", g_variant_new_string("Test status"));
    GVariant *data = g_variant_builder_end(b);
    g_variant_builder_unref(b);

    Events::Status parsed(data);
    ASSERT_EQ(parsed.major, StatusMajor::CONFIG);
    ASSERT_EQ(parsed.minor, StatusMinor::CFG_OK);
    ASSERT_EQ(parsed.message, "Test status");

    g_variant_unref(data);
}

TEST(StatusEvent, parse_gvariant_valid_tuple)
{
    GVariant *data = g_variant_new("(uus)",
                                   (guint)StatusMajor::CONFIG,
                                   (guint)StatusMinor::CFG_REQUIRE_USER,
                                   "Parse testing again");
    Events::Status parsed(data);
    ASSERT_EQ(parsed.major, StatusMajor::CONFIG);
    ASSERT_EQ(parsed.minor, StatusMinor::CFG_REQUIRE_USER);
    ASSERT_EQ(parsed.message, "Parse testing again");

    g_variant_unref(data);
}


TEST(StatusEvent, GetGVariantTuple)
{
    Events::Status reverse(StatusMajor::CONNECTION, StatusMinor::CONN_INIT, "Yet another test");
    GVariant *revparse = reverse.GetGVariantTuple();
    guint maj = 0;
    guint min = 0;
    gchar *msg_c = nullptr;
    g_variant_get(revparse, "(uus)", &maj, &min, &msg_c);
    std::string msg(msg_c);
    g_free(msg_c);

    ASSERT_EQ((StatusMajor)maj, reverse.major);
    ASSERT_EQ((StatusMinor)min, reverse.minor);
    ASSERT_EQ(msg, reverse.message);

    g_variant_unref(revparse);
}


TEST(StatusEvent, GetVariantDict)
{
    Events::Status dicttest(StatusMajor::SESSION, StatusMinor::SESS_NEW, "Moar testing is needed");
    GVariant *revparse = dicttest.GetGVariantDict();

    // Reuse the parser in StatusEvent.  As that has already passed the
    // test, expect this to work too.
    Events::Status cmp(revparse);
    g_variant_unref(revparse);

    ASSERT_EQ(cmp.major, dicttest.major);
    ASSERT_EQ(cmp.minor, dicttest.minor);
    ASSERT_EQ(cmp.message, dicttest.message);
}


std::string test_compare(const Events::Status &lhs, const Events::Status &rhs, const bool expect)
{
    bool r = (lhs.major == rhs.major
              && lhs.minor == rhs.minor
              && lhs.message == rhs.message);
    if (r != expect)
    {
        std::stringstream err;
        err << "StatusEvent compare check FAIL: "
            << "{" << lhs << "} == {" << rhs << "} returned "
            << (r ? "true" : "false")
            << " - expected: "
            << (expect ? "true" : "false");
        return err.str();
    }

    r = (lhs.major != rhs.major
         || lhs.minor != rhs.minor
         || lhs.message != rhs.message);
    if (r == expect)
    {
        std::stringstream err;
        err << "Negative StatusEvent compare check FAIL: "
            << "{" << lhs << "} == {" << rhs << "} returned "
            << (r ? "true" : "false")
            << " - expected: "
            << (expect ? "true" : "false");
        return err.str();
    }
    return "";
}


TEST(StatusEvent, compare_eq_1)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::SESS_AUTH_CHALLENGE);
    Events::Status chk(StatusMajor::SESSION, StatusMinor::SESS_AUTH_CHALLENGE);
    std::string res = test_compare(ev, chk, true);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, compare_eq_2)
{
    Events::Status ev(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT, "var1");
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT, "var1");
    std::string res = test_compare(ev, chk, true);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, operator_eq_1)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::SESS_AUTH_CHALLENGE);
    Events::Status chk(StatusMajor::SESSION, StatusMinor::SESS_AUTH_CHALLENGE);
    ASSERT_TRUE(ev == chk);
}


TEST(StatusEvent, operator_eq_2)
{
    Events::Status ev(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT, "var1");
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT, "var1");
    ASSERT_TRUE(ev == chk);
}


TEST(StatusEvent, compare_neq_1)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::CFG_REQUIRE_USER);
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT);
    std::string res = test_compare(ev, chk, false);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, compare_neq_2)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::PKCS11_DECRYPT, "var1");
    Events::Status chk(StatusMajor::SESSION, StatusMinor::SESS_BACKEND_COMPLETED, "var1");
    std::string res = test_compare(ev, chk, false);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, compare_neq_3)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::PKCS11_DECRYPT, "var1");
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT);
    std::string res = test_compare(ev, chk, false);
    ASSERT_TRUE(res.empty()) << res;
}


TEST(StatusEvent, operator_neq_1)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::CFG_REQUIRE_USER);
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT);
    ASSERT_TRUE(ev != chk);
}


TEST(StatusEvent, operator_neq_2)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::PKCS11_DECRYPT, "var1");
    Events::Status chk(StatusMajor::SESSION, StatusMinor::SESS_BACKEND_COMPLETED, "var1");
    ASSERT_TRUE(ev != chk);
}


TEST(StatusEvent, operator_neq_3)
{
    Events::Status ev(StatusMajor::SESSION, StatusMinor::PKCS11_DECRYPT, "var1");
    Events::Status chk(StatusMajor::PROCESS, StatusMinor::PKCS11_ENCRYPT);
    ASSERT_TRUE(ev != chk);
}


TEST(StatusEvent, stringstream)
{
    Events::Status status(StatusMajor::CONFIG, StatusMinor::CONN_CONNECTING, "In progress");
#ifdef DEBUG_CORE_EVENTS
    status.show_numeric_status = false; // DEBUG_CORE_EVENTS enables this by default
#endif
    std::stringstream chk;
    chk << status;
    std::string expect("Configuration, Client connecting: In progress");
    ASSERT_EQ(chk.str(), expect);

    std::stringstream chk1;
    status.show_numeric_status = true;
    chk1 << status;
    std::string expect1("[1,6] Configuration, Client connecting: In progress");
    ASSERT_EQ(chk1.str(), expect1);


    Events::Status status2(StatusMajor::SESSION,
                           StatusMinor::SESS_BACKEND_COMPLETED);
    std::stringstream chk2;
    status2.show_numeric_status = true;
    chk2 << status2;
    std::string expect2("[3,18] Session, Backend Session Object completed");
    ASSERT_EQ(chk2.str(), expect2);

    std::stringstream chk3;
    status2.show_numeric_status = false;
    chk3 << status2;
    std::string expect3("Session, Backend Session Object completed");
    ASSERT_EQ(chk3.str(), expect3);
}


} // namespace unittest
