float targ[3];
float corner[3];

float myState[12], myPos[3], vectorBetween[3], myVel[3];

void init() {
    targ[0] = -0.7;
    targ[1] = 0.7;
    targ[2] = 0.7;

    corner[0] = 0.3;
    corner[1] = 0.7;
    corner[2] = 0.7;
}

void loop() {
    float a[3];
    a[0] = 1;
    a[1] = 0;
    a[2] = 0;
    float b[3];
    b[0] = -1;
    b[1] = 0;
    b[2] = 0;

    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
    }

    if (api.getTime() < 40) {
        api.setPositionTarget(corner);
    } else {
        // if (myPos[0] < 0.05) {
        //     api.setVelocityTarget(a);
        // } else {
        //     api.setVelocityTarget(b);
        // }
        mathVecSubtract(vectorBetween, targ, myPos, 3);
        float dist = mathVecMagnitude(vectorBetween, 3);
        if (dist < 0.01) {
            api.setPositionTarget(targ);
        } else {
            mathVecNormalize(vectorBetween, 3);
            if (mathVecMagnitude(myVel, 3) / dist > 0.163) {
                for (int i = 0; i < 3; i++) {vectorBetween[i] *= -1;}
            }
            api.setVelocityTarget(vectorBetween);
        }
    }
}