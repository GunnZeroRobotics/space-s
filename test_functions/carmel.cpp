void dock(int itemID) {
    
    
    //game.getItemLoc(itemLoc, itemID);
    moveTo(items[0]);
    api.setAttitudeTarget(items[0]);
}

void itemLoop()
{
	float speed[3];
	float vMag = pow(my_state[3], 2) + pow(my_state[4], 2) + pow(my_state[5], 2);
	vMag = sqrt(vMag);
	float vMagTarget = vMag / .009; //getting the magnitude of vector to be just under .01
	speed[0] = my_state[3] / vMagTarget;
	speed[1] = my_state[4] / vMagTarget;
	speed[2] = my_state[5] / vMagTarget;
	
	if(computeDistance(my_state, items[0]) < 0.2) {
	    api.setVelocityTarget(speed);
	}
	
	if(computeDistance(my_state, items[0]) < 0.1) {
	    game.dockItem(0);
	}
	// api.getMyZRState
}