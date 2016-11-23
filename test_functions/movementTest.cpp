float targ[3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

float initPos[3];

bool fail;
bool success;

void init() {
    targ[0] = -0.7;
    targ[1] = 0.7;
    targ[2] = 0.7;

    corner[0] = -0.5;
    corner[1] = 0.7;
    corner[2] = 0.7;

    fail = false;
    success = false;
}

void loop() {

    float k = 0.25;

    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (api.getTime() < 40) {
        api.setPositionTarget(corner);
    } else if (api.getTime() == 40) {
        for (int i = 0; i < 3; i++) { initPos[i] = myPos[i]; }
    } else {
        mathVecSubtract(vectorBetween, targ, myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);
        if (dist < 0.01) {
            api.setPositionTarget(targ);
        } else {
            for (int i = 0; i < 3; i++) {vectorBetween[i] *= k;}
            api.setVelocityTarget(vectorBetween);
        }
    }

    if ((fail || (myPos[0] < (targ[0] - 0.023))) && !success) {
        fail = true;
        DEBUG(("FAIL"));
        return;
    }

    if (dist(myPos, targ) < 0.01 && mathVecMagnitude(myVel, 3) < 0.01 && !fail && !success) {
        success = true;
        float d = dist(initPos, targ);
        DEBUG(("d: %f, k: %f", d, k));
    }
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3]) {
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}
