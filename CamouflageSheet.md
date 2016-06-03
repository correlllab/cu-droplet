# Pseudo-code of a single Droplet:
1. Item 1 Before each Frame -- 
Find out degree of each Droplet
2. Item 2 Each Frame , when it is its slot --
Send out message (the histogram array it stores, its degree, ) with sendHistMsg
_Length of Slot: 300ms_
3. Item 3 Each Frame , when it is not its slot --
Receive messages from other Droplets and store them with handle_msg(ir_msg* msg_struct)  
4. Item 4 At the end of the Frame --
Each droplet  computes a new histogram array by doing weighted average with the arrays and degrees it received during the loop and its own.
_Length of Idle: 300ms_

# Procedure of the Consensus on Droplets:
1. Item 1 Each droplet has a random RGB value and show the color
2. Item 2 Each droplet shares its RGB value with its neighbors at each round
3. Item 3 Define the graph, compute the degree of each droplet by counting the number of its neighbors
4. Item 4 Compute the histogram it gets from neighbors
5. Item 5 Show a different color with the information got


# Technical issues:
1. Item 1 Define the graph using the droplets’ positions/ just find out who are its neighbors? How?
2. Item 2 Share or get information from other droplets using which function? [Something like this: ir_send(ALL_DIRS, (char*)(&msg), sizeof(HistMsg));]


Note:
1. Item 1 **There is something wrong with the weights, why the first sometimes becomes negative**



