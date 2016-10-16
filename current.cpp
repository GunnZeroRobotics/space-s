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

int rB; //modifies SPS locations based on our starting position
int nextItem; //ID of item we are picking up
int itemHeld; //0 if no item held, 1 if we are holding an item

void init()
{
    game.dropSPS(); // drop SPS at spawn point

    // Set SPS locations
    // If there's a more consise way to set these please let me know - Kevin Li
    updateState();
    itemHeld = 0;
    rB = (myPos[1] < 0) ? -1 : 1;
    spsLoc[0][0] = 0.15 * rB;
    spsLoc[0][1] = 0;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.5 * rB;
    spsLoc[1][1] = 0.3 * rB;
    spsLoc[1][2] = 0;
    spsLoc[2][0] = -0.39 * rB;
    spsLoc[2][1] = -0.23 * rB;
    spsLoc[2][2] = -0.23 * rB;
}

void loop()
{
    // Updates state arrays of SPHEREs and items
    updateState();

    if (game.getNumSPSHeld() != 0) {
        // Code for placing SPSs
        int spsHeld = game.getNumSPSHeld();

        if (closeTo(myPos, spsLoc[3 - spsHeld], (spsHeld == 1) ? 0.03 : 0.08)) {
            // If close to sps location, drop SPS and update SPS position array
            // Large tolerance used (8 cm) because precision not needed and
            // takes too long to slow down
            // Small tolerance used for last SPS because it is right next to an item
            game.dropSPS();
            for (int i = 0; i < 3; i++) { spsLoc[3 - spsHeld][i] = myPos[i]; }
            spsHeld--;

            if (spsHeld == 0) {
                // Save assembly zone location if no SPSs left
                float ass[4];
                game.getZone(ass);
                for (int i = 0; i < 3; i++) { assemblyZone[i] = ass[i]; }

                // If requirements of docking are satisfied, immediately dock (saves 1 second)
                float vectorBetween[3];
                mathVecSubtract(vectorBetween, itemPos[1], myPos, 3);
                if (mathVecMagnitude(myVel, 3) < 0.01 || mathVecMagnitude(vectorBetween, 3) < 0.173 || !isFacing(itemPos[1], 0.25) || mathVecMagnitude(vectorBetween, 3) > 0.151) {
                    game.dockItem(1);
                }
            }
        } else {
            moveFast(spsLoc[3 - game.getNumSPSHeld()]);
        }
    } else { // All SPSs are placed
        if (itemHeld == 0){
            nextItem = optimalItem();
        }
        dock(nextItem);
    }
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
            itemHeld = 0; //updates itemHeld state
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
            itemHeld = 1; //updates itemHeld state
        }
    }
}

// TODO: Write this function.
// Do testing in a separate file (use template.cpp)
// Return the itemID of the best item to dock with
// I think this requires moveFast to be completed -- Kevin Li
int optimalItem()
{
    int currID = 0;
    float minDist = 2.59807621;
    //^diagonal distance of interaction space is upper limit of any distance
    int minID = -1;
    float vectorBetween[3];

    //three loops: large, medium, small
    for (int i = 0; i < 3; i++){
        //reset these values to recompute best item for next item size class
        currID = i * 2;
        minDist = 2.59807621;
        minID = -1;

        while (currID < (i + 1) * 2){
            if (game.hasItem(currID) != 2 && !game.hasItemBeenPickedUp(currID)){
                //^if enemy doesn't have item and item is not in their assembly zone
                mathVecSubtract(vectorBetween, itemPos[currID], myPos, 3);
                if (mathVecMagnitude(vectorBetween, 3) < minDist){
                    //^if this item is the closest so far
                    minDist = mathVecMagnitude(vectorBetween, 3);
                    minID = currID;
                }
            }
            currID++;
        }
        if (minID != -1) return minID;
    }
}
