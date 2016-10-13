float myPos[3];
float myVel[3];
float myAtt[3];

float itemPos[6][3];

float assemblyZone[3];

float spsLoc[2][3];

int moveCommand;

void init()
{
    game.dropSPS();

    moveCommand = 0;

    spsLoc[0][0] = -0.5;
    spsLoc[0][1] = 0.3;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.4;
    spsLoc[1][1] = -0.3;
    spsLoc[1][2] = -0.22;
}

void loop()
{
    // Omitted satellite states, because I don't like them :)
    updateState();

    if (game.getNumSPSHeld() == 2)
    {
        if (closeTo(myPos, spsLoc[0], 0.08))
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
        if (closeTo(myPos, spsLoc[1], 0.08))
        {
            game.dropSPS();
            float ass[4];
            game.getZone(ass);
            for (int i = 0; i < 3; i++)
            {
                assemblyZone[i] = ass[i];
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
        int IDcount = 1;
        while (game.hasItemBeenPickedUp(IDcount) && game.hasItem(IDcount) != 1)
        {
            IDcount--;
            if (IDcount < 0)
            {
                IDcount = 3;
                break;
            }
        }
        dock(IDcount);
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

    // SPHERE States
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        myAtt[i] = myState[i + 6];
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

bool isFacing(float target[3])
{
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

    // If you are holding the item, put it in your assembly zone
    if (game.hasItem(itemID) == 1)
    {
        if (closeTo(myPos, assemblyZone, dockingDist))
        {
            game.dropItem();
        }
        else
        {
            api.setPositionTarget(assemblyZone);
            pointToward(assemblyZone);
        }
        return;
    }

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
