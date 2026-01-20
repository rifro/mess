// updated with ringFilter pow2 and RdvVoter params (repurposed for single-mode)
struct Configuration {
    struct BoerRingFilter {
        float r1sq = pow2(0.001f);      // 1mm
        float r2sq = pow2(0.07f);       // 7cm
        float epsSq = pow2(0.001f);     // 1mm tolerance
        float minAreaSq = pow2(1e-6f);  // collinear check
        float mortonLeapSq = pow2(10.0f);  // 10m leap threshold
    } ringFilter;

    struct RdvVoter {
        float cosCutoff = 0.2588f;      // cos75 for orthogonal
        int burstWindow = 4;            // repurposed for ringSize if burst off
        float planeThreshold = 0.98f;   // cos11.5 for plane (optional in single)
        int minBurstCount = 3;          // for early-stop in voting
        float recentAxisCutoff = 15.0f; // our: skip old axes in reverse
        float mergeToleranceDeg = 7.5f; // our: cosMerge for reduce
    } RdvVoter;

    // ... other structs
};
constant Configuration g_config;
