float myPos[3];
float myVel[3];

float itemPos[6][3];

void init()
{
}

void loop()
{ 
   updateState();
}

// MARK: Helper Methods

void moveFast(float target[3]) { }

void updateState()
{
    float myState[12];

    // SPHERE States
    // Currently omitted current satellite state, can add when needed
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    // Item Positions
    for (int i = 0; i < 6; i++) {
        float itemState[12];
        game.getItemZRState(itemState, i);
        for (int j = 0; j < 3; j++) {
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

void dock(int itemID)
{
    float vectorBetween[3];
    float vectorTarget[3];
    for (int i = 0; i < 3; i++) {
        vectorTarget[i] = itemPos[itemID][i];
    }
    mathVecSubtract(vectorBetween, itemPos[itemID], myPos,3);
    float dockingDist = (itemID < 2) ? 0.162 : ((itemID < 4) ? 0.149 : 0.135);
    float mProportion = (mathVecMagnitude(vectorBetween, 3) - dockingDist * 0.9) / mathVecMagnitude(vectorTarget, 3);
    for (int i = 0; i < 3; i++) {
        vectorBetween[i] = vectorBetween[i] * mProportion;
        vectorTarget[i] = vectorBetween[i] + myPos[i];
        vectorBetween[i] = vectorBetween[i] / mProportion;
    }
    
    if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist) {
        api.setPositionTarget(vectorTarget);
        pointToward(itemPos[itemID]);
    } else if (game.hasItem(itemID) != 1){
        game.dockItem();
    }
}
