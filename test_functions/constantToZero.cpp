float targ[3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

float initPos[3];

void init()
{
    targ[0] = 0.7;
    targ[1] = 0.7;
    targ[2] = 0.7;
}

void loop()
{
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (dist(myPos, targ) < 0.05) {
        DEBUG(("%d", api.getTime()));
    }

    mathVecSubtract(vectorBetween, targ, myPos, 3);
    float dist = mathVecMagnitude(vectorBetween, 3);
    if (dist < 0.03)
    {
        api.setPositionTarget(targ);
    }
    else
    {
        if (mathVecMagnitude(myVel, 3) / dist > 0.18) { // This is slow down constant
            DEBUG(("SLOW"));
            for (int i = 0; i < 3; i++) {
                vectorBetween[i] = 0;
            }
        } else {
            DEBUG(("SPEED"));
            for (int i = 0; i < 3; i++) {
                vectorBetween[i] *= 0.16; // This is velocity constant
            }
        }
    }

    api.setVelocityTarget(vectorBetween);
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3])
{
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}
