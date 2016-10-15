// STRATEGY: Repeatedly hit the opponent with the item you're holding
//           to invoke the collision penalty for opponent
//
// Currently very basic implementation, only chases after 
// opponent while holding item
//
// Questionable strategy because of inconsistency and the small magnitude
// of the collision penalty

float myPos[3];
float myVel[3];
float myAtt[3];

float oppPos[3];

float itemPos[6][3];

float assemblyZone[3];

float spsLoc[2][3];

int moveCommand;

float dockTroll;

void init()
{
    game.dropSPS();
    game.dropSPS();
    game.dropSPS();

    moveCommand = 0;

    spsLoc[0][0] = -0.5;
    spsLoc[0][1] = 0.3;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.4;
    spsLoc[1][1] = -0.3;
    spsLoc[1][2] = -0.22;
    
    dockTroll = 0.163;
}

void loop()
{
    // Omitted satellite states, because I don't like them :)
    updateState();

    if (!game.hasItemBeenPickedUp(0)) {
        dock(0);
    } else {
        api.setPositionTarget(oppPos);  
        pointToward(oppPos);     
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
    float targetAtt[3];
    mathVecSubtract(targetAtt, target, myPos, 3);
    mathVecNormalize(targetAtt, 3);
    float theta;
    theta = acosf(mathVecInner(targetAtt, myAtt, 3));
    return theta < 14.5f;
}

void dock(int itemID)
{
    float vectorBetween[3];
    float vectorTarget[3];

    float dockingDist = (itemID < 2) ? 0.162 : ((itemID < 4) ? 0.149 : 0.135);

    mathVecSubtract(vectorBetween, itemPos[itemID], myPos, 3);
    float scale = (mathVecMagnitude(vectorBetween, 3) - dockingDist) / mathVecMagnitude(vectorBetween, 3);
    for (int i = 0; i < 3; i++)
    {
        vectorBetween[i] = vectorBetween[i] * scale;
        vectorTarget[i] = vectorBetween[i] + myPos[i];
        vectorBetween[i] = vectorBetween[i] / scale;
    }

    if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist || !isFacing(itemPos[itemID]))
    {
        api.setPositionTarget(vectorTarget);
        pointToward(itemPos[itemID]);
    }
    else if (game.hasItem(itemID) != 1)
    {
        game.dockItem();
    }
}
