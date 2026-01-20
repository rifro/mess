// Jules' + onze adds
struct RdvConfig
{
    struct RdvSettings
    {
        float recentAxisCutoff = 15.0f; // Onze: Skip oude axes in reverse
        // ...
    } rdv;
    struct BoerRing
    {
        float r1Sq         = 0.1f;
        float r2Sq         = 1.0f;
        float mortonLeapSq = 100.0f;
    } ringFilter;
    // ...
};
