// PROBABLY INCORRECT: const static float acc = 0.121; // meters/second
const static int totalGameTime = 180; // seconds

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
   
//    float vectorBetween[3];
   if (gameTime < 30) { 
       float targetPoint[3] = {0.23, 0.23, 0.17};
        float targetPos[3] = {0.14, 0.14, 0.14};
       pointToward(targetPoint);
       api.setPositionTarget(targetPos);
   } else if (gameTime == 30) {
        game.dockItem();
        if (game.hasItemBeenPickedUp(0)) {
            DEBUG(("YAY"));
        } else {
            DEBUG(("NAY"));
        }
   }
//    mathVecSubtract(vectorBetween, itemPos[0], myPos, 3);

       
//    if (mathVecMagnitude(vectorBetween, 3) > 0.173) {
    //    api.setPositionTarget(itemPos[0]);
    //    pointToward(itemPos[0]);
//    } else {
    //    game.dockItem();
//    }
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
