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
												
float basis_angle[6] = {-(M_PI/6.0), -M_PI_2, -((5.0*M_PI)/6.0), ((5.0*M_PI)/6.0), M_PI_2, (M_PI/6.0)};
	

void range_algs_init(){
	sensorHealthHistory = 0;
	for(uint8_t i=0 ; i<6 ;i++){
		for(uint8_t j=0 ; j<6 ; j++){
			brightMeas[i][j] = 0;
		}
	}
	rnbCmdID=0;
	rnbProcessingFlag=0;
}

float expected_bright_mat(float r, float b, float h, uint8_t i, uint8_t j){
	float jX = cosf(basis_angle[i]+h);
	float jY = sinf(basis_angle[i]+h);
	float rijX = r*cosf(b)+DROPLET_RADIUS*(jX-bearingBasis[j][0]);
	float rijY = r*sinf(b)+DROPLET_RADIUS*(jY-bearingBasis[j][1]);
	float alpha = rijX*bearingBasis[j][0]+rijY*bearingBasis[j][1];
	float beta = -rijX*jX-rijY*jY;
	float rijMag = hypotf(rijX, rijY);
	if(alpha>0&&beta>0) return (alpha*beta*amplitude_model(rijMag, 255))/powf(rijMag,2.0);
	else				return 0;
}

void full_expected_bright_mat(float bM[6][6], float r, float b, float h){
	float rX = r*cosf(b);
	float rY = r*sinf(b);
	float riX, riY;
	float jX, jY;
	float rijX, rijY;
	float alpha, beta;
	float rijMag, rijMagSq;
	for(uint8_t i=0;i<6;i++){
		jX = DROPLET_RADIUS*cosf(basis_angle[i]+h);
		jY = DROPLET_RADIUS*sinf(basis_angle[i]+h);
		riX = rX + jX;
		riY = rY + jY;
		for(uint8_t j=0;j<6;j++){
			rijX = riX-scaledBearingBasis[j][0];
			rijY = riY-scaledBearingBasis[j][1];
			alpha = rijX*bearingBasis[j][0]+rijY*bearingBasis[j][1];
			beta =  -rijX*jX - rijY*jY;
			rijMagSq = rijX*rijX+rijY*rijY;
			rijMag = sqrtf(rijMagSq);
			bM[i][j] = (alpha>0 && beta>0) ? ((alpha*beta*amplitude_modelNF(rijMag))/rijMagSq) : 0;
		}
	}
}

float finiteDifferenceStep(float r0, float b0, float h0, float* r1, float* b1, float* h1){
	float err =    calculate_innovationNF(r0, b0, h0);
	//float errRp = calculate_innovationNF(r0+deltaR, b0, h0);
	float errBp =  calculate_innovationNF(r0, b0+FD_DELTA_B, h0);
	float errHp =  calculate_innovationNF(r0, b0, h0+FD_DELTA_H);
	
	//int8_t sgnDeltaEdR = (errRp-err)/FD_DELTA_R;
	int8_t sgnEdB = (err<errBp)-(errBp<err);
	int8_t sgnEdH = (err<errHp)-(errHp<err);
	
	//rStep = rStep*(sgn(deltaEdR)==sgn(prevDeltaEdR) ? STEP_GROW : STEP_SHRINK);
	//rStep = rnb_constrain(rStep);
	bStep.f = bStep.f*(1.5 - (sgnEdB!=prevSgnEdB)); //1.5 if equal, 0.5 if not equal
	//bStep = bStep*(sgnEdB==prevSgnEdB ? FD_STEP_GROW : FD_STEP_SHRINK);
	bStep.f = rnb_constrain(bStep.f);
	hStep.f = hStep.f*(1.5 - (sgnEdH!=prevSgnEdH)); //1.5 if equal, 0.5 if not equal
	//hStep = hStep*(sgnEdH==prevSgnEdH ? FD_STEP_GROW : FD_STEP_SHRINK);
	hStep.f = rnb_constrain(hStep.f);
	
	bMinFlipCount += ((bStep.f==FD_MIN_STEP) && (sgnEdB!=prevSgnEdB)) ? 1 : -bMinFlipCount;
	hMinFlipCount += ((hStep.f==FD_MIN_STEP) && (sgnEdH!=prevSgnEdH)) ? 1 : -hMinFlipCount;
	
	fdStep bAdjust, hAdjust;
	//*r1 = r0-rStep*sgn(deltaEdR);
	*r1 = r0;
	bAdjust.d = (bStep.d|(((int32_t)sgnEdB)&0x80000000));  //flip sign bit of step if sgnEdB is negative.
	*b1 = b0-bAdjust.f;
	hAdjust.d = (hStep.d|(((int32_t)sgnEdH)&0x80000000)); //flip sign bit of step if if sgnEdH is negative.
	*h1 = h0-hAdjust.f; 
	
	//prevDeltaEdR = deltaEdR;
	prevSgnEdH = sgnEdH;
	prevSgnEdB = sgnEdB;
	
	return (bMinFlipCount>=2)&&(hMinFlipCount>=2);
}

float calculate_innovation(float r, float b, float h){
	float expBM[6][6];
	float expNorm  = 0.0;
	float realNorm = 0.0;

	for(uint8_t i=0;i<6;i++){
		for(uint8_t j=0;j<6;j++){
			expBM[i][j]=expected_bright_mat(r, b, h, i,j);
			expNorm += expBM[i][j]*expBM[i][j];
			realNorm += ((int32_t)brightMeas[i][j])*((int32_t)brightMeas[i][j]);
		}
	}
	float expNormInv  = powf(expNorm,-0.5);
	float realNormInv = powf(realNorm,-0.5);
	//printf("%04X | %7.3f %7.3f %7.1f %7.1f\r\n",cmdID, realFrob, expFrob, realTot, expTot);	
	
	
	float error=0.0;	
	for(uint8_t i=0;i<6;i++){
		for(uint8_t j=0;j<6;j++){
			error+=fabsf(brightMeas[i][j]*realNormInv-expBM[i][j]*expNormInv);
		}
	}
	return error;
}

float calculate_innovationNF(float r, float b, float h){
	float expBM[6][6];
	full_expected_bright_mat(expBM, r, b, h);
	float expNorm  = 0.0;
	float realNorm = 0.0;

	for(uint8_t i=0;i<6;i++){
		for(uint8_t j=0;j<6;j++){
			expNorm += expBM[i][j]*expBM[i][j];
			realNorm += ((int32_t)brightMeas[i][j])*((int32_t)brightMeas[i][j]);
		}
	}
	float expNormInv  = powf(expNorm,-0.5);
	float realNormInv = powf(realNorm,-0.5);
	//printf("%04X | %7.3f %7.3f %7.1f %7.1f\r\n",cmdID, realFrob, expFrob, realTot, expTot);
	
	
	float error=0.0;
	for(uint8_t i=0;i<6;i++){
		for(uint8_t j=0;j<6;j++){
			error+=fabsf(brightMeas[i][j]*realNormInv-expBM[i][j]*expNormInv);
		}
	}
	return error;
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
			//printf("rnb_b\r\n");
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		rnbProcessingFlag = 0;
	}
}

//void receive_rnb_data(){
	//if(!rnbProcessingFlag && !hp_ir_block_bm){
		//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			//cmdID=cmd_sender_id;
			//rnbProcessingFlag=1;			
			//hp_ir_block_bm = 1;
		//}
		//ir_range_meas();
		//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			//hp_ir_block_bm = 0;
		//}
		////uint8_t power = 25; //TODO: get this from the message.
		//schedule_task(5, use_rnb_data, NULL);
	//}
//}

void use_rnb_data(){
	//uint32_t start = get_time();
	uint8_t power = 255;
	int16_t matrixSum = processBrightMeas();
	float bearing, heading;
	float error;
	
	calculate_bearing_and_heading(&bearing, &heading);
	float initial_range = get_initial_range_guess(bearing, heading, power);
	if(initial_range!=0&&!isnanf(initial_range)){	
		float range = range_estimate(initial_range, bearing, heading, power);
		if(!isnanf(range)){
			if(range<2*DROPLET_RADIUS) range=5.0;
			float fdR, fdB, fdH;
			float conf = sqrtf(matrixSum);
			
			fdB = bearing;
			fdH = heading;
			fdR = range;

			rStep.f = 10*FD_INIT_STEP;
			bStep.f = FD_INIT_STEP;
			hStep.f = FD_INIT_STEP;
			prevSgnEdR=0;
			prevSgnEdB=0;
			prevSgnEdH=0;
			
			//print_brightMeas();
			error = calculate_innovation(range, bearing, heading);
			//printf("(RNB) ID: %04X \r\n\tBefore: % 5.1f, % 6.1f, % 6.1f\r\n", rnbCmdID, fdR, rad_to_deg(fdB), rad_to_deg(fdH));
			//uint8_t i;			
			//start = get_time();
			//uint8_t earlyAbort;
			//float newR, newB, newH;
			//for(i=0;i<15;i++){
				//earlyAbort = finiteDifferenceStep(fdR, fdB, fdH, &newR, &newB, &newH);
				////printf("\t\t% 5.1f, % 6.1f, % 6.1f  |  %6.4f, %6.4f\r\n", fdR, rad_to_deg(fdB), rad_to_deg(fdH), bStep.f, hStep.f);			
				//fdR = newR;
				//fdB = newB;
				//fdH = newH;
				//if(earlyAbort) break;
			//}
			range = fdR;
			bearing = fdB;
			heading = fdH;
			error = calculate_innovation(range, bearing, heading);
			//printf("\t After: % 5.1f, % 6.1f, % 6.1f, %6.2f [%hu]\r\n", fdR, rad_to_deg(fdB), rad_to_deg(fdH), error>3.0 ? (conf/(10.0*error*error)) : (conf/(error*error)), i);			
			//printf("\tTook %lu ms.\r\n", get_time()-start);
			conf = conf/(error*error);
			if(error>3.0){
				conf = conf/10.0; //Nerf the confidence hard if the calculated error was too high.
			}
			if(isnan(conf)){
				conf = 0.01;
			}
	
			last_good_rnb.id_number = rnbCmdID;
			last_good_rnb.range		= range;
			last_good_rnb.bearing	= bearing;
			last_good_rnb.heading	 = heading;
			last_good_rnb.conf		 = conf;
			rnb_updated=1;
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		rnbProcessingFlag=0;
	}
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

float get_initial_range_guess(float bearing, float heading, uint8_t power){
	int8_t bestS = (6-((int8_t)ceilf((3.0*bearing)/M_PI)))%6;
	float alpha = pretty_angle(bearing - basis_angle[bestS]);				  //alpha using infinite approximation
	int8_t bestE = (6-((int8_t)ceilf((3.0*(bearing-heading-M_PI))/M_PI)))%6;					
	float  beta = pretty_angle(bearing - heading - basis_angle[bestE] - M_PI); //beta using infinite approximation	
	
	//printf("(alpha: %f, sensor %u)\r\n", rad_to_deg(alpha), bestS); 	
	if((alpha > M_PI_2) || (alpha < -M_PI_2)){
		printf("ERROR: alpha out of range (alpha: %f, sensor %u)\r\n", rad_to_deg(alpha), bestS); 
		return 0;
	}
	if((beta > M_PI_2)  || (beta < -M_PI_2)){
		printf("ERROR: beta out of range (beta: %f, emitter %u)\r\n",  beta, bestE); 
		return 0;
	}
	//printf("(beta: %f, emitter %u)\r\n",  rad_to_deg(beta), bestE); 	
	// expected contribution (using infinite distance approximation)
	float amplitude;
	float exp_con = sensor_model(alpha)*emitter_model(beta);
	
	if(exp_con > 0)	amplitude = brightMeas[bestE][bestS]/exp_con;	
	else{
		printf("ERROR: exp_con (%f) is negative (or zero)!\r\n", exp_con); 
		return 0;
	}
	//printf("amp_for_inv: %f\t",amplitude);
	float rMagEst = inverse_amplitude_model(amplitude, power);
	
	float RX = rMagEst*cos(bearing)+DROPLET_RADIUS*(bearingBasis[bestS][0]-headingBasis[bestE][0]);
	float RY = rMagEst*sin(bearing)+DROPLET_RADIUS*(bearingBasis[bestS][1]-headingBasis[bestE][1]);
	
	float rangeEst = hypotf(RX,RY);
	
	return rangeEst;
}

float range_estimate(float init_range, float bearing, float heading, uint8_t power){
	float range_matrix[6][6];
	
	float sensorRXx, sensorRXy, sensorTXx, sensorTXy;
	float alpha, beta, sense_emit_contr;
	float calcRIJmag, calcRx, calcRy;

	int16_t maxBright = -32768;
	uint8_t maxE=0;
	uint8_t maxS=0;
	for(uint8_t e = 0; e < 6; e++){
		for(uint8_t s = 0; s < 6; s++){
			if(brightMeas[e][s]>maxBright){
				maxBright = brightMeas[e][s];
				maxE = e;
				maxS = s;
			}
			
			if(brightMeas[e][s] > 0){												
				sensorRXx = DROPLET_RADIUS*getCosBearingBasis(0,s);
				sensorRXy = DROPLET_RADIUS*getSinBearingBasis(0,s);
				sensorTXx = DROPLET_RADIUS*cosf(basis_angle[e]+heading) + init_range*cosf(bearing);
				sensorTXy = DROPLET_RADIUS*sinf(basis_angle[e]+heading) + init_range*sinf(bearing);

				alpha = atan2f(sensorTXy-sensorRXy,sensorTXx-sensorRXx) - basis_angle[s];
				beta = atan2f(sensorRXy-sensorTXy,sensorRXx-sensorTXx) - basis_angle[e] - heading;

				alpha = pretty_angle(alpha);
				beta = pretty_angle(beta);
				
				sense_emit_contr = sensor_model(alpha)*emitter_model(beta);
				//printf("sense_emit_contr: %f\r\n",sense_emit_contr);
				if(sense_emit_contr>0){
					calcRIJmag = inverse_amplitude_model(brightMeas[e][s]/sense_emit_contr, power);
				}else{
					calcRIJmag = 0;
				}
				calcRx = calcRIJmag*cosf(alpha) + sensorRXx - DROPLET_RADIUS*cosf(basis_angle[e]+heading);
				calcRy = calcRIJmag*sinf(alpha) + sensorRXy - DROPLET_RADIUS*sinf(basis_angle[e]+heading);
				range_matrix[e][s] = hypotf(calcRx, calcRy);
				continue;
			}
			range_matrix[e][s]=0;
		}
	}
	
	float rangeMatSubset[3][3];
	float brightMatSubset[3][3];
	float froebNormSquared=0;
	for(uint8_t e = 0; e < 3; e++){
		for(uint8_t s = 0; s < 3; s++){
			uint8_t otherE = ((maxE+(e+5))%6);
			uint8_t otherS = ((maxS+(s+5))%6);
			rangeMatSubset[e][s] = range_matrix[otherE][otherS];
			brightMatSubset[e][s] = (float)brightMeas[otherE][otherS];
			froebNormSquared+=powf(brightMatSubset[e][s],2);
		}
	}
	float froebNorm = sqrtf(froebNormSquared);
	float range = 0;
	for(uint8_t e = 0; e < 3; e++){
		for(uint8_t s = 0; s < 3; s++){
			range+= rangeMatSubset[e][s]*powf(brightMatSubset[e][s]/froebNorm,2);
		}
	}
	//printf("R: %f\r\n", range);	
	//print_range_matrix(range_matrix);
	//printf("\n");
	return range;
}

int16_t processBrightMeas(){
	int16_t val;
	int16_t valSum=0;
	uint8_t allColZeroCheck = 0b00111111;
	
	for(uint8_t e = 0; e < 6; e++){
		for(uint8_t s = 0; s < 6; s++){
			val = brightMeas[e][s];
			allColZeroCheck &= ~((!!val)<<s);	
			val=val*(val>0);
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
		startup_light_sequence();
	}	
	return valSum;
}

void ir_range_meas(){
	//int32_t times[16] = {0};
	cmd_arrival_dir;
	cmd_sender_dir;
	//times[0] = get_time();
	while((get_time()-rnbCmdSentTime+8)<POST_BROADCAST_DELAY);
	//times[1] = get_time();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		uint32_t pre_sync_op = get_time();
		while((get_time()-pre_sync_op)<TIME_FOR_SET_IR_POWERS);
		//times[2] = get_time();
		for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++){
			pre_sync_op = get_time();
			//times[2*emitter_dir+3] = pre_sync_op;
			get_ir_sensors(brightMeas[emitter_dir] , 9); //11
			//times[2*emitter_dir+4] = get_time();			
			while((get_time() - pre_sync_op) < TIME_FOR_GET_IR_VALS);				
			delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		}
	}
	//times[15] = get_time();
	//printf("M\r\n");
	//printf("T: %lu\r\n\t", times[0]-rnbCmdSentTime+8);
	//for(uint8_t i=1; i<16;i++){
		//printf("%hu ", (uint8_t)(times[i]-times[i-1]));
	//}
	//printf("\r\n");
}

void ir_range_blast(uint8_t power){
	//int32_t times[16] = {0};
	//times[0] = get_time();
	while((get_time()-rnbCmdSentTime)<POST_BROADCAST_DELAY);
	//times[1] = get_time();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
		uint32_t pre_sync_op = get_time();
		set_all_ir_powers(256);
		while((get_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS);
		//times[2] = get_time();
		for(uint8_t dir = 0; dir < 6; dir++){
			pre_sync_op = get_time();
			//set_red_led(255);
			//times[2*dir+3] = pre_sync_op;			
			ir_led_on(dir);
			while((get_time() - pre_sync_op) < TIME_FOR_GET_IR_VALS);
			ir_led_off(dir);
			//times[2*dir+4] = get_time();				
			//set_red_led(0);					
			delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		}
	}
	//times[15] = get_time();
	//printf("B\r\n");
	//printf("T: %lu\r\n\t", times[0]-rnbCmdSentTime+8);
	//for(uint8_t i=1; i<16;i++){
		//printf("%hu ", (uint8_t)(times[i]-times[i-1]));
	//}
	//printf("\r\n");
}

float pretty_angle(float alpha){
	float val;
	if (alpha >= 0) val = fmodf(alpha + M_PI, 2*M_PI) - M_PI;
	else val = fmodf(alpha - M_PI, 2*M_PI) + M_PI;
	return val;
}

float rad_to_deg(float rad){
	return (pretty_angle(rad) / M_PI) * 180;
}

float deg_to_rad(float deg){
	return pretty_angle( (deg / 180) * M_PI );
}

float sensor_model(float alpha){
	if(fabsf(alpha)>=1.5){
		return 0.0;
	}else if(fabsf(alpha)<=0.62){
		return (1-powf(alpha,4));
	}else{
		return 0.125/powf(alpha,4);
	}
}

float emitter_model(float beta){	
	if(fabsf(beta)>=1.5){
		return 0.0;
	}else if(fabsf(beta)<=0.72){
		return (0.94+powf(beta,2)*0.5-powf(beta,4));
	}else{
		return 0.25/powf(beta,4);
	}
}

float amplitude_model(float r, uint8_t power){
	if(power==255)			return 15.91+(12985.5/powf(r+0.89,2.0));
	//else if(power ==250)	return (1100./((r-4.)*(r-4.)))+12.5;
	else					printf_P(PSTR("ERROR: Unexpected power: %hhu\r\n"),power);
	return 0;
}

float inverse_amplitude_model(float ADC_val, uint8_t power){
	if(power == 255)		return 2*(-1.5+(131.5/sqrtf(ADC_val-3.85)));
	//else if(power == 250) return (33.166/sqrtf(ADC_val - 12.5)) + 4;
	else					printf_P(PSTR("ERROR: Unexpected power: %hhu\r\n"),power);
	return 0;
}


void debug_print_timer(uint32_t timer[14]){
	printf_P(PSTR("Duration: %lu\r\n"),(timer[13] - timer[0]));
	printf("|  ");
	for(uint8_t i=1 ; i<13 ; i++)
	{
		printf_P(PSTR("%3lu  |  "),timer[i] - timer[i-1]);
	}
	printf("\r\n");
}

void print_brightMeas(){
	printf("{\r\n");
	for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++){
		printf("\t{");
		for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++){
			printf("%4d",brightMeas[emitter_num][sensor_num]);
			if(sensor_num<5) printf(",");
		}
		printf("}");		
		if(emitter_num<5) printf(",");
		printf("\r\n");
	}
	printf("};\r\n");
}

void print_range_matrix(float range_matrix[6][6]){
	printf("{\r\n");
		for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++){
			printf("\t{");
				for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++){
					printf("%3.2f",range_matrix[emitter_num][sensor_num]);
					if(sensor_num<5) printf(",");
				}
			printf("}");
			if(emitter_num<5) printf(",");
			printf("\r\n");
		}
	printf("};\r\n");
}