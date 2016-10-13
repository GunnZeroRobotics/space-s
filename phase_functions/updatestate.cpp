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

	if (game.getNumSPSHeld() > 0)
	{
		state = SPS;
	}
	else if (game.hasItem(IDcount))
	{
		state = ITEM;
	}
	else 
	{
		state = ASSEMBLY;
	}
}