// Temporary values that are relatively accurate
float accMax;
float mass;
float fMax;

int gameTime;

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

void init()
{
    gameTime = 0;

    game.dropSPS(); // drop SPS at spawn point

    // Set SPS locations
    // If there's a more consise way to set these please let me know - Kevin Li
    updateState();
    rB = (myPos[1] < 0) ? -1 : 1;
    spsLoc[0][0] = 0;
    spsLoc[0][1] = 0.15 * rB;
    spsLoc[0][2] = 0;
    spsLoc[1][0] = -0.5 * rB;
    spsLoc[1][1] = 0.3 * rB;
    spsLoc[1][2] = 0;
    spsLoc[2][0] = -0.39 * rB;
    spsLoc[2][1] = -0.23 * rB;
    spsLoc[2][2] = -0.23 * rB;

    // Temporary values that are relatively accurate
    mass = 4.65; // 4.64968
    accMax = 0.008476;
    fMax = mass * accMax; // 0.039411
}

void loop()
{
    gameTime++;

    // Updates state arrays of SPHEREs and items
    updateState();

    if (game.getNumSPSHeld() != 0) {
        // Code for placing SPSs
        int spsHeld = game.getNumSPSHeld();

        if (closeTo(myPos, spsLoc[3 - spsHeld], (spsHeld == 1) ? 0.03 : 0.08)) {
            // If close to sps location, drop SPS and update SPS position array
            // Large tolerance used (8 cm) because precision not needed and takes too long to slow down
            // Small tolerance used for last SPS because it is right next to an item
            game.dropSPS();
            for (int i = 0; i < 3; i++) { spsLoc[3 - spsHeld][i] = myPos[i]; }
            spsHeld--;

            if (spsHeld == 0) {
                // Save assembly zone location if no SPSs left
                float ass[4];
                game.getZone(ass);
                for (int i = 0; i < 3; i++) { assemblyZone[i] = ass[i]; }
                dock(optimalItem());
            }
        } else {
            moveFast(spsLoc[3 - game.getNumSPSHeld()]);
        }
    } else { // All SPSs are placed
        dock(optimalItem());
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

// Checks if SPHERES is ____ away from target location -- if so, use setVelocity, else, use setPosition
void moveFast(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    float dist = mathVecMagnitude(vectorBetween, 3);

    float velocityMag = mathVecMagnitude(myVel, 3);

    switch (game.getNumSPSHeld()) {
        case 0: 

                //if (velocityMag > 0.09) {  // Distance between SPHERE and target location
                //    float zero[3] = {0, 0, 0};
                //    DEBUG(("too fast 5 u"));
                    // float dist = mathVecMagnitude(vectorBetween, 3);
                //    api.setVelocityTarget(zero);
                //    return;
                //}
                
                // Not sure who changed values inside of if statment & added for loop - Larry
                if (dist > 0.5 * 0.01 * 36 + mathVecMagnitude(myPos + 3, 3) * 6) {
                    for (int n = 0; n < 3; n++) { //scale velocity (disp) to speed
                        vectorBetween[n] *= dist;
                    }
                    api.setVelocityTarget(vectorBetween);
                } else {
                    api.setPositionTarget(target);
                }
                break;
        default: 
                if (dist < 0.03) {
                    api.setPositionTarget(target);
                } else {

                    float vPerpMag = velocityMag * sinf(angleBetween(vectorBetween, myVel));
                    float vParallelMag = velocityMag * cosf(angleBetween(vectorBetween, myVel));

                    // Find the direction of the velocity component that is perpendicular to vectorBetween
                    float vTemp[3]; // myVel cross product vectorBetween, perpendicular to both
                    mathVecCross(vTemp, myVel, vectorBetween);
                    float vPerp[3]; // vectorBetween cross product result of above, gives perpendicular component of velocity
                    mathVecCross(vPerp, vectorBetween, vTemp);

                    mathVecNormalize(vectorBetween, 3);
                    mathVecNormalize(vPerp, 3);

                    float perpForce = 0;
                    float parallelForce = 0;

                    if (dist < ((vParallelMag * vParallelMag) / (2 * accMax * accFactor() * 0.85))) { // The second constant determines how early we should start decelerating
                        parallelForce = -0.9 * fMax; // This constant determines how fast we should slow down
                        //if ((mass * vPerpMag) < sqrtf((fMax * fMax) - (parallelForce * parallelForce))){
                        //    perpForce = mass * vPerpMag;
                        //} else perpForce = sqrtf((fMax * fMax) - (parallelForce * parallelForce));
                    } else {
                        perpForce = mass * vPerpMag;
                        if (perpForce < fMax) {
                            parallelForce = sqrtf((fMax * fMax) - (perpForce * perpForce));
                        }
                        // Tried to fix overshooting with constants. It sucks.
                        // if (vParallelMag > 0.06) { parallelForce *= 0.25; }
                        // else if (dist < 0.02) { parallelForce = 0; }
                        // else if (dist < 0.05) { parallelForce *= 0.25; }
                        // else if (dist < 0.15 && vParallelMag > 0.03) { parallelForce *= 0.25; }
                        // else if (dist < 0.25 && vParallelMag > 0.05) { parallelForce *= 0.25; }
                        // else { parallelForce *= 0.95; }
                    }

                    float totalForce[3];
                    for (int i = 0; i < 3; i++) {
                        vPerp[i] *= (perpForce * -1);
                        vectorBetween[i] *= parallelForce;
                        totalForce[i] = vPerp[i] + vectorBetween[i];
                    }

                    //DEBUG(("dist: %f", dist));
                    //DEBUG(("vel: %f, %f", vParallelMag, vPerpMag));
                    //DEBUG(("force: %f, %f", parallelForce, perpForce));
                    //DEBUG(("-------------------------------------"));

                    api.setForces(totalForce);
                }
                break;

    }

    
}

float accFactor() {
    float accFactor = 1;
    switch (game.getNumSPSHeld()) {
        case 1: accFactor *= (8.0 / 9.0);
        case 2: accFactor *= (4.0 / 5.0);
        default: accFactor *= 1;
    }
    for (int i = 0; i < 3; i++) {
        if (game.hasItem(i * 2) || game.hasItem((i * 2) + 1)) {
            accFactor *= ((i = 0) ? (8.0 / 11.0) : (i = 1) ? (4.0 / 5.0) : (8.0 / 9.0));
        }
    }
    return accFactor;
}

// Sets attitude toward a given point
void pointToward(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

float angleBetween(float vector1[3], float vector2[3]) {
    return acosf(mathVecInner(vector1, vector2, 3) / (mathVecMagnitude(vector1, 3) * mathVecMagnitude(vector2, 3)));
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
        if (closeTo(myPos, assemblyZone, avgDockingDist) && isFacing(assemblyZone, (3.14 / 8.0))) {
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
        float scale = (mathVecMagnitude(vectorBetween, 3) - maxDockingDist) / mathVecMagnitude(vectorBetween, 3);
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

// TODO: Add weight for stealing
// Return the itemID of the best item to dock with
int optimalItem()
{
    int maxPtsID = 0;
    float maxPts = -1;

    for (int itemID = 0; itemID < 6; itemID++) {
        // If the item is in our assembly zone, skip it
        while (game.itemInZone(itemID)) {
            itemID++;
        }
        if (itemID > 5) { break; }

        // If we're holding an item, return that item
        if (game.hasItem(itemID) == 1) { return itemID; }

        float itemDist[3]; // Vector between SPHERE and item
        float zoneDist[3]; // Vector between item and assembly zone

        // If opponent has the item, assume it's in their assembly zone
        if (game.hasItem(itemID) == 2) {
            float oppAss[3];
            for (int i = 0; i < 3; i++) { oppAss[i] = assemblyZone[i] * -1; }
            mathVecSubtract(itemDist, oppAss, myPos, 3);
            mathVecSubtract(zoneDist, assemblyZone, oppAss, 3);
        }  else {
            mathVecSubtract(itemDist, itemPos[itemID], myPos, 3);
            mathVecSubtract(zoneDist, assemblyZone, itemPos[itemID], 3);
        }

        float travelTime = mathVecMagnitude(itemDist, 3) + mathVecMagnitude(zoneDist, 3); // Replace this once we have an estimate for movement time

        float timeInZone = 180 - gameTime - travelTime;

        float itemPPS = (itemID < 2) ? 0.2 : ((itemID < 4) ? 0.15 : 0.1);

        if (itemPPS * timeInZone > maxPts) {
            maxPts = itemPPS * timeInZone;
            maxPtsID = itemID;
        }

    }

    return maxPtsID;
}