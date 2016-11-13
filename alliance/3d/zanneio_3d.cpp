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
	switch(step){
	
            case 0: //Drop first SPS and set heading for the second one
                game.dropSPS();
                memcpy((void *)initial_point, (void*)cpos, VEC_SIZE);
                
                //error = 0.083  Score = 1.12  (35-36 sec)
                destination[0] = color==BLUE ? -0.45f :  0.45f;
                destination[1] = color==BLUE ?  0.34f : -0.34f;
                destination[2] = color==BLUE ? 0.0f :  0.0f;
                
                //We believe that if we used these point we would have been #1
                //error = 0.080  Score = 1.22  (35-36 sec)
                //destination[0] = color==BLUE ? -0.38f :  0.38f;
                //destination[1] = color==BLUE ?  0.35f : -0.35f;
                //destination[2] = color==BLUE ? 0.0f :  0.0f;
                
                step=1;
            break;
            
            case 1: //Drop second SPS and set heading for the third one
                i = color==BLUE ?  1 : 0;
                fvector[0] = color==BLUE ?  -0.230f  :  0.230f;
                fvector[1] = color==BLUE ?  -0.230f  :  0.230f;
                fvector[2] = color==BLUE ?  -0.230f  :  0.230f;
                mathVecSubtract(fvector,fvector,opos,3);
                flocal = mathVecNormalize(fvector,3);
                //Determine the best possible item to grab
                if ((game.hasItem(i)!=0)|(mathVecInner(fvector,ovel,3)/ovelocity>0.98f)|(flocal<0.3f)){
                    //Go to the nearest yellow item
                    if (fabs(cpos[0])>0.34f){
                        game.dropSPS();
                        step = 3;
                        destination[0] = color==BLUE ?  0.230f  :  -0.230f;
                        destination[1] = color==BLUE ?  0.385f  :  -0.385f;
                        destination[2] = color==BLUE ?  0.230f  :  -0.230f;
                        selection_time = 0;
                    }
                }
                else{
                    //Go to the furthest yellow item
                    if (distance_from_destination<=tol){
                        game.dropSPS();
                        destination[0] = color==BLUE ?  -0.375f  :  0.375f;
                        destination[1] = color==BLUE ?  -0.230f  :  0.230f;
                        destination[2] = color==BLUE ?  -0.230f  :  0.230f;
                        step=2;
                        selection_time = 0;
                    }
                }

            break;
            
            
            
            case 2://Go near the Furthest Large Item
                if (distance_from_destination < 0.08){  //0.25f
                    step = 4; 
                    goto FIRST_LARGE;
                }
                
            break;
            
            
            case 3: //Go near the nearest Item
                selection_time++;
                fvector[0] = 0.0f;
                fvector[1] = color==BLUE ? -1.0f : 1.0f;
                fvector[2] = 0.0f;
                api.setAttitudeTarget(fvector);
                if (distance_from_destination < 0.08f){ 
                    step = 4; 
                    goto FIRST_LARGE;
                }
            break;
            
            FIRST_LARGE:
            case 4: // Furthest Large Item Selection
                selection_time++;
                if (game.hasItem(id0)==2){//we lost the item
                    game.dropSPS();
                }
                //if there is a free large item go for it
                id0 = ClosestItem(destination, 0, 1);
                if (id0>=0){
                    r = 0.165f;
                }
                else{//otherwise go for the nearest item
                    step=5;
                    goto ITEM_SELECTION;
                }
                
                if (CheckDockingConditions()){
                    step = 6;
                    game.dropSPS();
                    goto MY_ZONE;
                }
            break;
            
            
            ITEM_SELECTION:
            case 5: //Nearest Item Selection
                selection_time++;
                id0 = ClosestItem(destination, 0, 5);
                
                r = (id0<2) ?  0.165f : (id0<4) ? 0.152f : 0.136f;
                
                if (fuel<MINIM_FUEL)
                    step = 9;
                else{
                    if ((distance_from_destination>0.25f)|(TIME>100)){
                    //if there isn't a nearby item go to enemy zone
                            step = 10;
                            goto ENEMY_ZONE;
                    }
                    else{//go to nearby item
                        if (CheckDockingConditions()){
                            step = 6;
                        }
                    }
                }
            break;
            
            
            MY_ZONE:
            case 6: //Set heading to my zone
                memcpy((void *)destination, (void*)zone, VEC_SIZE);
                mathVecSubtract(fvector,cpos,zone,3);
                flocal = mathVecMagnitude(fvector,3);
                if (flocal<0.15f)
                    r = 0.2f;
                else{
                    step = 7;
                    r = 0.10f;
                }
            break;
            
            
            case 7: //Drop Item Inside Zone
                //set up destination
                r = 0.10f;
                memcpy((void *)destination, (void*)zone, VEC_SIZE);

                //dropping conditions
                if (InsideMyZone()){
                    game.dropItem();
                    //Decide where to go
                    step = 5;
                    selection_time = 0;
                    goto ITEM_SELECTION;
                }
            break;
            
            
            case 9: //Guard our zone
                memcpy((void *)destination, (void*)zone, VEC_SIZE);
                r=0;
            break;
            
            
            ENEMY_ZONE:
            case 10: //Set heading to enemy's Zone
                memcpy((void *)destination, (void*)zone, VEC_SIZE);
                vrescale(destination,-1.0f);
                step = 12;
            break;
            
            
            case 12: //Go to a point near the enemy's Zone
                r = 0.30f;
                memcpy((void *)destination, (void*)zone, VEC_SIZE);
                vrescale(destination,-1.0f);
                mathVecAdd(fvector,opos,zone,3);
                flocal = mathVecMagnitude(fvector,3);
                if ((distance_from_destination<=0.4f)&(flocal>0.16f)){
                    //If enemy zone contains items
                    if (EnemyZoneHasItems()){
                        step = 14;
                        selection_time = 0;
                        goto GET_ENEMY_ITEMS;
                    }
                }
            break;
            
            
            GET_ENEMY_ITEMS:
            case 14: //Get opponent items
                selection_time++;
                //find closest larger item
                id0 = ClosestItem(destination, 0, 1);
                if (id0<0){//if there isn't any large item available go for another
                    id0 = ClosestItem(destination, 0, 5);
                }
                
                r = (id0<2) ?  0.165f : (id0<4) ? 0.152f : 0.136f;
                //Dock
                if (CheckDockingConditions()){
                    step = 6;
                    goto MY_ZONE;
                }
  
            break;
	}
	
	
	
	
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