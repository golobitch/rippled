#include <xrpl/tx/transactors/directdebit/AuthorizationDelete.h>

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
AuthorizationDelete::getFlagsMask(PreflightContext const& ctx)
{
    // spec 4.2.3.2: no tf flags defined for this transaction.
    return tfUniversalMask;
}

NotTEC
AuthorizationDelete::preflight(PreflightContext const& ctx)
{
    // TODO(spec 4.2.3): temDISABLED if !ctx.rules.enabled(featureDirectDebit).
    return tesSUCCESS;
}

TER
AuthorizationDelete::preclaim(PreclaimContext const& ctx)
{
    // TODO(spec 4.2.3.4 - tec checks):
    //   tecAUTH_NOT_FOUND  - no Authorization with AuthorizationID
    //   tecNO_PERMISSION   - non-Payer submitter while unexpired / no Expiration
    return tesSUCCESS;
}

TER
AuthorizationDelete::doApply()
{
    // TODO(spec 4.2.3.5 - state changes):
    //   - peek keylet::authorization(AuthorizationID)
    //   - dirRemove from Payer owner dir, adjustOwnerCount(-1), view().erase()
    //   - owner reserve returned to Payer
    return tesSUCCESS;
}

void
AuthorizationDelete::visitInvariantEntry(bool, SLE::const_ref, SLE::const_ref)
{
    // TODO(spec 4.1.1.8): Authorization invariants.
}

bool
AuthorizationDelete::finalizeInvariants(
    STTx const&,
    TER,
    XRPAmount,
    ReadView const&,
    beast::Journal const&)
{
    return true;
}

}  // namespace xrpl
