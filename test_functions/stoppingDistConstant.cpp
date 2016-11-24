float targ[3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

float initPos[3], initVel;

int startTime;
float startFuel;
float startPos;

bool passedOrigin;

void init()
{
    targ[0] = 0.7;
    targ[1] = 0.7;
    targ[2] = 0.7;

    corner[0] = -0.5;
    corner[1] = 0.7;
    corner[2] = 0.7;

    passedOrigin = false;
}

void loop()
{
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (api.getTime() < 40)
    {
        api.setPositionTarget(corner);
        return;
    }
    else if (api.getTime() >= 40)
    {
        mathVecSubtract(vectorBetween, targ, myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);
        mathVecNormalize(vectorBetween, 3);

        if (myPos[0] > 0)
        {
            if (!passedOrigin)
            {
                startFuel = game.getFuelRemaining();
                startTime = api.getTime();
                startPos = myPos[0];
                initVel = myVel[0];
                passedOrigin = true;
            }
            for (int i = 0; i < 3; i++)
            {
                vectorBetween[i] *= 0.1; // Velocity Constant
            }
            if (myVel[0] < 0)
            {
                int t = api.getTime() - startTime;
                float fuelPercent = (startFuel - game.getFuelRemaining()) / 60;
                float d = myPos[0] - startPos;
                DEBUG(("initVel: %f, fuel: %f, time: %d, dist: %f", initVel, fuelPercent, t, d));
            }
        }
        api.setVelocityTarget(vectorBetween);
    }
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3])
{
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}
