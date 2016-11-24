//#define DECISION_MESSAGES
//#define DEBUG_MESSAGES
//#define ZONE_MESSAGES
//#define MOV_DEBUG_MESSAGES
//#define MOV_DEBUG_MESSAGES2

#define  tol  0.05f
#define  MIN_ANGLE  0.79f
#define  MINIM_FUEL    10.0f
#define  VEC_SIZE    12
#define  DROP_POINT   0.5f

#define BLUE    0
#define RED     1


//Declare any variables shared between functions here
short               TIME, step, color, selection_time,sps_time, id0;
bool                wrong_heading;

float               destination[3], initial_point[3], zone[4], *cpos, *cvel, *catt, /**crot,*/ *oatt,
                    o_distance, velocity, fuel,
                    oponent_direction[3], State[12], oState[12], *opos, *ovel,
                    desired_attitude[3], distance_from_destination,
                    desired_velocity, heading[3], ovelocity;

float initPos[3];

void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	step = 0;
	distance_from_destination = 1.0f;
	selection_time = 0;
	zone[0]=0; zone[1]=0; zone[2]=0; zone[3]=0;
	id0=-1;
	DEBUG(("-------Greetings from ZANNEIO Stardust v02.G------"));

	api.getMyZRState(State);

    initPos[0] = State[0];
    initPos[1] = State[1];
    initPos[2] = State[2];
}



void loop(){
	//This function is called once per second.  Use it to control the satellite.
	
	float           r, flocal, fvector[3], destination2[3];
    short           i, collision;
	
	
	TIME = game.getCurrentTime();
	//Read state of opponent's Sphere
	api.getOtherZRState(oState);
	opos = oState;          //opponent's position
	ovel = &oState[3];      //oponent's velocity
	oatt = &oState[6];      //opponent's attitude
	ovelocity = mathVecMagnitude(ovel,3);
	
	//Read state of our Sphere
	api.getMyZRState(State);
	//copy information to vectors
	cpos = State;               //our position
	cvel = &State[3];           //our velocity
	catt = &State[6];           //attitude (the direction that our camera faces)
	//crot = &State[9];           //rotation vector
	if (TIME<2) //Red: 1  Blue:  0
	    color = cpos[1]>0  ?  BLUE  :  RED;
	
	//rot_vel = mathVecMagnitude(crot,3);             //rotational velocity
	velocity = mathVecMagnitude(cvel,3);            //velocity
	fuel = game.getFuelRemaining();             //fuel 
	
    
    
	
    //Messages (printed only if DEBUG_MESSAGES is defined)
	#ifdef DEBUG_MESSAGES
	DEBUG(("TIME: %d, Step: %d.\n", TIME, step));
	#endif

    
    r = 0;
    selection_time++;
    
    
    game.getZone(zone);
    //DEBUG(("%f, %f, %f, %f", zone[0], zone[1], zone[2], zone[3]));
    //flocal = sqrtf((zone[0]+0.31)*(zone[0]+0.31) + (zone[1]+0.38)*(zone[1]+0.38) + (zone[2]+0.33)*(zone[2]+0.33));
    //DEBUG(("%f", flocal));
    //desired_velocity = (step<=3) ? 0.07f : 0.06f;
    desired_velocity = (((fuel<MINIM_FUEL)&(step!=7))|(TIME>135)) ? 0.04f : 0.07f;
    collision = 0;
    //Future positions
    for (i=0;i<=10;i++){
        LinearComb(fvector,cpos,cvel,1,(float)i);
        LinearComb(destination2,opos,ovel,1,(float)i);
        mathVecSubtract(fvector,fvector,destination2,3);
        if ((mathVecMagnitude(fvector,3)<0.35f)){
            collision++;
        }
    }
    #ifdef DEBUG_MESSAGES
    if (collision>0) DEBUG(("COLLISION"));
    #endif
    
    //-----------------------------------------------------------------
	//Strategic Steps (step is initialized at 0)
	
    destination[0] = 0.7;
    destination[1] = 0.7;
    destination[2] = 0.7;
	
    if (dist(cpos, destination) < .022 && mathVecMagnitude(cvel, 3) < 0.01) {
        DEBUG(("dist: %f, time: %d, fuel: %f", (dist(destination, initPos)), api.getTime(), ((60-game.getFuelRemaining())/60)));
    }
    r = 0.03;

    //--------------------------------------------------------
	//moving stuff
    mathVecSubtract(heading, destination, cpos, 3);
    distance_from_destination = mathVecNormalize(heading, 3);
    
    distance_from_destination -=  r;
    LinearComb(destination2, cpos,  heading, 1.0f, distance_from_destination );
    distance_from_destination = distance_from_destination>0 ?
        distance_from_destination : -distance_from_destination;
    
    mathVecSubtract(fvector,destination2,cpos,3);
    distance_from_destination = mathVecNormalize(fvector,3);
    
    //0.40; 0.13
    flocal = (distance_from_destination>0.40f) ? desired_velocity : 0.16f*distance_from_destination;
    if ((collision)&(step<=3))
        flocal = flocal*0.80f;
    flocal = (flocal<0.010f) ? 0.010f : flocal;
    //if ((distance>0.10f)&(velocity<0.2f))
    //    flocal = 0.02f;
    
    fvector[0] = flocal * fvector[0];
    fvector[1] = flocal * fvector[1];
    fvector[2] = flocal * fvector[2];
    
    wrong_heading = mathVecInner(cvel,heading,3)/velocity < 0.98f ? true : false;
    #ifdef MOV_DEBUG_MESSAGES
    DEBUG(("Velocity = %f, desired velocity = %f", velocity, flocal));
    #endif
    
    if (distance_from_destination>0.05f){
        if ((fabs(velocity - flocal) > 0.005f)|(wrong_heading)|(step==2)){
            #ifdef MOV_DEBUG_MESSAGES
            DEBUG(("VELOCITY CONTROL : vel = %f", velocity));
            #endif
            api.setVelocityTarget(fvector);
        }
    }
    else
        api.setPositionTarget(destination2);
        
    #ifdef MOV_DEBUG_MESSAGES
    DEBUG(("GOING TO %f  %f  %f dist = %f", destination2[0], destination2[1], destination2[2], distance_from_destination));
    #endif
	
    
    
    if ((step>3)&(step!=9)){
        /*if (velocity>0.015f){
            //Set attitude according to future position
            mathVecSubtract(fvector, destination, cpos, 3);
            LinearComb(fvector, fvector, cvel, 1.0f, -1.0f);
            mathVecNormalize(fvector,3);
            api.setAttitudeTarget(fvector);
        }
        else*/
            //Set attitude according to current position
            api.setAttitudeTarget(heading);
    }
    
}





//------------------------------------------------------
//rescales a vector a --> l*a
void    vrescale(float *vector, float local)
{
    vector[0] *=local;
    vector[1] *=local;
    vector[2] *=local;
}


//--------------------------------------------------
//Stabilization function - Hard breaks
void  HardStabilize(){
    float    force[3];
    
    memcpy(force, cvel, VEC_SIZE);
    vrescale(force, -4.0f);
    api.setForces(force);
    #ifdef MOV_DEBUG_MESSAGES
    DEBUG(("FRENOOOOOOOOOOOOOOOOOOOOOOOOO"));
    #endif
}


bool InsideMyZone(){
    float   fvector[3], flocal;
    
    LinearComb(fvector, cpos, catt, 1.0f, 0.10f);
    mathVecSubtract(fvector,zone,fvector,3);
    flocal = mathVecMagnitude(fvector,3);
    if (mathVecInner(catt,heading,3)>0.96f)
        if (flocal<=0.08f){
            LinearComb(fvector, cpos, catt, 1.0f, 0.17f);
            mathVecSubtract(fvector,zone,fvector,3);
            flocal = mathVecMagnitude(fvector,3);
            if(flocal<=0.08f){
                return true;
            }
        }
    return false;
}


bool EnemyZoneHasItems()
{
    short   i;
    float   temp[3];
    
    for (i=0;i<=5;i++){
        game.getItemLoc(temp, i);
        mathVecAdd(temp,temp,zone,3);
        //if ((mathVecMagnitude(temp,3)<0.14)&(game.hasItemBeenPickedUp(i))&(game.hasItem(i)==0))
        if ( (mathVecMagnitude(temp,3)<0.14)&(game.hasItem(i)==0))    
            return true;
    }
    return false;
}


bool    CheckDockingConditions(){
    if ((selection_time>2)&(distance_from_destination<=0.01f)
                    &(velocity<0.01f)&(mathVecInner(catt,heading,3)>0.967f)) 
        if (game.dockItem(id0))
            return  true;  
    return false;
}

short    ClosestItem(float  *vector, short id1, short id2){
    float   temp[3], dmin=10.0f, d;
    short   i, imin=-1;
    
    for (i=id1;i<=id2;i++){
        game.getItemLoc(vector, i);
        if ((game.hasItem(i)==0)&((!game.itemInZone(i))|(!game.hasItemBeenPickedUp(i)))){
            //DEBUG(("Item = %d", i));
            mathVecSubtract(temp, cpos, vector, 3);
            d = mathVecMagnitude(temp,3);
            if (d<dmin){
                dmin = d;
                imin = i;
            }
        }
    }
    game.getItemLoc(vector, imin);
    #ifdef DEBUG_MESSAGES
	DEBUG(("Item Picked %d.\n", imin));
	#endif
	return imin;
}



void   LinearComb(float *r, float  *a, float *b,  float x, float y){
    short i;
    
    for(i=0;i<3;i++)
        r[i] = a[i]*x + b[i]*y;
}

// Returns the magnitude of the difference of two vectors
float dist(float a[3], float b[3])
{
    float diff[3];
    mathVecSubtract(diff, a, b, 3);
    return mathVecMagnitude(diff, 3);
}
