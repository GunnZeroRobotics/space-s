int game_time; 

float my_state[3]; //our sat position
float other_state[3]; //enemy sat position
float items[6][3]; //0-1 Large, 2-3 Medium, 4-5 Small
float target[3]; //target locations for moveTo function
float origin[3]; //origin of field, move to here in ENDGAME phase
float spsLoc[3]; //SPS target locations 

enum State {
	SPS, //starting game phase, drops SPS items
	ITEM, //
	ASSEMBLY, //
	STEAL, //
	ENDGAME, //uses all remaining fuel to move to origin

}
State state;

void init()
{
	game_time = 0;
	origin[0] = origin[1] = origin[2] = 0; 
	state = SPS;
	game.dropSPS();
	spsLoc[0] = 0.1;
	spsLoc[1] = -0.3;
	spsLoc[2] = 0;
}

void loop()
{ 
	updateState();
	game_time++;

	switch (state) { 
	case SPS: //SPS phase
		switch (game.getNumSPSHeld()) {
			case 2: //2 SPSs left
				moveTo(spsLoc);
				game.dropSPS();
				spsLoc[0] = 0.6;
				spsLoc[1] = 0.5;
				spsLoc[2] = 0;
				break;
			case 1: //1 SPS left 
				moveTo(spsLoc);
				break;
		}
		break;

	case ITEM: //Item pickup phas
		itemLoop();
		break;

	case ASSEMBLY:
		assemblyLoop();
		break;

	case STEAL:
		
		break;

	case ENDGAME:

		break;
	}
}

void updateState()
{
	getSphereStates();

	for (i = 0; i < 6; ++i) {
        game.getItemLoc(items[i], i);
    }

}

void getSphereStates()
{
	api.getMyZRState(my_state);
	api.getOtherZRState(other_state);
}

void moveTo(float target[])
{

}
