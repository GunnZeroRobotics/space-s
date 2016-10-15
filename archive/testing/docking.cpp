// UPDATE: Docking is functional (although with some errors), moving this to archive
// KEPT ONLY FOR REFERENCE
//
// Simulate with:
// Satellite 1: X = any, Y = any, Z = any, AttX = any, AttY = any, AttZ = any
// Satellite 2: X = any, Y = any, Z = any, AttX = any, AttY = any, AttZ = any

int gameTime;

float myState[12]; // our satellite state
float myPos[3];
float myVel[3];
float otherState[12]; // enemy satellite state
float otherPos[3];

float itemPos[6][3];

void init()
{
    // Initial item positions
    for (int i = 0; i < 6; i++) { 
        itemPos[(i < 3) ? 0 : 1][i % 3] = (i < 3) ? 0.23 : -0.23; // Large items
        itemPos[(i < 3) ? 2 : 3][i % 3] = (i % 2 == 0) ? 0.36 : -0.36; // Medium items
        itemPos[(i < 3) ? 4 : 5][i % 3] = (i < 1 || i > 3) ? 0.50 : -0.50; // Small items
    }
    
    gameTime = 0;
}

void loop()
{ 
   gameTime++; 
   updateState();
   
   dock(0);
}

// MARK: Helper Methods

void updateState()
{
    // SPHERE States
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        otherPos[i] = otherState[i];
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
    float dockingDist = (itemID < 2) ? 0.162 : ((itemID < 4) ? 0.139 : 0.128);
    float mProportion = (mathVecMagnitude(vectorBetween, 3) - dockingDist) / mathVecMagnitude(vectorBetween, 3);
    for (int i = 0; i < 3; i++) {
        vectorBetween[i] = vectorBetween[i] * mProportion;
        vectorTarget[i] = vectorBetween[i] + myPos[i];
        vectorBetween[i] = vectorBetween[i] / mProportion;
    }
    
    if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist) {
        api.setPositionTarget(vectorTarget);
        pointToward(itemPos[itemID]);
    } else {
        game.dockItem();
    }
}
