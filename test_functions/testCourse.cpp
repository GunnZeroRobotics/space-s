float targPos[6][3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

int stage, totalStage;

void init()
{
    stage = 0;
    totalStage = 6;

    // About dist 1.13 from start
    targPos[0][0] = 0.7;
    targPos[0][1] = 0.7;
    targPos[0][2] = 0.7;

    // About dist 2.42 from previous
    targPos[1][0] = -0.7;
    targPos[1][1] = -0.7;
    targPos[1][2] = -0.7;

    // About dist 0.2 from previous
    targPos[2][0] = -0.7;
    targPos[2][1] = -0.7;
    targPos[2][2] = -0.5;

    // About dist 0.3 from previous
    targPos[3][0] = -0.7;
    targPos[3][1] = -0.4;
    targPos[3][2] = -0.5;

    // About 0.1 from previous
    targPos[4][0] = -0.7;
    targPos[4][1] = -0.4;
    targPos[4][2] = -0.6;

    // About 0.5 from previous
    targPos[5][0] = -0.7;
    targPos[5][1] = -0.4;
    targPos[5][2] = 0.1;
}

void loop()
{
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (stage < totalStage)
    {
        if (dist(myPos, targPos[stage]) < 0.03)
        {
            DEBUG(("Stage %d completed", stage));
            stage++;
        }
        else
        {
            moveFast(targPos[stage]);
        }
    }
    else
    {
        DEBUG(("Course Completed"));
    }
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3])
{
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}

void moveFast(float target[3])
{
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    float dist = mathVecMagnitude(vectorBetween, 3);
    if (dist < 0.03)
    {
        api.setPositionTarget(target);
    }
    else
    {
        mathVecNormalize(vectorBetween, 3);

        if (mathVecMagnitude(myVel, 3) / dist > 0.161)
        {
            DEBUG(("SLOW"));
            for (int i = 0; i < 3; i++)
            {
                vectorBetween[i] = 0;
            }
        }
        else
        {
            DEBUG(("SPEED"));
            for (int i = 0; i < 3; i++)
            {
                vectorBetween[i] *= ((dist < 0.1) ? (-32.5 * dist + 3.4) : 0.1);
            }
        }

        api.setVelocityTarget(vectorBetween);
    }
}