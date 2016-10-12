int gameTime;
int rB;

float myState[12]; // our satellite state
float myPos[3];
float myVel[3];
float otherState[12]; // enemy satellite state
float otherPos[3];

float itemPos[6][3];

float assemblyZone[4];
float realAss[3];
float vectorBetweenUs[3];
float vectorBetweenThem[3];
float vectorTarget[3];

int currentBehavior;
int targetitem;

void init()
{
    currentBehavior = 1;
    gameTime = 0;
    api.getMyZRState(myState);
    rB = (myState[1] < 0) ? -1 : 1;
    game.dropSPS();
}

void loop()
{
    // update information on game state
    gameTime++;
    updateState();
    
    // perform the behavior we are on
    // todo: we should go to the next behavior based on once we reach the point, instead of based on time
    // todo: use better SPS triangle coordinates (RESOLVED)
    // todo: make the docking function faster, better, etc.
    // todo: make the item placement in the center of assembly zone
    // todo: make moving better, efficient.
    // todo: make item target selection better.
    if(currentBehavior == 1)
    {
        // moving towards second SPS point
        float temp[3] = {-.5 * rB, .3 * rB, 0};
        moveTo(temp);
        // go to next behavior and drop second SPS
        if(gameTime > 40)
        {
            currentBehavior = 2;
            game.dropSPS();
        }
    }
    if(currentBehavior == 2)
    {
        // moving towards third SPS point
        float temp[3] = {-.35 * rB, -.3 * rB, -.22 * rB};
        moveTo(temp);
        // go to next behavior, drop third SPS, get assembly zone
        if(gameTime > 65)
        {
            currentBehavior = 3;
            // get assembly zone info
            game.dropSPS();
            game.getZone(assemblyZone);
            DEBUG(("%f, %f, %f, %f", assemblyZone[0], assemblyZone[1], assemblyZone[2], assemblyZone[3]));
            for (int i = 0; i < 3; i++) {
                realAss[i] = assemblyZone[i];// * 0.85;
            }
            // choose item to go after
            targetitem = chooseItemToPickup();
        }
    }
    if(currentBehavior == 3)
    {
        // moving towards item and rotate to face it, to pick it up
        for (int i = 0; i < 3; i++) {
            vectorTarget[i] = itemPos[targetitem][i];
        }
        mathVecSubtract(vectorBetweenUs, itemPos[targetitem], myPos,3);
        float dockingDist = (targetitem < 2) ? 0.162 : ((targetitem < 4) ? 0.149 : 0.135);
        float mProportion = (mathVecMagnitude(vectorBetweenUs, 3) - dockingDist) / mathVecMagnitude(vectorBetweenUs, 3);
        for (int i = 0; i < 3; i++) {
            vectorBetweenUs[i] = vectorBetweenUs[i] * mProportion;
            vectorTarget[i] = vectorBetweenUs[i] + myPos[i];
            vectorBetweenUs[i] = vectorBetweenUs[i] / mProportion;
        }
        moveTo(vectorTarget);
        pointToward(itemPos[targetitem]);
        // go to next behavior once its close enough and facing right direction, and dock.
        if(mathVecMagnitude(myVel, 3) < 0.01 && mathVecMagnitude(vectorBetweenUs, 3) < dockingDist)
        {
            currentBehavior = 4;
            game.dockItem();
        }
    }
    if(currentBehavior == 4)
    {
        // we have the item, moving towards the assembly point.
        moveTo(realAss);
        pointToward(realAss);
        
        float vb[3];
        mathVecSubtract(vb, realAss, myPos, 3);
        // go to next behavior once we're at the assembly point, and drop the item.
        if(mathVecMagnitude(vb, 3) < 0.03)
        {
            game.dropItem();
            
            // Switch back to behavior #3, so we can go pick up more items.
            // Also, we need to choose a new item to pickup.
            targetitem = chooseItemToPickup();
            currentBehavior = 3;
        }
    }
}

// which item should we go after?
// todo: right now this just picks by item ID LOL we should make it smarter,
// like picking the one with most points/time efficiency
// EDIT: ^^ now chooses closest item with respect to size
int chooseItemToPickup()
{
    int IDcount = 0;
    float currDistance;
    float minDistance = 2.59807621; //cross diagonal of interaction space
    int minIndex = 0;
    
    while (IDcount <= 5) {
        if (game.hasItem(IDcount) != 2){
            mathVecSubtract(vectorBetweenUs, itemPos[targetitem], myPos,3);
            currDistance = mathVecMagnitude(vectorBetweenUs, 3);
            if (currDistance < minDistance){
                currDistance = minDistance;
                minIndex = IDcount;
            }
        }
        return minIndex;
    }
}

void moveTo(float point[3])
{
    // is there a better way to do this?
    api.setPositionTarget(point);
}

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
    mathVecSubtract(vectorBetweenUs, target, myPos, 3);
    mathVecNormalize(vectorBetweenUs, 3);
    api.setAttitudeTarget(vectorBetweenUs);
}
