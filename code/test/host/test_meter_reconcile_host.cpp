// Host-only unit test for the pure reconciliation core (no ESP-IDF needed).
// Build & run (from this directory):
//   g++ -std=c++11 -I../../components/jomjol_flowcontroll test_meter_reconcile_host.cpp ../../components/jomjol_flowcontroll/MeterReconcile.cpp -o reconcile_test
//   ./reconcile_test
//
// The scenarios exercise the failure modes the reconciler is designed to handle. They use
// generic numbers with a most-significant-dial step (msbStep) of 100, so a change of 100
// represents one full step of the most-significant analog dial.

#include "MeterReconcile.h"
#include <cstdio>
#include <cmath>
#include <string>

using namespace meter;

static int g_pass = 0, g_fail = 0;

static void check(const char* what, double got, double want) {
    bool ok = std::fabs(got - want) < 0.05;
    printf("  [%s] %-52s got=%.2f want=%.2f\n", ok ? "PASS" : "FAIL", what, got, want);
    if (ok) g_pass++; else g_fail++;
}
static void checkAction(const char* what, ReconcileAction got, ReconcileAction want) {
    bool ok = got == want;
    printf("  [%s] %-52s action got=%d want=%d\n", ok ? "PASS" : "FAIL", what, (int)got, (int)want);
    if (ok) g_pass++; else g_fail++;
}

static ReconcileParams stdParams() {
    ReconcileParams p;
    p.msbStep      = 100.0;   // one step of the most-significant analog dial
    p.maxJump      = 5000.0;  // hard ceiling on a single-frame change
    p.noiseTol     = 2.0;
    p.recoverHolds = 5;       // clean carry accepted as recovery only after this many holds
    p.band         = 0.30f;
    return p;
}

static ReconcileState anchoredAt(double v, const ReconcileParams& p) {
    ReconcileState st;
    reconcileStep(st, v, /*clean*/true, /*wrap*/false, p);
    return st;
}

int main() {
    ReconcileParams p = stdParams();

    printf("A: cold start, then normal clean forward motion is tracked\n");
    {
        ReconcileState st;
        ReconcileResult r = reconcileStep(st, 500000.0, true, false, p);
        check("cold start accepts", r.value, 500000.0);
        checkAction("cold start -> ACCEPT", r.action, RA_ACCEPT);
        r = reconcileStep(st, 500005.0, true, false, p);
        check("small clean forward accepted", r.value, 500005.0);
        r = reconcileStep(st, 500011.0, true, false, p);
        check("continues to track", r.value, 500011.0);
    }

    printf("B: a premature carry on an AMBIGUOUS frame (no wrap) is held\n");
    {
        ReconcileState st = anchoredAt(500200.0, p);
        for (int i = 0; i < 4; ++i) {
            ReconcileResult r = reconcileStep(st, 500300.0, /*clean*/false, /*wrap*/false, p);
            if (i == 0) { check("held at last good value", r.value, 500200.0);
                          checkAction("-> HOLD", r.action, RA_HOLD); }
        }
    }

    printf("C: a premature carry on a CLEAN frame (no wrap) is ALSO held  [key fix]\n");
    {
        // A dial misread ~a third of a step high sits at the band edge, so the frame can look
        // 'clean' - but a carry without an observed wrap must still be rejected, not trusted.
        ReconcileState st = anchoredAt(500200.0, p);
        ReconcileResult r = reconcileStep(st, 500300.0, /*clean*/true, /*wrap*/false, p);
        check("clean premature carry held (not accepted)", r.value, 500200.0);
        checkAction("clean premature carry -> HOLD", r.action, RA_HOLD);
    }

    printf("D: a legitimate carry (observed wrap, single forward step) is accepted\n");
    {
        ReconcileState st = anchoredAt(500200.0, p);
        ReconcileResult r = reconcileStep(st, 500305.0, /*clean*/false, /*wrap*/true, p);
        check("carry with observed wrap accepted", r.value, 500305.0);
        checkAction("legit carry -> ACCEPT", r.action, RA_ACCEPT);
    }

    printf("E: a backward carry is held even with an observed wrap\n");
    {
        ReconcileState st = anchoredAt(500300.0, p);
        ReconcileResult r = reconcileStep(st, 500250.0, /*clean*/false, /*wrap*/true, p);
        check("backward carry held", r.value, 500300.0);
    }

    printf("F: a multi-step carry is held even with a single observed wrap\n");
    {
        ReconcileState st = anchoredAt(500200.0, p);
        ReconcileResult r = reconcileStep(st, 500450.0, /*clean*/false, /*wrap*/true, p); // 2 steps
        check("two-step carry held (one wrap != two steps)", r.value, 500200.0);
    }

    printf("G: a physically impossible jump is rejected (and cannot be confirmed)\n");
    {
        ReconcileState st = anchoredAt(500000.0, p);
        ReconcileResult r = reconcileStep(st, 600000.0, /*clean*/false, false, p); // +100000 > maxJump
        check("gross misread held (ambiguous)", r.value, 500000.0);
        r = reconcileStep(st, 600000.0, /*clean*/true, false, p);
        check("gross misread held (clean)", r.value, 500000.0);
        for (int i = 0; i < 8; ++i) reconcileStep(st, 600000.0, true, false, p); // beyond recoverHolds
        r = reconcileStep(st, 600000.0, true, false, p);
        check("still held (> maxJump beats recovery)", r.value, 500000.0);
        r = reconcileStep(st, 500050.0, true, false, p);
        check("a valid reading is still accepted afterwards", r.value, 500050.0);
    }

    printf("H: a clean carry is accepted only after a SUSTAINED hold (stuck-recovery)\n");
    {
        ReconcileState st = anchoredAt(500200.0, p);
        ReconcileResult r;
        for (int i = 0; i < p.recoverHolds; ++i) {                 // 5 ambiguous carry holds
            r = reconcileStep(st, 500300.0, /*clean*/false, /*wrap*/false, p);
        }
        check("still held through the stall", r.value, 500200.0);
        r = reconcileStep(st, 500300.0, /*clean*/true, /*wrap*/false, p);  // now clean -> recover
        check("clean frame re-anchors after sustained holds", r.value, 500300.0);
        checkAction("recovery -> ACCEPT", r.action, RA_ACCEPT);
    }

    printf("I: backward jitter beyond noise on an ambiguous (no-carry) frame is rejected\n");
    {
        ReconcileState st = anchoredAt(500050.0, p);
        ReconcileResult r = reconcileStep(st, 500000.0, /*clean*/false, false, p); // -50, same bucket
        check("ambiguous backward held", r.value, 500050.0);
    }

    printf("J: tiny sub-noise jitter on an ambiguous frame is accepted\n");
    {
        ReconcileState st = anchoredAt(500050.0, p);
        ReconcileResult r = reconcileStep(st, 500049.0, /*clean*/false, false, p); // -1, within noiseTol
        check("ambiguous sub-noise jitter accepted", r.value, 500049.0);
    }

    printf("K: forward sub-step motion on an ambiguous frame is accepted\n");
    {
        ReconcileState st = anchoredAt(500010.0, p);
        ReconcileResult r = reconcileStep(st, 500030.0, /*clean*/false, false, p); // +20, no carry
        check("ambiguous forward (no carry) accepted", r.value, 500030.0);
    }

    printf("L: a clean no-carry change is accepted immediately\n");
    {
        ReconcileState st = anchoredAt(500030.0, p);
        ReconcileResult r = reconcileStep(st, 500045.0, /*clean*/true, false, p);
        check("clean no-carry accepted", r.value, 500045.0);
    }

    printf("M: dial helper functions\n");
    {
        check("wrap 9.5 -> 0.5",                       dialWrapped(9.5f, 0.5f, 0.30f) ? 1 : 0, 1);
        check("wrap 9.8 -> 1.2 (overshoots 1.0)",      dialWrapped(9.8f, 1.2f, 0.30f) ? 1 : 0, 1);
        check("wrap 9.0 -> 0.1",                       dialWrapped(9.0f, 0.1f, 0.30f) ? 1 : 0, 1);
        check("NOT wrap 0.9 -> 0.1 (low-int jitter)",  dialWrapped(0.9f, 0.1f, 0.30f) ? 1 : 0, 0);
        check("NOT wrap 7.8 -> 0.1 (mid-dial)",        dialWrapped(7.8f, 0.1f, 0.30f) ? 1 : 0, 0);
        check("near boundary 3.05",                    dialNearBoundary(3.05f, 0.30f) ? 1 : 0, 1);
        check("near boundary 9.8 (0/10 seam)",         dialNearBoundary(9.8f,  0.30f) ? 1 : 0, 1);
        check("NOT near boundary 3.5",                 dialNearBoundary(3.5f,  0.30f) ? 1 : 0, 0);
        std::vector<float> clear_; clear_.push_back(3.5f); clear_.push_back(5.5f);
        check("framesClean: all clear",                framesClean(clear_, 0.30f) ? 1 : 0, 1);
        std::vector<float> near_; near_.push_back(3.5f); near_.push_back(9.95f);
        check("framesClean: one near boundary",        framesClean(near_, 0.30f) ? 1 : 0, 0);
        std::vector<float> nan_; nan_.push_back(3.5f); nan_.push_back(-1.0f);
        check("framesClean: unreadable (N)",           framesClean(nan_, 0.30f) ? 1 : 0, 0);
        std::vector<float> empty_;
        check("framesClean: empty",                    framesClean(empty_, 0.30f) ? 1 : 0, 0);
    }

    printf("N: the carry gate is scale-invariant (small msbStep, FP-safe)\n");
    {
        ReconcileParams ps; ps.msbStep = 0.1; ps.maxJump = 50.0; ps.noiseTol = 0.02;
        ps.recoverHolds = 5; ps.band = 0.30f;
        ReconcileState st = anchoredAt(2000.0, ps);
        ReconcileResult r = reconcileStep(st, 1999.5, /*clean*/false, false, ps);   // backward carry
        check("small-scale backward held", r.value, 2000.0);
        r = reconcileStep(st, 2000.05, /*clean*/false, false, ps);                  // forward sub-step, no FP carry
        check("small-scale forward sub-step accepted", r.value, 2000.05);
    }

    printf("O: the reAnchored flag reflects whether confirmed was updated\n");
    {
        ReconcileState st = anchoredAt(500000.0, p);
        ReconcileResult acc = reconcileStep(st, 500005.0, true, false, p);
        check("accept sets reAnchored", acc.reAnchored ? 1 : 0, 1);
        ReconcileResult hld = reconcileStep(st, 500300.0, /*clean*/true, /*wrap*/false, p);
        check("hold clears reAnchored", hld.reAnchored ? 1 : 0, 0);
    }

    printf("P: msbStep == 0 disables the carry gate (struct default if not derived)\n");
    {
        ReconcileParams pt = stdParams(); pt.msbStep = 0.0;
        ReconcileState st = anchoredAt(500200.0, pt);
        ReconcileResult r = reconcileStep(st, 500300.0, /*clean*/false, /*wrap*/false, pt);
        check("no carry gate -> ambiguous forward accepted", r.value, 500300.0);
    }

    printf("Q: maxJump <= 0 removes the ceiling but the carry gate still applies\n");
    {
        ReconcileParams pq = stdParams(); pq.maxJump = -1.0;
        ReconcileState st = anchoredAt(500000.0, pq);
        ReconcileResult r = reconcileStep(st, 900000.0, /*clean*/true, /*wrap*/false, pq); // huge carry
        check("huge carry still held (gate applies)", r.value, 500000.0);
        for (int i = 0; i < pq.recoverHolds; ++i) reconcileStep(st, 900000.0, false, false, pq);
        r = reconcileStep(st, 900000.0, /*clean*/true, false, pq);   // sustained hold -> recover
        check("recovery accepts once stuck (no ceiling)", r.value, 900000.0);
    }

    printf("R: a wrap seen one frame before the valid carry still accepts (sticky wrap)\n");
    {
        // Mirrors a cascade rollover: the dial-below wraps on a frame whose assembled value is a
        // misread multi-step jump (held), then the valid single-step carry arrives next frame.
        ReconcileState st = anchoredAt(500900.0, p);
        ReconcileResult r = reconcileStep(st, 505900.0, /*clean*/false, /*wrap*/true, p); // +50 steps: misread
        check("misread multi-step held despite wrap", r.value, 500900.0);
        r = reconcileStep(st, 501005.0, /*clean*/false, /*wrap*/false, p);                // valid +1 step
        check("valid carry accepted within wrap window", r.value, 501005.0);
        checkAction("sticky-wrap carry -> ACCEPT", r.action, RA_ACCEPT);
    }

    printf("S: the wrap window expires (a carry long after a wrap, no new wrap, is held)\n");
    {
        ReconcileState st = anchoredAt(500900.0, p);
        reconcileStep(st, 505900.0, /*clean*/false, /*wrap*/true, p);                     // arm (held)
        for (int i = 0; i < p.wrapWindow; ++i) reconcileStep(st, 500905.0, false, false, p); // age it out
        ReconcileResult r = reconcileStep(st, 501005.0, /*clean*/false, /*wrap*/false, p);
        check("carry held after wrap window expired", r.value, 500905.0);
    }

    printf("T: a valid carry with NO observed wrap is accepted via sub-position inference\n");
    {
        // Stopped-flow stall replay: the wrap frame was missed entirely; the candidate's
        // sub-position (94 -> 35) proves the dial below passed through zero.
        ReconcileState st = anchoredAt(500094.0, p);
        ReconcileResult r = reconcileStep(st, 500135.0, /*clean*/false, /*wrap*/false, p);
        check("carry accepted via inferred wrap", r.value, 500135.0);
        checkAction("inferred wrap -> ACCEPT", r.action, RA_ACCEPT);
    }

    printf("U: a flip keeps the sub-position (increases) -> inference does NOT fire\n");
    {
        ReconcileState st = anchoredAt(500294.0, p);   // sub = 94
        ReconcileResult r = reconcileStep(st, 500396.0, /*clean*/true, /*wrap*/false, p); // sub = 96
        check("flip (sub increased) held", r.value, 500294.0);
    }

    printf("V: small backward jitter of the dial below cannot fake wrap evidence\n");
    {
        ReconcileState st = anchoredAt(500294.0, p);   // sub = 94
        ReconcileResult r = reconcileStep(st, 500391.0, /*clean*/false, /*wrap*/false, p); // sub = 91
        check("flip with jittered-low sub held (margin)", r.value, 500294.0);
    }

    printf("W: stuck-recovery additionally requires a STABLE recognition\n");
    {
        ReconcileState st = anchoredAt(500200.0, p);
        ReconcileResult r;
        for (int i = 0; i < 8; ++i) {   // clean carry frames, but the value flip-flops
            double cand = (i % 2 == 0) ? 500320.0 : 500340.0;
            r = reconcileStep(st, cand, /*clean*/true, /*wrap*/false, p);
        }
        check("unstable candidates never recover", r.value, 500200.0);
        reconcileStep(st, 500320.0, /*clean*/true, /*wrap*/false, p);      // set lastCandidate
        r = reconcileStep(st, 500320.0, /*clean*/true, /*wrap*/false, p);  // stable -> recover
        check("stable candidate recovers", r.value, 500320.0);
        checkAction("stable recovery -> ACCEPT", r.action, RA_ACCEPT);
    }

    printf("\n==== %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
