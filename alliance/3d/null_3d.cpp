//Declare any variables shared between functions here

/**
 * Lit as af, as the hip kids say
 * @author Maxwell Salvadore, Alycia Lee, Thomas Lee
 * @author Kenny Daily, Steven Qiang, Daryia Roberts, Greg Orr
 */
 
float myState[12];
float posTarget[3];
float zoneInfo[4];
float attTarget[3];
bool stepThree;
bool firstTime;

float pieceLoc[12];

int pieceNum;
int side;

//int sideConst1; //probably wont be used
int sideConst2;


/**
 * Initializes variables
 */
void init() {
   api.getMyZRState(myState);
   side = sideCheck(myState[1]);
   stepThree = false;
   sideConst2 =1;
    firstTime = false;
    //This function is called once when your code is first loaded.
    //IMPORTANT: make sure to set any variables that need an initial value.
    //Do not assume variables will be set to 0 automatically!
    //DEBUG(("THIS PROGRAM IS BEING RUN"));
    setPos(side*0.7, side*0.7, 0); //goes to a hard-coded location to drop sps
    setAtt(side*0.47f, side*0.47f, side*-0.23f); //makes picking up large piece easier
    pieceLoc[0] = 0;
    pieceLoc[1] = 0;
    pieceLoc[2] = 0;
    pieceNum = 0;
    //attRateTarget[0] = 0.0;
    //attRateTarget[1] = 0.0;
    //attRateTarget[2] = 0.0;
   
}

 
/**
 * Function that determines what side you are on
 */
int sideCheck(float y)
{
    if (y > 0)
    {
        return 1; //blue side
    }
    else if (y < 0)
    {
        return -1; //red side
    }
}
 /**
 * Function that runs the program. Loops every second
 */
void loop() { 
    
    if(game.getNumSPSHeld() != 0){
        setSPS();
    }
    else if(stepThree && (game.itemInZone(2) || game.itemInZone(3))){
        stealPiece();
    }
    else if (game.getNumSPSHeld() == 0 && (game.hasItem(0)==1 || game.hasItem(1)==1 ) ){
        moveZone();
    }
    else if (game.getNumSPSHeld() == 0 && (game.itemInZone(0) || game.itemInZone(1))){
        getPieceTwo();
    }

}
/**
 * Checks to see if the satelite is within certain bounds
 * Returns true if so, false otherwise
 */
 
bool boundCheck(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax){
   if (xmin < xmax)
   {
        if ( myState[0] >= xmin && myState[0] <= xmax &&      //If the robot is at this pos
             myState[1] >= ymin && myState[1] <= ymax &&
             myState[2] >= zmin && myState[2] <= zmax )
        {
             return true;      
        }
        else
        {
            return false;
        }
   }
   else if (xmax < xmin)
   {
       if ( myState[0] >= xmax && myState[0] <= xmin &&      //If the robot is at this pos
             myState[1] >= ymax && myState[1] <= ymin &&     //this is to prevent stuff front not working when on red side
             myState[2] >= zmax && myState[2] <= zmin )
        {
             return true;      
        }
        else
        {
            return false;
        }
   }
 }
/**
 * Sets the values of posTarget[3]
 */
void setPos(float x, float y, float z){
    posTarget[0] = x;
    posTarget[1] = y;
    posTarget[2] = z;
}
/**
 * Sets the values of attTarget
 */
void setAtt(float x, float y, float z){
    attTarget[0] = x;
    attTarget[1] = y;
    attTarget[2] = z;
}
/**
 * Function used to move the robot in a triangle and make an SPS triangle
 */
void setSPS(){
    
    api.getMyZRState(myState);//THIS UPDATES MyState
    api.setAttitudeTarget(attTarget);
    if(game.getNumSPSHeld() == 3) {
        game.dropSPS();
    }
    
    
    else if(boundCheck(side*0.69,side*0.75,side*0.69,side*0.75,side*-0.2,side*0.2))
    {       //it drops the sps and changes the destination
        if(game.getNumSPSHeld() == 2){  //if only 1 SPS             
            game.dropSPS();             //has been dropped 
            setPos(side*0.32, side*0.32, side*0.2838);
            setAtt(side*(0.23-0.32f), side*(0.23-0.32f), side*(0.23-0.2838f));
            api.setAttitudeTarget(attTarget);
            
        }
    }
    
    else if(boundCheck(side* 0.30,  side*0.34,  side*0.30,  side*0.34,  side*0.2638,  side*0.3038))
    {
       if(game.getNumSPSHeld() == 1){  //if only 2 SPS             
            game.dropSPS();             //has been dropped  
        }
    }
     
     
     //getting the Large object and making sure bot is in bounds
    if (game.getNumSPSHeld() == 0 && sqrtf((myState[0]-side*0.23f)*(myState[0]-side*0.23f)+(myState[1]-side*0.23f)*(myState[1]-side*0.23f)+(myState[2]-side*0.23f)*(myState[2]-side*0.23f)) <= 0.16f)
    {
        game.dockItem();
        //DEBUG(("DOCK ATTEMPTED"));
    }
     
     
     
    //ACTIVATING POSITION TARGET
    api.setPositionTarget(posTarget);
} //despite it being made because of my habits of object orientation ( ͡° ͜ʖ ͡°)
  //this is actually functional programming
  //dont patronize me, Kenny

float disCalc(float x, float y , float z)
{
    return sqrtf(mathSquare(myState[0]-x)+mathSquare(myState[1]-y)+mathSquare(myState[2]-z));
}

/**
 * Moves the robot to estimated assembly zone 
 */
void moveZone(){
    api.getMyZRState(myState);
    
    game.getZone(zoneInfo); //fills zoneInfo with estimated area of assembly zone
                            //accuracy inversely proportional with SPS area
    
   
    setAtt(zoneInfo[0]-myState[0], zoneInfo[1]-myState[1], zoneInfo[2]-myState[2]);
    api.setAttitudeTarget(attTarget);
 
    setPos(zoneInfo[0], zoneInfo[1], zoneInfo[2]);
    api.setPositionTarget(posTarget);
    
    //if object is in bounds
    if (disCalc(zoneInfo[0], zoneInfo[1], zoneInfo[2]) <= zoneInfo[3]+0.1)
    {
        game.dropItem();
    }
    
}

void getPieceTwo(){
    api.getMyZRState(myState);
//-0.36, 0.36, -0.21 vs 0.36,-0.36,0.21
        if (!firstTime)
        {
            if (disCalc(0.36,-0.36,0.21) <= disCalc(-0.36,0.36,-0.21)) //If our position is closer to one medium piece, it goes to that and the other medium piece if it is closer
            {
                setPos(.36, -.36, .21);
                setAtt(0,0,0.5);
                pieceNum = 2;
                sideConst2 = 1;
            }
            else
            {
                setPos(-.36, .36, -.21);
                setAtt(0,0,-0.5);
                pieceNum = 3;
                sideConst2 = -1;
            }
            firstTime = true;
        }
        api.setPositionTarget(posTarget);
       // setAtt(.36-myState[0], -.36-myState[1], .36-myState[2]);
        
        api.setAttitudeTarget(attTarget);
        
            if(boundCheck(sideConst2*0.35,sideConst2*0.37,sideConst2*-0.37,sideConst2*-0.35,sideConst2*0.2,sideConst2*0.22)&& game.hasItem(pieceNum) !=1 
            && !game.itemInZone(pieceNum) && !game.hasItemBeenPickedUp(pieceNum))
           {
            
               //api.setAttitudeTarget(attTarget);
                game.dockItem();
                stepThree = true;
            }
        if (game.hasItem(pieceNum) ==1)
        {
            moveZone();
        }
}

void stealPiece(){
   
   if(side == 1)
   {
       //pieceNum = 0;
        if(game.hasItem(1) != 2)
        { //if yellow piece is in zone it goes to grab it
             grabPiece(1);
        }
    }        
    else if(side == -1)
    {
       //pieceNum = 1;
        if( game.hasItem(0) != 2)
        { //if yellow piece is in zone it goes to grab it
        
            grabPiece(0);
        }
    }
}

void grabPiece(int itemID){
       
    game.getItemZRState(pieceLoc, itemID); //sets destination to location of piece
    if (pieceLoc[2] > 0)
    {
        sideConst2 = 1;
    }
    else
    {
        sideConst2 = -1;
    }
    setPos(pieceLoc[0], pieceLoc[1], pieceLoc[2] - sideConst2*0.16);
  
    
    setAtt(0,0,sideConst2*0.5);
    api.setAttitudeTarget(attTarget);
    api.setPositionTarget(posTarget); //go to piece
    api.getMyZRState(myState);


    
    if(!game.hasItem(itemID) && boundCheck(posTarget[0] - .03, posTarget[0] + .03, posTarget[1] - .03, posTarget[1] + .03, posTarget[2] - .009,posTarget[2] + .009) && !game.itemInZone(itemID))
        { //BOUNDS SET INCORRECTLY, MAKE THEM CORRECT
               
        game.dockItem(); //if robot in right place pick up jawn
    }
    if (game.hasItem(itemID)){
        
        moveZone(); 
    } //goes back to zone
}