#include <test/jtx/Account.h>
#include <test/jtx/Env.h>
#include <test/jtx/amount.h>
#include <test/jtx/ter.h>

#include <xrpl/beast/unit_test/suite.h>
#include <xrpl/protocol/Feature.h>
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
    authUpdate(jtx::Account const& payer, std::string const& authID)
    {
        json::Value jv;
        jv[jss::TransactionType] = jss::AuthorizationUpdate;
        jv[jss::Account] = payer.human();
        jv[jss::AuthorizationID] = authID;
        jv[jss::LimitAmount] = "10000000";  // 10 XRP, in drops
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

public:
    void
    run() override
    {
        using namespace jtx;
        FeatureBitset const all{testableAmendments()};
        testDisabled(all - featureDirectDebit);
    }
};

BEAST_DEFINE_TESTSUITE(DirectDebit, app, xrpl);

}  // namespace xrpl::test
