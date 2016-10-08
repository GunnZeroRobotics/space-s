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
// Using t = 18.5 s, acc(3 SPS) = 0.008765522279 ≈ 0.00877 m/s^2
// acc(no items) = 0.01205259313 ≈ 0.0121 m/s^2
//

float v[3];

void init()
{
    v[0] = v[1] = 0.0;
    v[2] = 10.0;
}

void loop()
{ 
    api.setVelocityTarget(v);
}