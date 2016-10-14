// TODO: Create a faster version of moving than setPositionTarget
//       using either setVelocity or setForce
//
// Last update:
// Could not develop a significantly faster version
// Ran into many problems, someone should revise/rewrite the code
// Feel free to delete anything/everything, currently does not work well
//  - Kevin Li

float myPos[3];
float myVel[3];
float myAtt[3];

float oppPos[3];

float itemPos[6][3];

float assemblyZone[3];

float spsLoc[2][3];

float t[3];

void init()
{
    t[0] = 0.75;
    t[1] = 0.75;
    t[2] = 0.75;
}

void loop()
{
    // Omitted satellite states, because I don't like them :)
    updateState();
    
    
    if (!closeTo(myPos, t, 0.01)) {
        moveFast(t);
    } else {
        game.dropSPS();
    }

}

// MARK: Helper Methods

void moveFast(float target[3]) {
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
 
    if (mathVecMagnitude(vectorBetween, 3) > 0.05) {
        api.setPositionTarget(target);
        return;
    }

    if (mathVecMagnitude(myVel, 3) < sqrtf(0.035 * (16.0 / 11.0) * mathVecMagnitude(vectorBetween, 3))) {
        mathScaleVect(vectorBetween, 1000);
        api.setVelocityTarget(vectorBetween);
    } else {
        mathScaleVect(vectorBetween, -1000);
        api.setVelocityTarget(vectorBetween);
    }
} 

void mathScaleVect(float orig[3], float factor) {
    for (int i = 0; i < 3; i++) {
        orig[i] = orig[i] * factor;
    }
}

bool closeTo(float vec[3], float target[3], float threshold)
{
    float diff[3];
    mathVecSubtract(diff, vec, target, 3);
    return mathVecMagnitude(diff, 3) < threshold;
}

void updateState()
{
    float myState[12];

    // SPHERE States
    api.getMyZRState(myState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
        myAtt[i] = myState[i + 6];
    }

    // Item Positions
    for (int i = 0; i < 6; i++)
    {
        float itemState[12];
        game.getItemZRState(itemState, i);
        for (int j = 0; j < 3; j++)
        {
            itemPos[i][j] = itemState[j];
        }
    }
}

void pointToward(float target[3])
{
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

bool isFacing(float target[3]) {
    float targetAtt[3];
    mathVecSubtract(targetAtt, target, myPos, 3);
    mathVecNormalize(targetAtt, 3);
    float theta;
    theta = acosf(mathVecInner(targetAtt, myAtt, 3));
    return theta < 14.5f;
}