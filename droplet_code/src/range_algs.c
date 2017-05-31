/*
*	README:
*	For consistencies sake, any time you loop through the brightness matrix, it should look like this:
*		for(emitter){
*			for(sensor){
*				brightness_matrix[emitter][sensor] ...
*			}
*		}
*	There were previous inconsistencies in this code.
*/
#include "range_algs.h"

//This is based on the time that elapses between when a RXing Droplet gets the end
// of a message sent from dir N, and when the TXing droplet finishes on its last channel.
static const uint8_t txDirOffset[6] = {7, 6, 3, 5, 4, 2};

static const float bearingBasis[6][2]=	{
	{SQRT3_OVER2 , -0.5},
	{0           , -1  },
	{-SQRT3_OVER2, -0.5},
	{-SQRT3_OVER2,  0.5},
	{0           ,  1  },
	{SQRT3_OVER2 ,  0.5}
};

static const float hats[6][2]={
	{11, ELEVEN_SQRT3},
	{22, 0},
	{11, -ELEVEN_SQRT3},
	{-11, -ELEVEN_SQRT3},
	{-22, 0},
	{-11, ELEVEN_SQRT3}
};

static const float headingBasis[6][2]={
	{-1          ,  0  },
	{-0.5,  SQRT3_OVER2},
	{ 0.5,  SQRT3_OVER2},
	{ 1          ,  0  },
	{ 0.5, -SQRT3_OVER2},
	{-0.5, -SQRT3_OVER2}
};

static const float basis_angle[6] = {-(M_PI/6.0), -M_PI_2, -((5.0*M_PI)/6.0), ((5.0*M_PI)/6.0), M_PI_2, (M_PI/6.0)};
static uint32_t sensorHealthHistory;
static int16_t brightMeas[6][6];

static inline float getCosBearingBasis(uint8_t i __attribute__ ((unused)), uint8_t j){
	return bearingBasis[j][0];
}

static inline float getSinBearingBasis(uint8_t i __attribute__ ((unused)), uint8_t j){
	return bearingBasis[j][1];
}

static inline float getCosHeadingBasis(uint8_t i, uint8_t j){
	return headingBasis[(j+(6-i))%6][0];
}

static inline float getSinHeadingBasis(uint8_t i, uint8_t j){
	return headingBasis[(j+(6-i))%6][1];
}

void calculate_bearing_and_heading(float* bearing, float* heading);
float calculate_range(float iRange, float bearing, float heading);
float calculate_error(float r, float b, float h);

static int16_t processBrightMeas(void);

static float magicRangeFunc(float a);
//static float invMagicRangeFunc(float r);

//static void print_brightMeas(void);
												
void range_algs_init(){
	sensorHealthHistory = 0;
	for(uint8_t i=0 ; i<6 ;i++){
		for(uint8_t j=0 ; j<6 ; j++){
			brightMeas[i][j] = 0;
		}
	}
	rnbCmdID=0;
	processing_rnb=0;
}

//TODO: handle variable power.
void broadcast_rnb_data(){
	uint8_t power = 255;
	uint8_t goAhead =0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processing_rnb){
			processing_rnb = 1;
			goAhead = 1;
		}
	}
	if(goAhead){
		uint32_t rnbCmdSentTime = get_time();
		char c = 'r';
		uint8_t result = hp_ir_targeted_cmd(ALL_DIRS, &c, 65, (uint16_t)(rnbCmdSentTime&0xFFFF));
		if(result){
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				hp_ir_block_bm = 0x3F;
			}		
			ir_range_blast(rnbCmdSentTime, power);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				hp_ir_block_bm = 0;
			}
			//printf("rnb_b\r\n");
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		processing_rnb = 0;
	}
}

void use_rnb_data(){
	//uint32_t start = get_time();
	int16_t matrixSum = processBrightMeas();
	//if(rand_byte()%2) broadcastBrightMeas();
	float bearing, heading;
	float error;
	calculate_bearing_and_heading(&bearing, &heading);
	float initialRange = magicRangeFunc(matrixSum/2.0739212652);
	if(initialRange!=0&&!isnanf(initialRange)){	
		float range = calculate_range(initialRange, bearing, heading);
		if(!isnanf(range)){
			if(range<2*DROPLET_RADIUS) range=46;
			error = calculate_error(range, bearing, heading);
			//printf("\t[%04X] %4u % 4d % 4d | %6.2f", rnbCmdID, (uint16_t)range, (int16_t)rad_to_deg(bearing), (int16_t)rad_to_deg(heading), error);
			if((range<110 && error>1.0) || (range<200 && error>1.5) || (range>200)){
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					processing_rnb=0;
				}
				//printf(" <!>\r\n");
				return;
			}else{
				//printf("\r\n");
			}
			
			last_good_rnb.id = rnbCmdID;
			last_good_rnb.range		= (uint16_t)(range);
			last_good_rnb.bearing	= (int16_t)rad_to_deg(bearing);
			last_good_rnb.heading	= (int16_t)rad_to_deg(heading);
			//print_brightMeas();
			rnb_updated=1;
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		processing_rnb=0;
	}
}

float calculate_range(float iRange, float bearing, float heading){
	float bigR[2] = {-iRange*sin(bearing), iRange*cos(bearing)};
	float txHats[6][2];
	for(uint8_t i=0;i<6;i++){
		txHats[i][0] = hats[i][0]*cos(heading) - hats[i][1]*sin(heading);
		txHats[i][1] = hats[i][0]*sin(heading) + hats[i][1]*cos(heading);
	}
	float rij[2];
	float rijMagSq;
	float alphaDotP, betaDotP;
	float cosAcosB;
	float total = 0;
	int16_t* fast_bm = (int16_t*)brightMeas;
	for(uint8_t i=0;i<36;i++){
		uint8_t rx = i%6;
		uint8_t tx = i/6;
		rij[0] = bigR[0] + txHats[tx][0] - hats[rx][0];
		rij[1] = bigR[1] + txHats[tx][1] - hats[rx][1];
		alphaDotP = rij[0]*hats[rx][0] + rij[1]*hats[rx][1];
		alphaDotP = rij[0]*hats[rx][0] + rij[1]*hats[rx][1];
		betaDotP = (-rij[0])*txHats[tx][0] + (-rij[1])*txHats[tx][1];
		betaDotP = betaDotP < 0 ? 0 : betaDotP;
		rijMagSq = rij[0]*rij[0] + rij[1]*rij[1];
		cosAcosB = (alphaDotP*betaDotP)/(rijMagSq*DROPLET_RADIUS_SQ);

		total += fast_bm[i]*cosAcosB;
	}

	return magicRangeFunc(total);
}

float calculate_error(float r, float b, float h){
	r = r<46 ? 46 : r;
	float bigR[2] = {-r*sin(b), r*cos(b)};
	float txHats[6][2];
	for(uint8_t i=0;i<6;i++){
		txHats[i][0] = hats[i][0]*cos(h) - hats[i][1]*sin(h);
		txHats[i][1] = hats[i][0]*sin(h) + hats[i][1]*cos(h);
	}
	float rij[2];
	float rijMagSq;
	float alphaDotP, betaDotP;
	float measTotal = 0;
	float cosAcosBTotal = 0;
	float cosAcosBmat[6][6];
	float* cosAcosB = (float*)cosAcosBmat;
	int16_t* fast_bm = (int16_t*)brightMeas;
	for(uint8_t i=0;i<36;i++){
		uint8_t rx = i%6;
		uint8_t tx = i/6;
		rij[0] = bigR[0] + txHats[tx][0] - hats[rx][0];
		rij[1] = bigR[1] + txHats[tx][1] - hats[rx][1];
		rijMagSq = rij[0]*rij[0] + rij[1]*rij[1];

		alphaDotP = rij[0]*hats[rx][0] + rij[1]*hats[rx][1];
		alphaDotP = alphaDotP < 0 ? 0 : alphaDotP;
		betaDotP = (-rij[0])*txHats[tx][0] + (-rij[1])*txHats[tx][1];
		betaDotP = betaDotP < 0 ? 0 : betaDotP;

		measTotal += fast_bm[i];
		cosAcosB[i] = (alphaDotP*betaDotP)/(rijMagSq*DROPLET_RADIUS_SQ);
		cosAcosBTotal += cosAcosB[i];
	}
	float conf = 0;
	for(uint8_t i=0;i<36;i++){
		conf += fabsf( (fast_bm[i]/measTotal) - (cosAcosB[i]/cosAcosBTotal) );
	}
	//printf("{\"%04X\", %f, {", rnbCmdID, conf);
	//for(uint8_t e=0 ; e<6 ; e++){
		//printf("{");
			//for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++){
				//printf("%d",brightMeas[e][sensor_num]);
				//if(sensor_num<5) printf(",");
			//}
		//printf("}");
		//if(e<5) printf(",");
	//}
	//printf("}, {");
	//for(uint8_t e=0 ; e<6 ; e++){
		//printf("{");
			//for(uint8_t s=0 ; s<6 ; s++){
				//printf("{%f, %f, %f}", cosAcosBmat[e][s], alphaMat[e][s], betaMat[e][s]);
				//if(s<5) printf(", ");
			//}
		//printf("}");
		//if(e<5) printf(",");
	//}
	//printf("}},\r\n");
	return conf;
}

void calculate_bearing_and_heading(float* bearing, float* heading){
	int16_t* fast_bm = (int16_t*)brightMeas;
	
	float bearingX = 0;
	float bearingY = 0;
	float headingX = 0;
	float headingY = 0;

	for(uint8_t i=0;i<36;i++){
		
		bearingX+=fast_bm[i]*getCosBearingBasis(i/6,i%6);
		bearingY+=fast_bm[i]*getSinBearingBasis(i/6,i%6);
		headingX+=fast_bm[i]*getCosHeadingBasis(i/6,i%6);
		headingY+=fast_bm[i]*getSinHeadingBasis(i/6,i%6);
	}
	
	*bearing = atan2f(bearingY, bearingX);	
	*heading = atan2f(headingY, headingX);
}

static int16_t processBrightMeas(void){
	int16_t val;
	int16_t valSum=0;
	uint8_t allColZeroCheck = 0b00111111;

	for(uint8_t e = 0; e < 6; e++){
		for(uint8_t s = 0; s < 6; s++){
			val = brightMeas[e][s];
			allColZeroCheck &= ~((!!val)<<s);	
			//val=val*(val>0);
			brightMeas[e][s] = val;
			valSum+=val;	
		}
	}

	uint8_t problem = 0;
	for(uint8_t i = 0; i<6; i++){
		if(allColZeroCheck&(1<<i)){
			sensorHealthHistory+=(1<<(4*i));
		}else{
			sensorHealthHistory&=~(0xF<<(4*i));
		}
		if(((sensorHealthHistory>>(4*i))&0xF)==0xF){
			printf_P(PSTR("!!!\tGot 15 consecutive nothings from sensor %hu.\t!!!\r\n"), i);
			sensorHealthHistory&=~(0xF<<(4*i));
			problem = 1;
		}		
	}
	if(problem){
		warning_light_sequence();
	}	
	return valSum;
}

void ir_range_meas(uint32_t rnbCmdSentTime){
	//int32_t times[16] = {0};
	cmd_arrival_dir;
	cmd_sender_dir;
	//times[0] = get_time();
	while((get_time()-rnbCmdSentTime+8)<POST_BROADCAST_DELAY);
	//times[1] = get_time();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		uint32_t pre_sync_op = get_time();
		while((get_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS) delay_us(500);
		//times[2] = get_time();
		for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++){
			pre_sync_op = get_time();
			//times[2*emitter_dir+3] = pre_sync_op;
			while((get_time() - pre_sync_op) < (TIME_FOR_GET_IR_VALS-TIME_FOR_IR_MEAS)/2) delay_us(500);
			get_ir_sensors(brightMeas[emitter_dir] , 9); //11
			//times[2*emitter_dir+4] = get_time();			
			while((get_time() - pre_sync_op) < TIME_FOR_GET_IR_VALS) delay_us(500);		
			delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		}
	}
}

void ir_range_blast(uint32_t rnbCmdSentTime, uint8_t power __attribute__ ((unused))){
	//int32_t times[16] = {0};
	//times[0] = get_time();
	while((get_time() - rnbCmdSentTime) < POST_BROADCAST_DELAY) delay_us(500);
	//times[1] = get_time();
	uint32_t pre_sync_op = get_time();
	uint16_t prevPower = curr_ir_power;
	set_all_ir_powers(256);	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		while((get_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS) delay_us(500);
		//times[2] = get_time();
		for(uint8_t dir = 0; dir < 6; dir++){
			pre_sync_op = get_time();
			//set_red_led(255);
			//times[2*dir+3] = pre_sync_op;			
			ir_led_on(dir);
			while((get_time() - pre_sync_op) < TIME_FOR_GET_IR_VALS) delay_us(500);
			ir_led_off(dir);
			//times[2*dir+4] = get_time();				
			//set_red_led(0);					
			delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		}
	}
	set_all_ir_powers(prevPower);
}


static float magicRangeFunc(float a){
	if(a<=0){
		return NAN;
		}else{
		float result = -24.3675811184*(0.0259969683 + powf(a, -0.5));
		result = 1 + exp(result);
		result = (778.0270114700/result) - 528.0270114700;
		return result;
	}
}

//static float invMagicRangeFunc(float r){
	//if(r>250){
		//return 0;
	//}else{
		//float logTerm = log((778.0270114700331/(r + 528.0270114700331)) - 1);
		//float result = 3367.2274479842324/(2.2757149424086466 + logTerm*(7.184767720344338 + logTerm*5.670842845179211));
		//return result;
	//}
//}
//
//static void print_brightMeas(void){
	//printf("{\"%04X\", \"%04X\", {", rnbCmdID, get_droplet_id());
	//for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++){
		//printf("{");
		//for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++){
			//printf("%d",brightMeas[emitter_num][sensor_num]);
			//if(sensor_num<5) printf(",");
		//}
		//printf("}");		
		//if(emitter_num<5) printf(",");
	//}
	//printf("}},\r\n");
//}
