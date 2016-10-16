// TODO: Create a faster version of moving than setPositionTarget
//       using either setVelocity or setForce
//
// Last update:

// myState variables
float myPos[3];
float myVel[3];
float myAtt[3];

// otherState variables
// Currently not in use, uncomment these and their corresponding lines
// in the updateState function if needed
// float otherPos[3];
// float otherVel[3];
// float otherAtt[3];

float itemPos[6][3]; // itemPos[itemID][x/y/z coordinate]

float assemblyZone[3]; // x, y, z coordinates of assembly zone

float spsLoc[3][3]; // spsLoc[sps drop number][x/y/z coordinate]

void init()
{
    game.dropSPS(); // drop SPS at spawn point

    // Set SPS locations
    // If there's a more consise way to set these please let me know - Kevin Li
    spsLoc[0][0] = 0.15;
    spsLoc[0][1] = 0;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.5;
    spsLoc[1][1] = 0.3;
    spsLoc[1][2] = 0;
    spsLoc[2][0] = -0.36;
    spsLoc[2][1] = -0.3;
    spsLoc[2][2] = -0.22;
}

void loop()
{
    updateState();
}

// MARK: Helper Methods

void updateState()
{
    float myState[12];
    // float otherState[12];

    // SPHERE States
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        myAtt[i] = myState[i + 6];
        // otherPos[i] = otherState[i];
        // otherVel[i] = otherState[i + 3];
        // otherAtt[i] = otherAtt[i + 6];
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

bool closeTo(float vec[3], float target[3], float threshold) {
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < threshold;
}

void moveFast(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    if (mathVecMagnitude(vectorBetween, 3) < 0.2) {
        api.setPositionTarget(vectorBetween);
    } else {
        api.setVelocityTarget(vectorBetween);
    }
}

// Sets attitude toward a given point
void pointToward(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

// Checks if SPHERE is facing a target point with threshold of 0.25 radians (14.3 degrees)
bool isFacing(float target[3]) {
    float targetAtt[3];
    mathVecSubtract(targetAtt, target, myPos, 3);
    mathVecNormalize(targetAtt, 3);
    float theta;
    theta = acosf(mathVecInner(targetAtt, myAtt, 3));
    return theta < 14.3f;
}

void dock(int itemID)
{
    float dockingDist = (itemID < 2) ? 0.154 : ((itemID < 4) ? 0.142 : 0.128);

    // If you are holding the item, put it in your assembly zone
    // Note: You do not have to stop/slow down to drop an item
    if (game.hasItem(itemID) == 1) {
        if (closeTo(myPos, assemblyZone, dockingDist)) {
            game.dropItem();
        }
        else {
            api.setPositionTarget(assemblyZone);
            pointToward(assemblyZone);
        }
    } else {
        float vectorBetween[3]; // Vector between SPHERE and target item
        float vectorTarget[3]; // Coordinates of target location to move to

        mathVecSubtract(vectorBetween, itemPos[itemID], myPos, 3);
 
        // Scale vectorTarget to the right length based on the docking distance
        float scale = (mathVecMagnitude(vectorBetween, 3) - dockingDist) / mathVecMagnitude(vectorBetween, 3);
        for (int i = 0; i < 3; i++) {
            vectorBetween[i] = vectorBetween[i] * scale;
            vectorTarget[i] = vectorBetween[i] + myPos[i];
            vectorBetween[i] = vectorBetween[i] / scale;
        }
            
        // Checks if SPHERE satisfies docking requirements -- if so, docks
        if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist || !isFacing(itemPos[itemID])) {
            api.setPositionTarget(vectorTarget);
            pointToward(itemPos[itemID]);
        } else {
            game.dockItem(itemID);
        }
    }
}

int optimalItem() { return 0; }
