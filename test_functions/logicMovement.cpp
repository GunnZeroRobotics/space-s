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

float totalDist; // total distance to an item
float minDist; // distance where sphere switches to setPositionTarget
float forces[3]; // force vector
float totalForces[3];

void init()
{
    game.dropSPS(); // drop SPS at spawn point
    totalDist = 0;
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
    totalForces[0] = totalForces[1] = totalForces[2] = 0;
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
            totalDist = 0;
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

bool closeTo(float vec[3], float target[3], float tolerance) {
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < -100;
}

// moveFast Function using Forces: accelerates halfway to the given target, then decelerates to the point.
void moveFast(float target[3]) {
    float dist; // current distance from target
    float temp[3]; // temp vector for calculations setting forces
    mathVecSubtract(temp, target, myPos, 3);
    dist = mathVecNormalize(temp, 3);

    if (totalDist == 0) { // initial setup, run once per unique target
        // sets the totalDist to the dist found above = Total Distance needed to travel
        // sets minDist = dist – minDist is the minimum distance to the point the sphere has reached
        totalDist = minDist = dist;
        // set the force vector
        forces[0] = temp[0];
        forces[1] = temp[1];
        forces[2] = temp[2];
        // reset forces – doesnt rlly work, if theres another way let me (@myh1000) know or just comment this out if we're using the circumscribe method.
        temp[0] = temp[1] = temp[2] = 0;
        api.setForces(temp);
        DEBUG(("totalforces = [%f, %f, %f]",totalForces[0], totalForces[1], totalForces[2]));
        DEBUG(("totalDist = %f",totalDist));
    }
    // minDist represents when the sphere decelerates to a point where setPositionTarget is better
    if (dist < minDist) {
        minDist = dist;
    }

    DEBUG(("dist = %f; %f, %f",dist, totalDist, minDist));
    if (dist > totalDist / 2) { // not halfway there yet; accelerates
        DEBUG(("accelerating, dist = %f",dist));
        DEBUG(("accelerating, totalDist/2 = %f",totalDist / 2)); // halfway point, decelerates when distance < totalDist/2
        for (int n = 0; n < 3; n++) {
            totalForces[n] += forces[n];
        }
        api.setForces(forces);
    }
    else if (dist <= minDist) { // past halfway point; decelerates
        DEBUG(("decelerating, dist = %f", dist));
        // make the force vector point in the opposite direciton
        for (int n = 0; n < 3; n++) {
            totalForces[n] += forces[n] * -1;
            temp[n] = forces[n] * -1;
        }
        api.setForces(temp); // slows it down by a set amount
    }
    /* when dist is > minDist, that means the sphere has started to move backwards a bit.
    Sphere has slowed down considerably, use setPositionTarget */
    else {
        minDist = 0; // so this else statement is always called after the first time
        // might be unnecessary if setPositionTarget overrides any forces
        temp[0] = temp[1] = temp[2] = 0;
        for (int n = 0; n < 3; n++) {
            // totalForces[n] += forces[n] * -1;
            temp[n] = totalForces[n] * -1;
        }
        api.setForces(temp);
        DEBUG(("totalforces = [%f, %f, %f]",totalForces[0], totalForces[1], totalForces[2]));
        // api.setPositionTarget(target);
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
            totalDist = 0;
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
            totalDist = 0;
            game.dockItem(itemID);
        }
    }
}

// TODO: Write this function.
// Do testing in a separate file (use template.cpp)
// Return the itemID of the best item to dock with
// I think this requires moveFast to be completed -- Kevin Li
int optimalItem() { return 0; }
