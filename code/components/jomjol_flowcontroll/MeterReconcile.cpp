#include "MeterReconcile.h"
#include <cmath>

namespace meter {

bool dialNearBoundary(float dialValue, float band) {
    // distance to the nearest integer, treating the dial as a 0..10 ring (so 9.9 is near 0).
    float frac = dialValue - std::floor(dialValue);     // 0 .. <1
    float dist = frac < 0.5f ? frac : (1.0f - frac);    // distance to nearest integer in [0,0.5]
    return dist <= band;
}

bool dialWrapped(float prevValue, float nowValue, float band) {
    // Forward 9 -> 0 rollover: the dial was near the TOP of the 0..10 ring on the previous frame
    // and dropped sharply to the bottom. We key on a genuine large high->low drop, NOT merely a
    // high fractional part - so 0.9 -> 0.1 (low-integer jitter) is not a wrap, while 9.5 -> 0.5
    // and 9.8 -> 1.2 (a fast rollover overshooting 1.0) are.
    bool prevHigh = prevValue >= (9.0f - band);
    bool nowLow   = nowValue  <= 3.0f;                   // a real rollover lands near 0 (generous for fast flow)
    return prevHigh && nowLow && ((prevValue - nowValue) > 5.0f);
}

bool framesClean(const std::vector<float>& dials, float band) {
    if (dials.empty()) return false;
    for (size_t i = 0; i < dials.size(); ++i) {
        if (dials[i] < 0.0f) return false;          // negative sentinel == unreadable (N)
        if (dialNearBoundary(dials[i], band)) return false;
    }
    return true;
}

static ReconcileResult accept(ReconcileState& st, double v) {
    st.confirmed = v;
    st.held = 0;
    ReconcileResult r; r.value = v; r.action = RA_ACCEPT; r.reAnchored = true;
    return r;
}

static ReconcileResult hold(ReconcileState& st) {
    st.held++;
    ReconcileResult r; r.value = st.confirmed; r.action = RA_HOLD; r.reAnchored = false;
    return r;
}

// Which most-significant-dial "bucket" does a value fall in? A small relative epsilon absorbs
// floating-point error in (value / msbStep) at bucket boundaries (msbStep may be fractional, e.g.
// 0.1, while values are large), so a value sitting exactly on a boundary is not flickered into the
// wrong bucket by rounding noise.
static long stepBucket(double value, double step) {
    double q = value / step;
    return (long) std::floor(q + (std::fabs(q) + 1.0) * 1e-9);
}

ReconcileResult reconcileStep(ReconcileState& st, double candidate,
                              bool clean, bool lowerDialWrapped,
                              const ReconcileParams& p) {
    // Cold start: no anchor yet -> trust the reading.
    if (!st.hasConfirmed) {
        st.hasConfirmed = true;
        return accept(st, candidate);
    }

    const double delta = candidate - st.confirmed;

    // Stability of the recognition itself: is this frame's candidate (essentially) the same as
    // the previous frame's? Used to bound the stuck-recovery path below.
    const bool stable = st.hasLast && std::fabs(candidate - st.lastCandidate) <= p.noiseTol;
    st.lastCandidate = candidate;
    st.hasLast = true;

    // Arm / age the wrap window. A lower-dial wrap and the valid single-step carry it justifies
    // can land on *different* frames (e.g. the wrap frame is itself a misread), so remember an
    // observed wrap for a few frames rather than only on the exact frame it is seen.
    if (lowerDialWrapped) st.wrapArmed = p.wrapWindow;
    else if (st.wrapArmed > 0) st.wrapArmed--;

    // Physically impossible single-frame jump -> never accept (gross misread).
    if (p.maxJump > 0.0 && std::fabs(delta) > p.maxJump) {
        return hold(st);
    }

    // Does the candidate cross a most-significant-dial step vs. the confirmed value?
    long steps = 0;
    bool subWrapped = false;
    if (p.msbStep > 0.0) {
        steps = stepBucket(candidate, p.msbStep) - stepBucket(st.confirmed, p.msbStep);

        // Wrap INFERENCE from the values themselves: a genuine carry always passes the
        // sub-position (the part below one msb step, i.e. the dials underneath) through zero,
        // while a premature msb-dial flip leaves it unchanged. So "sub-position decreased
        // decisively" is evidence of a real wrap even if the observed dial-below transition was
        // missed (misread wrap frame, gap in frames, or no second dial at all). The margin must
        // exceed the dial-below reading scatter (~band of its step = band/10 of msbStep), so
        // ordinary backward jitter of the dial below cannot fake wrap evidence.
        double subConf = st.confirmed - (double) stepBucket(st.confirmed, p.msbStep) * p.msbStep;
        double subNow  = candidate    - (double) stepBucket(candidate,    p.msbStep) * p.msbStep;
        subWrapped = subNow < (subConf - 0.1 * p.msbStep);
    }

    if (steps != 0) {
        // Carry. Legitimate only as a single forward step justified by a wrap of the dial below,
        // either observed (recently, within the wrap window) or inferred from the sub-position --
        // enforced on clean AND ambiguous frames, so a dial misread ~half a step off (not near an
        // integer, hence "clean") cannot sneak a premature carry through.
        if (steps == 1 && delta > 0.0 && (st.wrapArmed > 0 || subWrapped)) {
            st.wrapArmed = 0;                 // one wrap justifies exactly one carry
            return accept(st, candidate);
        }
        // Recovery: if the value has been held for a sustained stretch it was genuinely stuck
        // (not flickering), so on a clean frame with a stable recognition trust the reading and
        // re-anchor across the carry (heals an anchor left behind by a missed carry).
        //
        // Bounded by recoverMaxSteps: "clean and stable for a while" is NOT evidence that a
        // far-away reading is right - a digit misread one position ahead sits perfectly clean and
        // stable for as long as the meter is idle, and recovering onto it replaces a correct value
        // with one a full digit out. Beyond the cap, keep holding: a frozen value is visible and
        // costs nothing, a poisoned anchor silently corrupts every later total.
        const long absSteps = steps < 0 ? -steps : steps;
        if (clean && stable && st.held >= p.recoverHolds &&
            (p.recoverMaxSteps <= 0 || absSteps <= (long) p.recoverMaxSteps)) {
            return accept(st, candidate);
        }
        return hold(st);
    }

    // No carry (candidate is in the same most-significant-dial bucket as confirmed).
    //
    // Backward motion beyond the noise tolerance. A mechanical counter cannot run backwards, so
    // this is a misread - unless the anchor ITSELF is wrong, and re-anchoring down is then the only
    // way back, because the meter can never climb to meet a too-high anchor. So the move is allowed
    // but must clear the same evidence bar as the stuck-recovery above: genuinely stuck (held for a
    // sustained stretch), not merely flickering.
    //
    // A clean frame is deliberately NOT sufficient on its own. "Clean" only means no dial sits near
    // an integer, and a dial misread by nearly a whole step sits comfortably clean - which is
    // exactly how a backward jump of almost a full dial step gets in. Such a jump stays inside one
    // most-significant-dial bucket, so the carry gate never sees it either.
    if (delta < -p.noiseTol) {
        if (clean && stable && st.held >= p.recoverHolds) {
            return accept(st, candidate);   // genuinely stuck too high -> re-anchor downwards
        }
        return hold(st);
    }

    // Forward, or backward within the noise tolerance: trustworthy on a clean frame, and ordinary
    // forward motion is fine on an ambiguous one too.
    return accept(st, candidate);
}

} // namespace meter
