const static float acc = 0.121; // meters/second
const static int totalGameTime = 180; // seconds

int gameTime;

float myState[12]; // our satellite state
float myPos[3];
float myVel[3];
float otherState[12]; // enemy satellite state
float otherPos[3];

float itemPos[6][3];

float assemblyZone[4];
float realAss[3];

int currentBehavior;
int targetitem;
float curveStartPos[3];
int framesThrusted;
int accel;
float distanceAcceled;

void init()
{
   currentBehavior = 0;
   gameTime = 0;

   framesThrusted = 0;
   accel = 0;

}

void loop()
{
   // update information on game state
   gameTime++;
   updateState();

   float p1[3] = {0.5, 0.3, 0};
   float p2[3] = {-0.39, -0.23, -0.23};

   curveStartPos[0] = 0.0;
   curveStartPos[1] = 0.15;
   curveStartPos[2] = 0.0;


   moveCurve(p1, p2);
}

void moveCurve(float p1[3], float p2[3])
{
   int numThrustFrames = 3;

   float majorDirection[3];
   mathVecSubtract(majorDirection, p1, curveStartPos, 3);



   if(accel == 0)
   {
       if(framesThrusted < numThrustFrames)
       {
           api.setForces(majorDirection);
       }

       if(framesThrusted == numThrustFrames)
       {
           float distanceTraveled[3];
           mathVecSubtract(distanceTraveled, myPos, curveStartPos, 3);
           distanceAcceled = mathVecMagnitude(distanceTraveled, 3);
           DEBUG(("x %f y %f z %f m %f", distanceTraveled[0], distanceTraveled[1], distanceTraveled[2], distanceAcceled));
           accel = 1;
       }

       DEBUG(("Accel is %d, thrustframes is %d, distAcceled %f", accel, framesThrusted, distanceAcceled));

       framesThrusted += 1;
   }
   if(accel == 1)
   {
       float distanceToTravel[3];
       mathVecSubtract(distanceToTravel, myPos, p1, 3);
       float magToTravel = mathVecMagnitude(distanceToTravel, 3);

       DEBUG(("Accel is %d, thrustframes is %d, toTravel %f, distAcceled %f", accel, framesThrusted, magToTravel, distanceAcceled));

       if(magToTravel < distanceAcceled)
       {
           // once we're in time to stop.
           for (int i = 0; i < 3; i++) { majorDirection[i] = (majorDirection[i] * -1); }
           api.setForces(majorDirection);
       }
   }


}

void updateState()
{
   // SPHERE States
   api.getMyZRState(myState);
   api.getOtherZRState(otherState);
   for (int i = 0; i < 3; i++) {
       myPos[i] = myState[i];
       myVel[i] = myState[i + 3];
       otherPos[i] = otherState[i];
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