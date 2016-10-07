const static float acc = 0.121; // meters/second
const static int totalGameTime = 180; // seconds

int gameTime;

float myState[12]; // our satellite state
float otherState[12]; // enemy satellite state
float items[6][3]; // 0-1 Large, 2-3 Medium, 4-5 Small
float origin[3]; 

void init()
{
    
}

void loop()
{ 
   gameTime++; 
   updateState();
}

void updateState()
{
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);
}