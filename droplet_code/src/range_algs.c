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



static const float basisAngle[6] =	{-M_PI_6, -M_PI_2, -5.0*M_PI_6, 5.0*M_PI_6, M_PI_2, M_PI_6};

static const float bearingBasis[6][2]=	{
	{SQRT3_OVER2 , -0.5},
	{0           , -1  },
	{-SQRT3_OVER2, -0.5},
	{-SQRT3_OVER2,  0.5},
	{0           ,  1  },
	{SQRT3_OVER2 ,  0.5}
};

static int16_t bm[6];
static uint32_t sensorHealthHistory;
static int16_t calculate_range(float bearing);
static float calculate_bearing();
static void checkSensorHealth();


void range_algs_init(){
	sensorHealthHistory = 0;
	for(uint8_t i=0 ; i<6 ;i++){
		bm[i] = 0;
	}
	rnbCmdID=0;
	rnbProcessingFlag=0;
}

//TODO: handle variable power.
void broadcast_rnb_data(){
	uint8_t power = 255;
	uint8_t goAhead =0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!rnbProcessingFlag){
			rnbProcessingFlag = 1;
			goAhead = 1;
		}
	}
	if(goAhead){
		rnbCmdSentTime = get_time();
		char c = 'r';
		uint8_t result = hp_ir_targeted_cmd(ALL_DIRS, &c, 65, (uint16_t)(rnbCmdSentTime&0xFFFF));
		if(result){
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				hp_ir_block_bm = 0xFF;
			}		
			ir_range_blast(power);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				hp_ir_block_bm = 0;
			}
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		rnbProcessingFlag = 0;
	}
}

void use_rnb_data(){
	//checkSensorHealth();
	float bearing = calculate_bearing();
	int16_t intRange = calculate_range(bearing);
	int16_t intBearing = (int16_t)rad_to_deg(bearing);
	printf("{\"%04X\", %4d, % 4d},\r\n", rnbCmdID, intRange, intBearing);
	last_good_rnb.id		= rnbCmdID;
	last_good_rnb.range		= intRange;
	last_good_rnb.bearing	= intBearing;
	rnb_updated=1;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		rnbProcessingFlag=0;
	}
}

static int16_t calculate_range(float bearing){
	float alpha;
	float cosScaledBM = 0.0;
	for(uint8_t s=0;s<6;s++){
		alpha = pretty_angle(bearing-basisAngle[s]);
		if(alpha>(-M_PI_2) && alpha<M_PI_2){
			cosScaledBM += cosf(alpha)*bm[s];
		}
	}
	if(cosScaledBM > 0 && cosScaledBM < 1713.56){
		return (int16_t)(488.533 - 0.03291*cosScaledBM - 49.7895*log(cosScaledBM));
	}else if(cosScaledBM >= 1713.56 && cosScaledBM < 5390.22){
		return (int16_t)(89.8554-0.00969*cosScaledBM);
	}else if(cosScaledBM >= 5390.22){
		return 44;
	}else{ //last case is <= 0.
		return 0;
	}
}

static float calculate_bearing(){
	float bearingX = 0;
	float bearingY = 0;
	float totalBM = 0;

	for(uint8_t i=0;i<6;i++){
		bearingX+=bm[i]*bearingBasis[i][0];
		bearingY+=bm[i]*bearingBasis[i][1];
		totalBM+=bm[i];
	}
	return pretty_angle(atan2f(bearingY, bearingX));	
	//Check wikipedia: "Directional_statistics#measures_of_location_and_spread" to justify next few lines
	//float r = hypotf(bearingX, bearingY)/totalBM;
	//*var = (1.0 - r); 
	//*stdDev = sqrtf(-2.0*log(r));
}

static void checkSensorHealth(){
	uint8_t problem = 0;
	for(uint8_t i = 0; i<6; i++){
		if(bm[i]==0){
			sensorHealthHistory+=(1<<(5*i));
		}else{
			sensorHealthHistory&=~(0x1F<<(5*i));
		}
		if(((sensorHealthHistory>>(5*i))&0x1F)==0x1F){
			printf_P(PSTR("!!!\tGot 31 consecutive nothings from sensor %hu.\t!!!\r\n"), i);
			sensorHealthHistory&=~(0xF<<(4*i));
			problem = 1;
		}
	}
	if(problem){
		startup_light_sequence();
	}
}
void ir_range_meas(){
	while((get_time()-rnbCmdSentTime+8)<POST_BROADCAST_DELAY);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		uint32_t pre_sync_op = get_time();
		while((get_time()-pre_sync_op)<TIME_FOR_SET_IR_POWERS);
		pre_sync_op = get_time();
		delay_ms((TIME_FOR_IR_MEAS-TIME_FOR_GET_IR_VALS)/2);
		get_ir_sensors(bm,9);
		while((get_time() - pre_sync_op) < TIME_FOR_IR_MEAS);
	}
}

void ir_range_blast(uint8_t power __attribute__ ((unused))){
	while((get_time()-rnbCmdSentTime)<POST_BROADCAST_DELAY);
	uint32_t pre_sync_op = get_time();
	set_all_ir_powers(256);	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		while((get_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS);
		for(uint8_t i=0;i<6;i++) ir_led_on(i);
		delay_ms(TIME_FOR_IR_MEAS);
		for(uint8_t i=0;i<6;i++) ir_led_off(i);
	}
}