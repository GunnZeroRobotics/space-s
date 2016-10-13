//
// UPDATE: This has already been finalized and resulting values stored 
// KEPT ONLY FOR REFERENCE
//
// Simulate with:
// Satellite 1: X = 0, Y = 0, Z = 0, AttX = any, AttY = any, AttZ = any
// Satellite 2: X = 0.75, Y = 0.75, Z = 0.75, AttX = any, AttY = any, AttZ = any
//
// Testing Results:
// Equation relating area of triangle formed by SPSs and radius of error is approximately
// A * R = 0.0095
// 
// Testing Data:
// TRIANGLE AREA | RADIUS OF ERROR
// 0.02          | 0.4598
// 0.045         | 0.2124
// 0.125         | 0.07744 

int gameTime;

float myState[12]; // our satellite state
float otherState[12]; // opponent satellite state
float items[6][3]; // 0-1 Large, 2-3 Medium, 4-5 Small
float origin[3]; 
float assemblyZone[4];

void init()
{
    gameTime = 0;
	game.dropSPS();
}

void loop()
{ 
	gameTime++; 
	updateState();

	if (gameTime < 40) {
		float temp[3] = {0, 0.7, 0};
		api.setPositionTarget(temp);
	} else if (gameTime == 40) {
		game.dropSPS(); 
	} else if (gameTime > 40 && gameTime < 80) {
		float temp[3] = {0, 0, 0.7};
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
}
