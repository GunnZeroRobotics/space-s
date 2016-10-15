/* Code for optimal SPS placement such that
 error radius is <= .1 (according to equation A * R = .0095)
 Minimum area is .095 or larger
 */

const static int totalGameTime = 180; // seconds

// float eqConst = 0.0095; //constant in equation
// float radius = .1; //desired radius
// float area = eqConst/radius; //corresponding area
//float side = area / (1.73205081/4);

int gameTime;
int rB;

float myState[12]; // our satellite state
float otherState[12]; // opponent satellite state
float items[6][3]; // 0-1 Large, 2-3 Medium, 4-5 Small
float spsLoc[3][3]; // SPS coordinates
float assemblyZone[4];

void init()
{
    gameTime = 0;
    api.getMyZRState(myState);
    rB = (myState[1] < 0) ? -1 : 1;
}

void loop()
{
    gameTime++;
    updateState();
    
    if (gameTime < 20){
        float temp[3] = {0, .15 * rB, 0};
        memcpy(spsLoc[0], temp, sizeof temp);
        api.setPositionTarget(temp);
    } else if (gameTime == 20){
        game.dropSPS();
        DEBUG(("%f, %f, %f", myState[0], myState[1], myState[2]));
    } else if (gameTime < 40) {
        float temp[3] = {-.5 * rB, .4 * rB, .1 * rB};
        memcpy(spsLoc[1], temp, sizeof temp);
        api.setPositionTarget(temp);
    } else if (gameTime == 40) {
        game.dropSPS();
        DEBUG(("%f, %f, %f", myState[0], myState[1], myState[2]));
    } else if (gameTime < 60) {
        float temp[3] = {-.35 * rB, -.3 * rB, -.22 * rB};
        memcpy(spsLoc[2], temp, sizeof temp);
        api.setPositionTarget(temp);
    } else if (gameTime == 60) {
        game.dropSPS();
        DEBUG(("%f, %f, %f", myState[0], myState[1], myState[2]));
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
