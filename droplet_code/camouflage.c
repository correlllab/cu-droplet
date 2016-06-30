/*
* camouflage.c
* For Camouflage Project
* Created: 5/25/2016, re-factored on 6/30/2016
* Author : Yang Li
* Description:
1. https://bitbucket.org/dominicverity/turing-pattern-generator
2. A local activator-inhibitor model of vertebrate skin patterns

Algorithmic Description:

INITIALIZATION:
Project one of the three patterns on Droplets

READ COLOR AND GRAPH PHASE:
Each frame, droplet senses the color on it, and store it
If its slot: Broadcasting to find out the four neighbors and store the information
If its slot: Share the neighbor information to have a bigger neighbor information 
	for Pattern formation
Last idle slot: figure out the big picture

GRADIENT PHASE:
Each frame, if its slot: broadcast to its neighbors where it is (distance and bearing) "my position"
Each frame, if not its slot: try to receive messages (color) from other Droplet, and store 
	them in four_Direction_Array
Each frame, during the last idle slot: Compute the gradients in each direction and update 
	the guess of pattern

CONSENSUS PHASE:
Each frame, if its slot: broadcast to its neighbors "my pattern"
Each frame, if not its slot: try to receive messages (guessed pattern and degree) from other 
	Droplet, and store them
Each frame, during the last idle slot: Try to make consensus on which the most possible 
	pattern to apply

TURING PATTERN PHASE:
__Generate a pattern based on Young's model__
Each frame, if its slot: broadcast it current "color"
Each frame, it not its slot: try to receive messages ("color") from droplets in the 
	two circles, and store them
Each frame, during the last idle slot: try to sum up the DCs and change the "color" accordingly


STRUCT:
Four neighbor struct and message  // FOR gradient
Eight neighbor struct // FOR consensus
Twelve neighbor struct // FOR formation
*/

#include "camouflage.h"

/*
* Any code in this function will be run once, when the robot starts.
*/
void init()
{

}

/*
* The code in this function will be called repeatedly, as fast as it can execute.
*/
/************************************************************************/
/*
*/
/************************************************************************/
void loop()
{

}

/*
* After each pass through loop(), the robot checks for all messages it has
* received, and calls this function once for each message.
*/
void handle_msg(ir_msg* msg_struct)
{

}


