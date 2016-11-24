float myPos[3], myVel[3], myAtt[3];

float otherPos[3];

float itemPos[7][3]; // itemPos[itemID][x/y/z coordinate]
float assemblyZone[3]; // coordinates of assembly zone

float assemblyError;
float spsLoc[2][3]; // locations of the last two SPSs (first one is at start)

float accMax, mass, fMax;
float fMaxSquared; // Variable used to reduce code size
float accFactor; // factor to multiply accMax by if holding items/SPSs, currently assumes we will not have items and SPSs at the same time

int rB; // -1 = red, 1 = blue

float firstItemAtt[3];

void init() {
    // Constants determined by testing
    mass = 4.65; // 4.64968 
    accMax = 0.008476; // Maximum Acceleration
    fMax = 0.039411; // Maximum Force
    fMaxSquared = 0.001553226921; 

    // Drop first SPS at start position and change acceleration factor because satellite is now holding 2 SPSs
    game.dropSPS(); 
    accFactor = 0.8; 

    // Get gameTime, SPHERE states, and item locations
    update();
    // Checks if we are blue or red and sets rB coefficient depending
    rB = (myPos[1] < 0) ? -1 : 1; 
}

void loop() {
    update(); 

    int spsHeld = game.getNumSPSHeld(); 

    // Drop second SPS if close enough to target location, or else keep moving towards target. 
    if (spsHeld == 2) { 
        if (!game.hasAdapter()) {
            float daLoc[3];
            game.getItemLoc(daLoc, 7);
            api.setPositionTarget(daLoc);
        } else if (game.hasAdapter()) {
            game.dropSPS();
        }
    } else if (spsHeld == 1) {
        if (game.hasItem(6) != 1) {
            dock(6);
        } else {
            float tempSPSLoc[3] = {0, 0, -0.5};
            if(dist(myPos, tempSPSLoc) < 0.03) {
                game.dropSPS();
                // Get assembly zone location
                float aZ[4];
                game.getZone(aZ);
                assemblyError = aZ[3];
                for (int i = 0; i < 3; i++) {
                    assemblyZone[i] = aZ[i];
                }
            } else {
                api.setPositionTarget(tempSPSLoc);
            }
        }
    } else {
        if (game.hasItem(6) == 1) {
            dock(6);
        } else {
            api.setPositionTarget(assemblyZone);
        }
    }
}

// Math and Movement Methods

void dock(int itemID) {
    float vectorBetween[3]; // Vector between SPHERE and target (item or assembly zone)
    float targetPos[3]; // Target coordinate to move to

    float minDockingDist = (itemID < 2) ? 0.151 : ((itemID < 4) ? 0.138 : 0.124);
    float maxDockingDist = (itemID < 2) ? 0.173 : ((itemID < 4) ? 0.160 : 0.146);

    // If you are holding the item, put it in your assembly zone
    if (game.hasItem(itemID) == 1) {
        mathVecSubtract(vectorBetween, assemblyZone, myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);

        float assemblyTolerance = 0.09 - assemblyError;
        if (dist < maxDockingDist + assemblyTolerance && dist > minDockingDist - assemblyTolerance && isFacing(assemblyZone, 0.392699)) {
            game.dropItem();
            accFactor = 1.0;
        } else {
            // Set position to assemblyZone's position scaled down by dockingDist
            for (int i = 0; i < 3; i++) {
                targetPos[i] = vectorBetween[i] * ((dist - minDockingDist) / dist) + myPos[i];
            }
            api.setPositionTarget(targetPos);
            pointToward(assemblyZone);
        }
    } else {
        DEBUG(("docking"));
        mathVecSubtract(vectorBetween, itemPos[itemID], myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);

        // Scale targetPos to the item's position minus docking distance
        for (int i = 0; i < 3; i++) {
            targetPos[i] = (vectorBetween[i] * (dist - maxDockingDist) / dist) + myPos[i];
        }

        // Checks if SPHERE satisfies docking requirements -- if so, docks
        if (mathVecMagnitude(myVel, 3) > 0.01 || dist > maxDockingDist || !isFacing(itemPos[itemID], 0.25) || dist < minDockingDist) {
            api.setPositionTarget(targetPos);
            pointToward(itemPos[itemID]);
        } else {
            if (game.dockItem(itemID)) {
                accFactor = (itemID < 2) ? 0.7272727 : ((itemID < 4) ? 0.8 : 0.8888889);
            }
        }
    }
}

// Closed-loop implementation of movement with the setForce function
void moveFast(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);

    float dist = mathVecMagnitude(vectorBetween, 3);

    // Use setPosition if close enough because it is more accurate
    if (dist < 0.01) {
        api.setPositionTarget(target);
    } else {
    // Use setForce
        float vMag = mathVecMagnitude(myVel, 3);
        float ang = angleBetween(vectorBetween, myVel); // Angle between velocity and vector between
        float vParallelMag = vMag * cosf(ang);
        float vPerpMag = vMag * sinf(ang);

        // Find a vector in the direction of the perpendicular velocity
        float vTemp[3];
        mathVecCross(vTemp, myVel, vectorBetween);
        float vPerp[3];
        mathVecCross(vPerp, vectorBetween, vTemp);

        // Normalize the vectors for scaling later
        mathVecNormalize(vectorBetween, 3);
        mathVecNormalize(vPerp, 3);

        float perpForce;
        float parallelForce = 0;

        // Most of this is derived from calculations using kinematic equations
        // The vector to the target is decomposed into parallel and perpendicular vectors
        // The force required to reach the target is then calculated for the two vectors using fMax 
        if (dist < ((vParallelMag * vParallelMag) / (2 * accMax * accFactor * 0.785))) {
            parallelForce = -0.9 * fMax;
            float temp = mass * (vPerpMag / 2); // Reduces code size
            if (temp < sqrtf(fMaxSquared - (parallelForce * parallelForce))){
                perpForce = temp;
            } else { 
                perpForce = sqrtf(fMaxSquared - (parallelForce * parallelForce));
            }
        } else {
            perpForce = mass * vPerpMag;
            if (perpForce < fMax) {
                parallelForce = sqrtf(fMaxSquared - (perpForce * perpForce));
            }

            if ((vParallelMag/dist > 0.17 && dist < 0.375) || vParallelMag > 0.06 || vParallelMag/dist > 0.19) {
                parallelForce = 0.0;
            } 
        }

        // Decomposed force vectors are combined into a single force vector 
        float totalForce[3];
        for (int i = 0; i < 3; i++) {
            totalForce[i] = (vPerp[i] * perpForce * -1) + (vectorBetween[i] * parallelForce);
        }

        // DEBUG(("dist: %f", dist));
        // DEBUG(("vel: %f, %f", vParallelMag, vPerpMag));
        // DEBUG(("force: %f, %f", parallelForce, perpForce));
        // DEBUG(("-------------------------------------"));

        api.setForces(totalForce);
    }
}

// Rotates SPHERE towards target vector
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
    return angleBetween(myAtt, targetAtt) < tolerance;
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3]) {
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}

// Returns the angle between two vectors
float angleBetween(float vector1[3], float vector2[3]) {
    return acosf(mathVecInner(vector1, vector2, 3) / (mathVecMagnitude(vector1, 3) * mathVecMagnitude(vector2, 3)));
}

// Updates gameTime, SPHERE states, and item locations
void update() {
    float myState[12];
    float otherState[12];

    // SPHERE States
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i]; // Position
        myVel[i] = myState[i + 3]; // Velocity
        myAtt[i] = myState[i + 6]; // Attitude 
        otherPos[i] = otherState[i]; // Enemy Position
        // otherVel[i] = otherState[i + 3];
        // otherAtt[i] = otherAtt[i + 6];
    }

    // Item Positions
    for (int i = 0; i < 7; i++) {
        float itemState[12];
        game.getItemZRState(itemState, i);
        for (int j = 0; j < 3; j++) {
            itemPos[i][j] = itemState[j];
        }
    }
}