float targ[3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

float initPos[3];

void init()
{
    targ[0] = 0.7;
    targ[1] = 0.7;
    targ[2] = 0.7;

    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        initPos[i] = myState[i];
    }
}

void loop()
{
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (dist(myPos, targ) < 0.022 && mathVecMagnitude(myVel, 3) < 0.01) {
        DEBUG(("dist: %f, fuel: %f, time: %d", dist(myPos, initPos), ((60-game.getFuelRemaining())/60), api.getTime()));
    }

    mathVecSubtract(vectorBetween, targ, myPos, 3);
    float dist = mathVecMagnitude(vectorBetween, 3);
    if (dist < 0.03)
    {
        api.setPositionTarget(targ);
    }
    else
    {
        if (mathVecMagnitude(myVel, 3) / dist > 0.175) { // This is slow down constant
            DEBUG(("SLOW"));
            for (int i = 0; i < 3; i++) {
                vectorBetween[i] *= 0; // This is velocity constant 2
            }
        } else {
            DEBUG(("SPEED"));
            for (int i = 0; i < 3; i++) {
                vectorBetween[i] *= 0.1; // This is velocity constant 1
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