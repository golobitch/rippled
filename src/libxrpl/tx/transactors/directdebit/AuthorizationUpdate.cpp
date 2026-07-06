#include <xrpl/tx/transactors/directdebit/AuthorizationUpdate.h>

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
AuthorizationUpdate::getFlagsMask(PreflightContext const& ctx)
{
    // spec 4.2.2.2: no tf flags defined for this transaction.
    return tfUniversalMask;
}

NotTEC
AuthorizationUpdate::preflight(PreflightContext const& ctx)
{
    // TODO(spec 4.2.2.4 - tem checks):
    //   temDISABLED   if !ctx.rules.enabled(featureDirectDebit)
    //   temBAD_AMOUNT      - LimitAmount non-positive / malformed
    //   temBAD_EXPIRATION  - Expiration > 0 && not strictly in the future
    //   temBAD_INTERVAL    - Interval > 0 && Interval < 60
    //   temMALFORMED       - none of LimitAmount/Interval/StartTime/Expiration
    //                        present; or StartTime && Expiration <= StartTime
    if (!ctx.rules.enabled(featureDirectDebit)) {
        return temDISABLED;
    }
    return tesSUCCESS;
}

TER
AuthorizationUpdate::preclaim(PreclaimContext const& ctx)
{
    // TODO(spec 4.2.2.4 - tec checks):
    //   tecAUTH_NOT_FOUND  - no Authorization with AuthorizationID
    //   temBAD_CURRENCY    - LimitAmount currency/issuer mismatch (may be here)
    //   tecNO_PERMISSION   - Account is not the Payer; or StartTime provided
    //                        while LastPullTimestamp != 0; or resulting
    //                        Expiration <= StartTime
    return tesSUCCESS;
}

TER
AuthorizationUpdate::doApply()
{
    // TODO(spec 4.2.2.5 - state changes):
    //   - peek keylet::authorization(AuthorizationID)
    //   - update LimitAmount / Interval / StartTime / Expiration as supplied
    //   - do NOT reset DeliveredInPeriod or LastPullTimestamp
    //   - Flags (incl. lsfSingleUse) are immutable
    return tesSUCCESS;
}

void
AuthorizationUpdate::visitInvariantEntry(bool, SLE::const_ref, SLE::const_ref)
{
    // TODO(spec 4.1.1.8): Authorization invariants.
}

bool
AuthorizationUpdate::finalizeInvariants(
    STTx const&,
    TER,
    XRPAmount,
    ReadView const&,
    beast::Journal const&)
{
    return true;
}

}  // namespace xrpl
