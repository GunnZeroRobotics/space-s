// DUPLICATE THIS FILE FOR TESTING

// CURRENT VERSION:
// Averages ~30 points per game without an opponent
//
// General TODOs:
// Replace all setPositionTarget with moveFast once it is completed
//
// Function/line specific TODOs commented on their corresponding lines

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
    // Set SPS locations
    // If there's a more consise way to set these please let me know - Kevin Li
    spsLoc[0][0] = 0.15;
    spsLoc[0][1] = 0;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.5;
    spsLoc[1][1] = 0.3;
    spsLoc[1][2] = 0;
    spsLoc[2][0] = -0.39;
    spsLoc[2][1] = -0.23;
    spsLoc[2][2] = -0.23;
}

void loop()
{
    // Updates state arrays of SPHEREs and items
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

bool closeTo(float vec[3], float target[3], float tolerance) {
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < tolerance;
}

// TODO: WHOEVER WROTE THIS PLEASE ADD MORE COMMENTS PLEASE PLEASE PLEASE 
void moveFast(float target[3]) {
    // api.setPositionTarget(target);
    float dist;
    float temp[3];
    int n;

    mathVecSubtract(temp, target, myPos, 3);
    dist = mathVecNormalize(temp, 3);

    if (dist > 0.5 * 0.01 * 36 + mathVecMagnitude(myPos + 3, 3) * 6) {
        for (n = 0; n < 3; n++) { //scale velocity (disp) to speed
            temp[n] *= dist;
        }
        api.setVelocityTarget(temp);
    } else {
        api.setPositionTarget(target);
    }
}

// Sets attitude toward a given point
void pointToward(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

// Checks if SPHERE is facing a target point with tolerance (in radians)
bool isFacing(float target[3], float tolerance) {
    float targetAtt[3];
    mathVecSubtract(targetAtt, target, myPos, 3);
    mathVecNormalize(targetAtt, 3);
    float theta;
    theta = acosf(mathVecInner(targetAtt, myAtt, 3));
    return theta < tolerance;
}

void dock(int itemID)
{
    float vectorBetween[3]; // Vector between SPHERE and target (item or assembly zone)

    float minDockingDist = (itemID < 2) ? 0.151 : ((itemID < 4) ? 0.138 : 0.124);
    float maxDockingDist = (itemID < 2) ? 0.173 : ((itemID < 4) ? 0.160 : 0.146);
    float avgDockingDist = (minDockingDist + maxDockingDist) / 2;

    // If you are holding the item, put it in your assembly zone
    // TODO: This needs to be improved. Looking for ideas. Post in slack.
    // Note: You do not have to stop/slow down to drop an item
    if (game.hasItem(itemID) == 1) {
        if (closeTo(myPos, assemblyZone, avgDockingDist) && isFacing(assemblyZone, (3.14 / 6.0))) {
            game.dropItem();
        }
        else {
            // Set position to assemblyZone's position scaled down by dockingDist
            mathVecSubtract(vectorBetween, assemblyZone, myPos, 3);
            float targetPos[3];
            for (int i = 0; i < 3; i++) { 
                targetPos[i] = vectorBetween[i] * ((mathVecMagnitude(vectorBetween, 3) - minDockingDist) / mathVecMagnitude(vectorBetween, 3));
                targetPos[i] += myPos[i];
            }
            moveFast(targetPos);
            pointToward(assemblyZone);
        }
    } else {
        float vectorTarget[3]; // Coordinates of target location to move to

        mathVecSubtract(vectorBetween, itemPos[itemID], myPos, 3);

        // Scale vectorTarget to the right length based on the docking distance
        float scale = (mathVecMagnitude(vectorBetween, 3) - minDockingDist) / mathVecMagnitude(vectorBetween, 3);
        for (int i = 0; i < 3; i++) { vectorTarget[i] = (vectorBetween[i] * scale) + myPos[i]; }
            
        // Checks if SPHERE satisfies docking requirements -- if so, docks
        // TODO: Check if we can use a tolerance > 0.25 because we can point at any of the 6 faces
        // TODO: Why are there still docking penalties??????? (although relatively rare)
        if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > maxDockingDist || !isFacing(itemPos[itemID], 0.25) || mathVecMagnitude(vectorBetween, 3) < minDockingDist) {
            moveFast(vectorTarget);
            pointToward(itemPos[itemID]);
        } else {
            game.dockItem(itemID);
        }
    }
}

// TODO: Write this function.
// Do testing in a separate file (use template.cpp)
// Return the itemID of the best item to dock with
// I think this requires moveFast to be completed -- Kevin Li
int optimalItem() { return 0; }
