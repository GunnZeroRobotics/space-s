// CURRENT VERSION:
// Averages ~35 points per game without an opponent
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
    // Updates state arrays of SPHEREs and items
    updateState();

    if (game.getNumSPSHeld() != 0) {
        // Code for placing SPSs
        int spsHeld = game.getNumSPSHeld();

        if (closeTo(myPos, spsLoc[3 - spsHeld], 0.08)) {
            // If close to sps location, drop SPS and update SPS position array
            // Large threshold used (8 cm) because precision not needed and
            // takes too long to slow down
            game.dropSPS();
            for (int i = 0; i < 3; i++) { spsLoc[3 - spsHeld][i] = myPos[i]; }
            spsHeld--;

            if (spsHeld == 0) {
                // Save assembly zone location if no SPSs left
                float ass[4];
                game.getZone(ass);
                for (int i = 0; i < 3; i++) { assemblyZone[i] = ass[i]; }
            }
        } else {
            api.setPositionTarget(spsLoc[3 - spsHeld]);
        }
    } else {
        // All SPSs are placed, docking and assembly code
        // Iterates through the items that are already in our assembly zone,
        // Docks with priority of large --> medium
        // Small items excluded due to lack of time (most likely)
        // If all large and medium items are already in our assembly zone, this 
        // enters an infinite loop. (Is that case possible???)
        
        // TODO: Replace item selection with optimalItem function once it is complete
        // Note: optimalItem probably requires moveFast -- correct me if I am wrong
        int IDcount = 1;
        while (game.itemInZone(IDcount)) {
            if (game.hasItem(IDcount) == 1) {
                break;
            }
            IDcount--;
            if (IDcount < 0) {
                IDcount = 3;
            }
        }

        dock(IDcount);
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

bool closeTo(float vec[3], float target[3], float threshold) {
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < threshold;
}

// TODO: URGENT -- complete this function
// Do testing in a separate file (either template.cpp or templateWithoutSPS.cpp)
void moveFast(float target[3]) {}

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
    // TODO: Make sure you are facing the assembly zone while being dockingDist meters away
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
        // TODO: Make sure SPHERE is not too close to item before docking
        if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist || !isFacing(itemPos[itemID])) {
            api.setPositionTarget(vectorTarget);
            pointToward(itemPos[itemID]);
        } else {
            game.dockItem(itemID);
        }
    }
}

// TODO: Write this function.
// Do testing in a separate file (use template.cpp)
// Return the itemID of the best item to dock with
int optimalItem() {}