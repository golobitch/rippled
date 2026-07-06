#include <test/jtx/Account.h>
#include <test/jtx/Env.h>
#include <test/jtx/amount.h>
#include <test/jtx/ter.h>

#include <xrpl/beast/unit_test/suite.h>
#include <xrpl/protocol/Feature.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/jss.h>

#include <string>

namespace xrpl::test {

class DirectDebit_test : public beast::unit_test::Suite
{
    static json::Value
    authCreate(jtx::Account const& payer, jtx::Account const& payee)
    {
        json::Value jv;
        jv[jss::TransactionType] = jss::AuthorizationCreate;
        jv[jss::Account] = payer.human();
        jv[jss::AuthorizedAccount] = payee.human();
        jv[jss::LimitAmount] = "10000000";  // 10 XRP, in drops
        return jv;
    }

    static json::Value
    authUpdate(jtx::Account const& payer, std::string const& authID, bool includeLimitAmount = true)
    {
        json::Value jv;
        jv[jss::TransactionType] = jss::AuthorizationUpdate;
        jv[jss::Account] = payer.human();
        jv[jss::AuthorizationID] = authID;
        if (includeLimitAmount) {
            jv[jss::LimitAmount] = "10000000";  // 10 XRP, in drops
        }
        return jv;
    }

    static json::Value
    authDelete(jtx::Account const& payer, std::string const& authID)
    {
        json::Value jv;
        jv[jss::TransactionType] = jss::AuthorizationDelete;
        jv[jss::Account] = payer.human();
        jv[jss::AuthorizationID] = authID;
        return jv;
    }

    void
    testDisabled(FeatureBitset features)
    {
        testcase("Amendment disabled");
        using namespace jtx;

        Env env{*this, features};
        Account const payer{"payer"};
        Account const payee{"payee"};
        env.fund(XRP(5000), payer, payee);
        env.close();

        // value does not mater here because transaction is rejected before
        // any lookup is done
        std::string const authID(64, 'A');

        // all direct debit transactions must be rejected with temDISABLED 
        // when feature is not enabled
        // this is enforced by Transactor::invokePreflight
        env(authCreate(payer, payee), Ter(temDISABLED));
        env(authUpdate(payer, authID), Ter(temDISABLED));
        env(authDelete(payer, authID), Ter(temDISABLED));

        // TODO: when AuthorizationID is added to payment, add test here as well

        env.close();
    }

    void
    testCreatePreflight(FeatureBitset features)
    {
        testcase("AuthorizationCreate preflight");
        using namespace jtx;

        Env env{*this, features};
        Account const payer{"payer"};
        Account const payee{"payee"};
        env.fund(XRP(5000), payer, payee);
        env.close();

        env(authCreate(payer, payee));
        env.close();

        // temBAD_AMOUNT: LimitAmount is non-positive.
        {
            auto jv = authCreate(payer, payee);
            jv[jss::LimitAmount] = "0";
            env(jv, Ter(temBAD_AMOUNT));
        }

        // temBAD_INTERVAL: 0 < Interval < 60.
        {
            auto jv = authCreate(payer, payee);
            jv[sfInterval.jsonName] = 30;
            env(jv, Ter(temBAD_INTERVAL));
        }

        // Interval == 0 (disabled) and Interval >= 60 are both fine.
        {
            auto jv = authCreate(payer, payee);
            jv[sfInterval.jsonName] = 60;
            env(jv);
            env.close();
        }

        // temDST_IS_SRC: AuthorizedAccount == Account.
        env(authCreate(payer, payer), Ter(temDST_IS_SRC));

        // temBAD_EXPIRATION: Expiration == 0.
        {
            auto jv = authCreate(payer, payee);
            jv[sfExpiration.jsonName] = 0;
            env(jv, Ter(temBAD_EXPIRATION));
        }

        // temMALFORMED: StartTime and Expiration both present with
        // Expiration <= StartTime.
        {
            auto jv = authCreate(payer, payee);
            jv[sfStartTime.jsonName] = 200;
            jv[sfExpiration.jsonName] = 100;
            env(jv, Ter(temMALFORMED));
        }

        // temMALFORMED: Expiration == StartTime (boundary).
        {
            auto jv = authCreate(payer, payee);
            jv[sfStartTime.jsonName] = 100;
            jv[sfExpiration.jsonName] = 100;
            env(jv, Ter(temMALFORMED));
        }

        env.close();
    }

    void
    testUpdatePreflight(FeatureBitset features) {
        testcase("AuthorizationUpdate preflight");
        using namespace jtx;

        Env env{*this, features};
        Account const payer{"payer"};
        env.fund(XRP(5000), payer);
        env.close();

        std::string const authID(64, 'A');
        env(authUpdate(payer, authID));
        env.close();

        // temBAD_AMOUNT - LimitAmount is non positive
        {
            auto jv = authUpdate(payer, authID);
            jv[jss::LimitAmount] = "0";
            env(jv, Ter(temBAD_AMOUNT));
        }

        // temBAD_INTERVAL - 0 < interval < 60
        {
            auto jv = authUpdate(payer, authID);
            jv[jss::Interval] = 30;
            env(jv, Ter(temBAD_INTERVAL));
        }

        // temBAD_EXPIRATION - Expiration = 0
        {
            auto jv = authUpdate(payer, authID);
            jv[sfExpiration.jsonName] = 0;
            env(jv, Ter(temBAD_EXPIRATION));
        }

        // temMALFORMED - No updatable fields present
        {
            auto jv = authUpdate(payer, authID, false);
            env(jv, Ter(temMALFORMED));
        }

        // temMALFORMED - expiration before start time
        {
            auto jv = authUpdate(payer, authID);
            jv[sfExpiration.jsonName] = 1;
            jv[sfStartTime.jsonName] = 2;
            env(jv, Ter(temMALFORMED));
        }

        // can update other field and not pass limitamount
        {
            auto jv = authUpdate(payer, authID, false);
            jv[sfInterval.jsonName] = 120;
            env(jv);
            env.close();
        }

        // only starttime provided is ok
        {
            auto jv = authUpdate(payer, authID, false);
            jv[sfStartTime.jsonName] = 100;
            env(jv);
            env.close();
        }
    }

public:
    void
    run() override
    {
        using namespace jtx;
        FeatureBitset const all{testableAmendments()};
        testDisabled(all - featureDirectDebit);
        testCreatePreflight(all);
        testUpdatePreflight(all);
    }
};

BEAST_DEFINE_TESTSUITE(DirectDebit, app, xrpl);

}  // namespace xrpl::test
