#include <xrpl/tx/transactors/directdebit/AuthorizationCreate.h>

#include <xrpl/basics/Log.h>
#include <xrpl/ledger/ApplyView.h>
#include <xrpl/ledger/View.h>
#include <xrpl/protocol/AccountID.h>
#include <xrpl/protocol/Feature.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/Protocol.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/TxFlags.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/Transactor.h>

#include <cstdint>

namespace xrpl {

std::uint32_t
AuthorizationCreate::getFlagsMask(PreflightContext const& ctx)
{
    // TODO(spec 4.2.1.2): allow tfSingleUse once it is defined in TxFlags.h,
    return tfUniversalMask;
}

NotTEC
AuthorizationCreate::preflight(PreflightContext const& ctx)
{
    // TODO(spec 4.2.1.4 - tem checks):
    //   temDISABLED  if !ctx.rules.enabled(featureDirectDebit)
    //   temBAD_AMOUNT      - LimitAmount non-positive / malformed
    //   temBAD_EXPIRATION  - Expiration not strictly in the future
    //   temBAD_INTERVAL    - Interval > 0 && Interval < 60
    //   temDST_IS_SRC      - AuthorizedAccount == Account
    //   temMALFORMED       - StartTime && Expiration && Expiration <= StartTime
    if (!ctx.rules.enabled(featureDirectDebit)) {
        return temDISABLED;
    }
    return tesSUCCESS;
}

TER
AuthorizationCreate::preclaim(PreclaimContext const& ctx)
{
    // TODO(spec 4.2.1.4 - tec checks):
    //   tecNO_DST                 - AuthorizedAccount not on ledger
    //   tecINSUFFICIENT_RESERVE   - Payer cannot meet incremented owner reserve
    return tesSUCCESS;
}

TER
AuthorizationCreate::doApply()
{
    // TODO(spec 4.2.1.5 - state changes):
    //   - Build the Authorization SLE via keylet::authorization(accountID_, seq)
    //   - Set fields: AuthorizedAccount, LimitAmount, Interval, StartTime, Expiration
    //   - Initialize DeliveredInPeriod = 0 (currency/issuer of LimitAmount),
    //     LastPullTimestamp = 0
    //   - Set lsfSingleUse iff tfSingleUse present
    //   - dirInsert into Payer owner dir, adjustOwnerCount(+1), view().insert()
    return tesSUCCESS;
}

void
AuthorizationCreate::visitInvariantEntry(bool, SLE::const_ref, SLE::const_ref)
{
    // TODO(spec 4.1.1.8): accumulate Authorization invariants here if scoping
    // them to this transactor rather than a global InvariantCheck.
}

bool
AuthorizationCreate::finalizeInvariants(
    STTx const&,
    TER,
    XRPAmount,
    ReadView const&,
    beast::Journal const&)
{
    return true;
}

}  // namespace xrpl
