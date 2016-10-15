// STRATEGY: block opponent from entering their assembly zone
//
// Currently a very basic implementation -- not very good
//  -- Stays at midpoint between opponent position and assembly zone
// 
// Strategy is questionable due to the speed at which opponent can dock with
// an item after placing their SPSs

float myPos[3];
float myVel[3];
float myAtt[3];

float oppPos[3];

float itemPos[6][3];

float assemblyZone[3];
float oppAss[3];

float spsLoc[2][3];

void init()
{
    game.dropSPS();

    spsLoc[0][0] = -0.4;
    spsLoc[0][1] = 0.7;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = 0.4;
    spsLoc[1][1] = 0.7;
    spsLoc[1][2] = 0;
}

void loop()
{
    // Omitted satellite states, because I don't like them :)
    updateState();

    if (game.getNumSPSHeld() == 2)
    {
        if (closeTo(myPos, spsLoc[0], 0.03))
        {
            game.dropSPS();
        }
        else
        {
            api.setPositionTarget(spsLoc[0]);
        }
    }
    else if (game.getNumSPSHeld() == 1)
    {
        if (closeTo(myPos, spsLoc[1], 0.03))
        {
            game.dropSPS();
            float ass[4];
            game.getZone(ass);
            for (int i = 0; i < 3; i++)
            {
                assemblyZone[i] = ass[i];
                oppAss[i] = ass[i] * -1;
            }
        }
        else
        {
            api.setPositionTarget(spsLoc[1]);
            pointToward(itemPos[1]);
        }
    }
    else
    {
        // Midpoint:
        float mid[3];
        for (int i = 0; i < 3; i++) {
            mid[i] = (oppPos[i] + oppAss[i]) / 2;
        }
        api.setPositionTarget(mid);
    }
}

// MARK: Helper Methods

bool closeTo(float vec[3], float target[3], float threshold)
{
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < threshold;
}

void moveFast(float target[3]) {}

void updateState()
{
    float myState[12];
    float oppState[12];

    // SPHERE States
    api.getMyZRState(myState);
    api.getOtherZRState(oppState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        myAtt[i] = myState[i + 6];
        oppPos[i] = oppState[i];
    }

    // Item Positions
    for (int i = 0; i < 6; i++)
    {
        float itemState[12];
        game.getItemZRState(itemState, i);
        for (int j = 0; j < 3; j++)
        {
            itemPos[i][j] = itemState[j];
        }
    }
}

void pointToward(float target[3])
{
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

bool isFacing(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    float diff[3];
    mathVecSubtract(diff, vectorBetween, myAtt, 3);
    return mathVecMagnitude(diff, 3) < 0.04;
}