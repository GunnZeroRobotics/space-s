int gameTime;

float myState[12]; // our satellite state
float myPos[3];
float myVel[3];
float otherState[12]; // enemy satellite state

float itemPos[6][3];

float assemblyZone[4];
float realAss[3];

void init()
{
    gameTime = 0;
}

void loop()
{ 
   gameTime++; 
   updateState();

   if (gameTime < 20) {
       float temp[3] = {-0.5, 0, 0};
       api.setPositionTarget(temp);
   } else if (gameTime == 20) {
       game.dropSPS();
   } else if (gameTime > 20 && gameTime < 40) {
       float temp[3] = {-0.5, 0.5, 0.75};
		api.setPositionTarget(temp);
   } else if (gameTime == 40) {
		game.dropSPS(); 
	} else if (gameTime > 40 && gameTime < 65) {
		float temp[3] = {0.13, 0.13, 0.13};
		api.setPositionTarget(temp); 
	} else if (gameTime == 65) {
        game.dropSPS();
        game.getZone(assemblyZone);
        DEBUG(("%f, %f, %f, %f", assemblyZone[0], assemblyZone[1], assemblyZone[2], assemblyZone[3]));
        for (int i = 0; i < 3; i++) {
            realAss[i] = assemblyZone[i] * 0.85;
        }
    } else {
        int IDcount = 0;
        while(game.hasItemBeenPickedUp(IDcount) && game.hasItem(IDcount) != 1) {
            IDcount++;
            if (IDcount > 5) {
                IDcount = 0;
                break;
            }
        }
        dock(IDcount);
    }
 
}

// MARK: Helper Methods

void updateState()
{
    // SPHERE States
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);
    for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myVel[i] = myState[i + 3];
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

void pointToward(float target[3]) 
{
    float vectorBetween[3];
    mathVecSubtract(vectorBetween, target, myPos, 3);
    mathVecNormalize(vectorBetween, 3);
    api.setAttitudeTarget(vectorBetween);
}

void dock(int itemID)
{
    if (game.hasItem(itemID) == 1) {
        float vb[3];
        mathVecSubtract(vb, realAss, myPos, 3);
        if (mathVecMagnitude(vb, 3) < 0.03) {
            game.dropItem();
        } else {
           api.setPositionTarget(realAss);
            pointToward(realAss);
        }
        return;
    }

    float vectorBetween[3];

    float vectorTarget[3];
    for (int i = 0; i < 3; i++) {
        vectorTarget[i] = itemPos[itemID][i];
    }
    mathVecSubtract(vectorBetween, itemPos[itemID], myPos,3);
    float dockingDist = (itemID < 2) ? 0.162 : ((itemID < 4) ? 0.149 : 0.135);
    float mProportion = (mathVecMagnitude(vectorBetween, 3) - dockingDist * 0.9) / mathVecMagnitude(vectorTarget, 3);
    for (int i = 0; i < 3; i++) {
        vectorBetween[i] = vectorBetween[i] * mProportion;
        vectorTarget[i] = vectorBetween[i] + myPos[i];
        vectorBetween[i] = vectorBetween[i] / mProportion;
    }
    
    if (mathVecMagnitude(myVel, 3) > 0.01 || mathVecMagnitude(vectorBetween, 3) > dockingDist) {
        api.setPositionTarget(vectorTarget);
        pointToward(itemPos[itemID]);
    } else if (game.hasItem(itemID) != 1){
        game.dockItem();
    }
}
