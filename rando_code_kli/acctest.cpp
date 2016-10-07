//
// Simulate with:
// Satellite 1: X = 0, Y = 0, Z = -0.75, AttX = any, AttY = any, AttZ = any
// Satellite 2: X = 0.75, Y = any, Z = any, AttX = any, AttY = any, AttZ = any
//
// Testing Results:
// Attitude does not affect acceleration
// Acceleration varies through multiple trials
// 
// Testing Data:
// Satellite traveled 1.50 meters while carrying 3 SPS (8/11 acc)
// t = 18.4 - 18.6 seconds
// Using t = 18.5 s, acc(3 SPS) = 0.008765522279 ≈ 0.00877 m/s
// acc(no items) = 0.01205259313 ≈ 0.0121 m/s
//

float o[3];
float v[3];
float s[3];

void init()
{
    for (int i = 0; i < 3; i++) {
        o[i] = 0.0;
        s[i] = -0.75;
        v[i] = 0.0;
    }
    v[2] = 10.0;
}

void loop()
{ 
    api.setVelocityTarget(v);
}