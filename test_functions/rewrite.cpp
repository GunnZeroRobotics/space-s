float myPos[3], myVel[3], myAtt[3];

float otherPos[3];

float itemPos[6][3]; // itemPos[itemID][x/y/z coordinate]
float assemblyZone[3]; // coordinates of assembly zone
float otherAss[3];
float assemblyError;
float spsLoc[2][3]; // locations of the last two SPSs (first one is at start)

float accMax, mass, fMax;
float fMaxSquared; // Variable used to reduce code size
float accFactor; // factor to multiply accMax by if holding items/SPSs, currently assumes we will not have items and SPSs at the same time

int rB; // -1 = red, 1 = blue

float firstItemAtt[3];

void init() {
    mass = 4.65; // 4.64968
    accMax = 0.008476;
    fMax = 0.039411;
    fMaxSquared = 0.001553226921;

    game.dropSPS();
    accFactor = 0.8;

    update();
    rB = (myPos[1] < 0) ? -1 : 1;
    spsLoc[0][0] = -0.5 * rB;
    spsLoc[0][1] = 0.27 * rB;
    spsLoc[0][2] = 0.05 * rB;

    firstItemAtt[0] = 0;
    firstItemAtt[1] = 0;
    firstItemAtt[2] = 0;
}

void loop() {
    update();

    int spsHeld = game.getNumSPSHeld();

    if (spsHeld == 2) {
        if (dist(myPos, spsLoc[0]) < 0.07) {
            game.dropSPS();
            accFactor = 0.7272727;
        } else {
            moveFast(spsLoc[0]);
        }
    } else if (spsHeld == 1) {
        float otherDistZero = dist(itemPos[0], otherPos);
        float otherDistOne = dist(itemPos[1], otherPos);
        float myDistZero = dist(itemPos[0], myPos);
        float myDistOne = dist(itemPos[1], myPos);

        if (((otherDistZero < otherDistOne && myDistZero < otherDistZero) || !(otherDistZero < otherDistOne || myDistOne < otherDistOne)) == (rB == -1)) {
            spsLoc[1][0] = -0.395 * rB;
            spsLoc[1][1] = -0.23 * rB;
            spsLoc[1][2] = -0.23 * rB;
            firstItemAtt[0] = 1 * rB;
        } else {
            spsLoc[1][0] = 0.22 * rB;
            spsLoc[1][1] = 0.384 * rB;
            spsLoc[1][2] = 0.22 * rB;
            firstItemAtt[1] = -1 * rB;
        }

        if (dist(myPos, spsLoc[1]) < 0.03) {
            game.dropSPS();
            accFactor = 1.0;

            // Get assembly zone location
            float aZ[4];
            game.getZone(aZ);
            assemblyError = aZ[3];
            for (int i = 0; i < 3; i++) {
                assemblyZone[i] = aZ[i];
                otherAss[i] = aZ[i] * -1;
            }
        } else {
            moveFast(spsLoc[1]);
            api.setAttitudeTarget(firstItemAtt);
        }
    } else {
        dock(optimalItem());
    }
}

// Math and Movement Methods

void dock(int itemID) {
    float vectorBetween[3]; // Vector between SPHERE and target (item or assembly zone)
    float targetPos[3]; // Target coordinate to move to
    float minDockingDist;
    float maxDockingDist;

    if (itemID < 2) {
        minDockingDist = 0.151;
        maxDockingDist = 0.173;
    }
    else if (itemID < 4) {
        minDockingDist = 0.138;
        maxDockingDist = 0.160;
    }
    else {
        minDockingDist = 0.124;
        maxDockingDist = 0.146;
    }

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
            moveFast(targetPos);
            pointToward(assemblyZone);
        }
    } else {
        mathVecSubtract(vectorBetween, itemPos[itemID], myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);

        // Scale targetPos to the item's position minus docking distance
        for (int i = 0; i < 3; i++) {
            targetPos[i] = (vectorBetween[i] * (dist - maxDockingDist) / dist) + myPos[i];
        }

        // Checks if SPHERE satisfies docking requirements -- if so, docks
        if (mathVecMagnitude(myVel, 3) > 0.01 || dist > maxDockingDist || !isFacing(itemPos[itemID], 0.25) || dist < minDockingDist) {
            moveFast(targetPos);
            pointToward(itemPos[itemID]);
        } else {
            if (game.dockItem(itemID)) {
                accFactor = (itemID < 2) ? 0.7272727 : ((itemID < 4) ? 0.8 : 0.8888889);
            }
        }
    }
}

void moveFast(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);

    float dist = mathVecMagnitude(vectorBetween, 3);

    if (dist < 0.01) {
        api.setPositionTarget(target);
    } else {
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
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        myAtt[i] = myState[i + 6];
        otherPos[i] = otherState[i];
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

// Strategy Methods

int optimalItem() {
    int maxPtsID = 0;
    float maxPts = -1;

    for (int itemID = 0; itemID < 6; itemID++) {

        // If we're holding an item, return that item
        if (game.hasItem(itemID) == 1) { return itemID; }

        // If the item is in our assembly zone, skip it
        if (game.itemInZone(itemID)) { continue; }

        float itemDist[3]; // Vector between SPHERE and item
        float zoneDist[3]; // Vector between item and assembly zone

        // If opponent has the item, assume it's in their assembly zone
        if (game.hasItem(itemID) == 2) {
            if (dist(otherAss, otherPos) < dist(otherAss, myPos)) {
                mathVecSubtract(itemDist, otherAss, myPos, 3);
                mathVecSubtract(zoneDist, assemblyZone, otherAss, 3);
            }
            else {
                continue;
            }
        }
        else {
            mathVecSubtract(itemDist, itemPos[itemID], myPos, 3);
            mathVecSubtract(zoneDist, assemblyZone, itemPos[itemID], 3);
        }

        float travelTime = mathVecMagnitude(itemDist, 3) + mathVecMagnitude(zoneDist, 3); // Replace this once we have an estimate for movement time

        float itemPPS = (itemID < 2) ? 0.2 : ((itemID < 4) ? 0.15 : 0.1);

        float timeInZone = 180 - api.getTime() - travelTime;

        float itemPoints = itemPPS * timeInZone;

        if (itemPoints > maxPts) {
            maxPts = itemPoints;
            maxPtsID = itemID;
        }
    }
    return maxPtsID;
}
