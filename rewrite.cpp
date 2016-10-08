// PROBABLY INCORRECT: const static float acc = 0.121; // meters/second
const static int totalGameTime = 180; // seconds
const static float origin[3] = {0, 0, 0};

int gameTime;

float myState[12]; // our satellite state
float myPos[3];
float otherState[12]; // enemy satellite state
float otherPos[3];

float itemPos[6][3];

void init()
{
    // Initial item positions
    for (int i = 0; i < 6; i++) { 
        itemPos[(i < 3) ? 0 : 1][i % 3] = (i < 3) ? 0.23 : -0.23; // Large items
        itemPos[(i < 3) ? 2 : 3][i % 3] = (i % 2 == 0) ? 0.36 : -0.36; // Medium items
        itemPos[(i < 3) ? 4 : 5][i % 3] = (i < 1 || i > 3) ? 0.50 : -0.50; // Small items
    }
    
    gameTime = 0;
}

void loop()
{ 
   gameTime++; 
   updateState();
}

// MARK: Helper Methods

void updateState()
{
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        otherPos[i] = otherState[i];
    }
}

void pointToward(float target[3]) 
{
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}
