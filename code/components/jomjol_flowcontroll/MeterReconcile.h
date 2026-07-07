// MeterReconcile - history-anchored reconciliation of a mechanical-counter reading.
//
// PURE, dependency-free C++ (no ESP-IDF, no I/O, no globals, no clock calls) so it can be
// unit-tested on the host. It turns the per-frame "candidate" value produced by the existing
// recognition pipeline into a robust output, using four physical priors of the meter:
//
//   1. Monotonic        - the meter only counts up.
//   2. Slow vs. frames  - real change between two frames is small; a large change is only
//                         legitimate as a *catch-up* after a gap/stall.
//   3. Absolute encoder - when no dial sits on a boundary, the reading itself is the truth.
//   4. Over-determined  - a most-significant analog dial may only carry when the dial below
//                         it wraps (9 -> 0), and a genuine wrap is visible twice: in the
//                         dial-below transition AND in the value's sub-position passing zero.
//
// The reconciler therefore:
//   - accepts a most-significant-dial *carry* only as a single forward step with wrap evidence
//     from one of two independent channels: the dial-below transition observed in a recent frame
//     (a short window, since the wrap and the valid carry may land on different frames), or
//     inference from the value itself (a genuine carry passes the sub-position through zero,
//     while a premature dial flip leaves it unchanged). The gate applies on BOTH ambiguous AND
//     "clean" frames, because a dial can be misread ~half a step off its true position without
//     landing near an integer - so a premature carry can otherwise masquerade as a trustworthy
//     "clean" reading;
//   - for non-carry motion (within the same most-significant-dial step) trusts a clean reading
//     and, on an ambiguous frame, allows forward motion while tolerating only sub-noise backward;
//   - re-anchors across a carry without wrap evidence only as *recovery*: after the value has
//     been held for a sustained stretch on a clean frame with a stable recognition (genuinely
//     stuck, not flickering or a wandering misread);
//   - never accepts a physically impossible single-frame jump (maxJump); holds instead of guessing.
//
// This is an opt-in layer. The digit-chain zero-crossing handling (CheckDigitIncreaseConsistency)
// is left in place; this adds the missing protection on the analog dial chain.

#ifndef METER_RECONCILE_H
#define METER_RECONCILE_H

#include <vector>
#include <cstddef>

namespace meter {

struct ReconcileParams {
    double maxJump;       // hard ceiling on a single-frame change (value units); <= 0 disables the limit
    double msbStep;       // value units represented by one full step of the most-significant analog dial
    double noiseTol;      // small downward tolerance for jitter (value units); should scale with resolution
    int    recoverHolds;  // consecutive holds before a clean, stable carry is accepted as stuck-recovery
    int    wrapWindow;    // frames an observed lower-dial wrap keeps justifying a single-step carry
    float  band;          // a dial within this distance of an integer is "near a boundary".
                          // INVARIANT: keep >= the recognition chain's own ambiguity band
                          // (Analog_error/10 in defines.h), so any frame the chain may have
                          // "corrected" across a boundary is classified ambiguous here and can
                          // never slip through the clean-recovery path.

    // Defaults are conservative; the firmware derives msbStep / noiseTol / maxJump from the
    // meter's decimal scaling each frame (see ClassFlowPostProcessing).
    ReconcileParams()
        : maxJump(-1.0), msbStep(0.0), noiseTol(0.0), recoverHolds(5), wrapWindow(3), band(0.30f) {}
};

struct ReconcileState {
    bool   hasConfirmed;  // false until the first reading is anchored
    double confirmed;     // last trusted value (mirrors PreValue)
    int    held;          // consecutive holds since the last accept (drives stuck-recovery)
    int    wrapArmed;     // frames remaining in which a recently-observed wrap justifies a carry
    double lastCandidate; // previous frame's candidate (stability check for stuck-recovery)
    bool   hasLast;

    ReconcileState() : hasConfirmed(false), confirmed(0.0), held(0), wrapArmed(0),
                       lastCandidate(0.0), hasLast(false) {}
};

enum ReconcileAction { RA_ACCEPT, RA_HOLD };

struct ReconcileResult {
    double          value;       // value to output this frame
    ReconcileAction action;
    bool            reAnchored;   // confirmed was updated from a trusted reading this frame
};

// Per-frame reconciliation.
//   candidate           : value assembled by the existing pipeline this frame.
//   clean               : true if NO dial is within `band` of an integer and no ROI was unreadable.
//   lowerDialWrapped    : true if the dial just below the most-significant analog dial was observed
//                         to wrap (high -> low) since the previous frame (justifies a carry).
ReconcileResult reconcileStep(ReconcileState& st, double candidate,
                              bool clean, bool lowerDialWrapped,
                              const ReconcileParams& p);

// --- helpers (also pure / testable), used by the firmware to derive the booleans above ---

// Is a single analog dial reading within `band` of an integer boundary (incl. the 0/10 seam)?
bool dialNearBoundary(float dialValue, float band);

// Did a dial wrap forward (high -> low, a true 9->0 rollover) between the previous and current frame?
bool dialWrapped(float prevValue, float nowValue, float band);

// Are ALL dials clear of their boundaries (-> the absolute reading is trustworthy)?
bool framesClean(const std::vector<float>& dials, float band);

} // namespace meter

#endif // METER_RECONCILE_H
