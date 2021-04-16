#define SERIAL_BAUD 115200  // Baudrate

// --------------------------
// Joystick related variables
// --------------------------

#define default_gain 1.0
#define totalGain default_gain
#define constantGain default_gain
#define rampGain default_gain
#define squareGain default_gain
#define sineGain default_gain
#define triangleGain default_gain
#define sawtoothdownGain default_gain
#define sawtoothupGain default_gain
#define springGain default_gain
#define inertiaGain default_gain
#define frictionGain 0.25
#define damperGain default_gain
#define damperSplineNumPoints 6

// comment out this line if you don't want to have a spline configuration for the damper
#define damperSplineGain float damperSplinePoints[2][damperSplineNumPoints] = { \
    {0, 0, 2500, 6000, 10000, 10000}, \
    {0, 0, 250, 4000, 5000, 5000}}
