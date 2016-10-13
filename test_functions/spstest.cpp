const static float acc = 0.121; // meters/second
const static int totalGameTime = 180; // seconds

int gameTime;

float myState[12]; // our satellite state
float otherState[12]; // opponent satellite state
float myPos[3]; // our satellite state
float otherPos[3]; // opponent satellite state
float items[6][3]; // 0-1 Large, 2-3 Medium, 4-5 Small
float spsLoc[3][3];
float origin[3]; 
float assemblyZone[4];

void init()
{
    gameTime = 0;
	game.dropSPS();
    spsLoc[0][0] = 0;
    spsLoc[0][1] = 0.15;
    spsLoc[0][2] = 0;
}

void loop()
{ 
	gameTime++; 
	updateState();

	if (gameTime < 40) {
		float temp[3] = {-0.18, 0.1, -0.18};
        spsLoc[1] = temp;
		api.setPositionTarget(temp);
	} else if (gameTime == 40) {
		game.dropSPS(); 
	} else if (gameTime > 40 && gameTime < 80) {
		float temp[3] = {-0.36, 0.36, -0.36};
        spsLoc[2] = temp;
		api.setPositionTarget(temp); 
	} else if (gameTime == 80) {
        game.dropSPS();
        game.getZone(assemblyZone);
        DEBUG(("%f, %f, %f, %f", assemblyZone[0], assemblyZone[1], assemblyZone[2], assemblyZone[3]));
    } else {
        float temp[3] = {assemblyZone[0], assemblyZone[1], assemblyZone[2]};
        api.setPositionTarget(temp);
    }
    
}

void updateState()
{
	api.getMyZRState(myState);
	api.getOtherZRState(otherState);
    for (int i = 0; i < 3; i++)
    {
        myPos[i] = myState[i];
        otherPos[i] = otherState[i];
    }
}
