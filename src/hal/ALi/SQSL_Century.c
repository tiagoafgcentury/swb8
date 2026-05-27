/**************************************************
*
*
*Author: Ken
****************************************************/

typedef struct {
	unsigned int 	level_start;
	unsigned int 	level_end;
	unsigned int 	cali_res;
} CK_SL_CALI_TABLE_S;


typedef struct s_CKTUNERStatus
{
	uint32_t FreKhz;
	uint32_t SymbolRate;
	uint32_t ChannelBW;
	uint32_t Modulation;
	e_CK_LNBPOLARIZATION Polarity;
	e_CK_LNB22KTONE Lnb22kState;
	e_CK_TUNERLOCK State;
	uint32_t SignalQuality;
	uint32_t BitErrorRate;
	uint32_t RFLevel;
	e_CK_LNBSTATUS LNBStatus;
	e_CK_DEMODTYPE DemodType;
	e_CK_DEMODCR	 DemodCR;
} s_CKTUNERStatus;



#ifdef GUICTBR
/********************************************************************
* Description: calibration signal quality
*
* Author: 	Vincent
*
**********************************************************************/
static void CK_CommonAPI_Tools_calibration_SQ(s_CKTUNERStatus *status)
{
	uint32_t ori_ber = status->BitErrorRate;
	double cal_ber = ((double)status->BitErrorRate*1.0)/1000;
	uint32_t cal_sq = 0;
	int8_t sq_offset = 0;// Ken: from Hubert field report

#if 1//Ken++
	CKGUICOMTOOLSDBG("ori SQ : %d\n", status->SignalQuality);

	if (status->DemodType == CK_DEMOD_DVBS)
	{
		CKGUICOMTOOLSDBG("Fun=[%s] Line=[%d] ==> Demod is DVBS, Code Rate : %d\n", __FUNCTION__, __LINE__, status->DemodCR);
		CKGUICOMTOOLSDBG("CK_DEMODCR : %d\n", status->DemodCR);
		
		switch (status->DemodCR)
		{
			case CK_DEMODCR_1_2:
			case CK_DEMODCR_2_3:	
			{
				if (ori_ber >= 59000)
				{
					cal_sq = (uint32_t) round(-0.3176 * cal_ber + 39.057);
					sq_offset = -1;
				}
				else if (ori_ber >= 2820)
				{
					cal_sq = (uint32_t) round(-1.2847 * cal_ber + 96.15);
					sq_offset = -1;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;
			
			case CK_DEMODCR_3_4:	
			{
				if (ori_ber >= 22000)
				{
					cal_sq = (uint32_t) round(0.0144 * pow(cal_ber, 2) - 1.9131 * cal_ber + 69.327);
					sq_offset = -3;
				}
				else if (ori_ber >= 1700)
				{
					cal_sq = (uint32_t) round(-0.1236 * pow(cal_ber, 2) + 0.0769 * cal_ber + 87.78);
					sq_offset = -3;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;

			case CK_DEMODCR_5_6:	
			{
				if (ori_ber >= 525)
				{
					cal_sq = (uint32_t) round(-0.0015 * pow(cal_ber, 3) + 0.19 * pow(cal_ber, 2) - 7.3501 * cal_ber + 96.445);
					sq_offset = -2;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;

			case CK_DEMODCR_6_7:			
			case CK_DEMODCR_7_8:
			{
				if (ori_ber >= 200)
				{
					cal_sq = (uint32_t) round(-0.012 * pow(cal_ber, 3) + 0.6507 * pow(cal_ber, 2) - 12.484 * cal_ber + 92.791);
					sq_offset = 0;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;
			
			default:	{ status->SignalQuality = 0; }  break;
		}
	}
	else if (status->DemodType == CK_DEMOD_DVBS2)
	{
		CKGUICOMTOOLSDBG("Fun=[%s] Line=[%d] ==> Demod is DVBS2, Code Rate : %d\n", __FUNCTION__, __LINE__, status->DemodCR);
		CKGUICOMTOOLSDBG("CK_DEMODCR : %d\n", status->DemodCR);
		
		switch (status->DemodCR)
		{
			case CK_DEMODCR_2_3:	
			{
				if (ori_ber >= 83000)
				{
					cal_sq = (uint32_t) round(-7.4164 * cal_ber + 675.46);
					sq_offset = -8;
				}
				else if (ori_ber >= 24200)
				{
					cal_sq = (uint32_t) round(-0.485 * cal_ber + 100.76);
					sq_offset = -8;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;
				
			case CK_DEMODCR_3_4:
			{
				if (ori_ber >= 60500)
				{
					cal_sq = (uint32_t) round(2.8914 * pow(cal_ber, 2) - 373.12 * cal_ber + 12042);
					sq_offset = -5;

				}
				else if (ori_ber >= 18150)
				{
					cal_sq = (uint32_t) round(-0.7446 * cal_ber + 102.07);
					sq_offset = -5;
				}
				else 
				{
					cal_sq = 90;
				}	
			}
			break;

			case CK_DEMODCR_4_5:
			case CK_DEMODCR_8_9:
			case CK_DEMODCR_9_10:
			case CK_DEMODCR_5_6:	
			{
				if (ori_ber >= 39200)
				{
					cal_sq = (uint32_t) round(3.6412 * pow(cal_ber, 2) - 311.47 * cal_ber + 6666.5);
					sq_offset = 0;
				}
				else if (ori_ber >= 6450)
				{
					cal_sq = (uint32_t) round(-1.0512 * cal_ber + 94.142);
					sq_offset = 0;
				}
				else
				{
					cal_sq = 90;
				}	
			}
			break;

			case CK_DEMODCR_1_4:
			case CK_DEMODCR_1_3:
			case CK_DEMODCR_2_5:
			case CK_DEMODCR_1_2:
			case CK_DEMODCR_3_5:	
			{
				if (ori_ber >= 109000)
				{
					cal_sq = (uint32_t) round(71.247 * pow(cal_ber/10, 2) - 1682.8 * (cal_ber/10) + 9938.2);
					sq_offset = -4;
				}
				else if (ori_ber >= 43500)
				{
					cal_sq = (uint32_t) round(-4.0013 * (cal_ber/10) + 107.02);
					sq_offset = -4;
				}
				else
				{
					cal_sq = 90;
				}
			}

			break;			

			default:	{ status->SignalQuality = 0; }  break;
		}
	}
	else
	{
		status->SignalQuality = 0;
	}
#else
	if (ori_ber > 85000) {
		cal_sq = CK_CommonAPI_Tools_round_to_int(1.3428 * cal_ber * cal_ber - 247.46 * cal_ber + 11401);
	}
	else {
		cal_sq = CK_CommonAPI_Tools_round_to_int(-0.4795 * cal_ber + 100.91);
	}
#endif

	if (status->State == CK_TUNER_UNLOCK)
	{
		status->SignalQuality = 0;
	}
	else if (cal_sq < 1)
	{
		status->SignalQuality = 1;
	}
	else if (cal_sq > 90)
	{
		status->SignalQuality = 90;
	}
	else
	{
		status->SignalQuality = cal_sq;
		status->SignalQuality+=sq_offset;
	}
	CKGUICOMTOOLSDBG("cali SQ : [%d]\n", status->SignalQuality);
}
/********************************************************************
* Description: calibration signal strength
*
* Author: 	Vincent
*
**********************************************************************/
static CK_SL_CALI_TABLE_S cali_sl_table1[] = {
		/* 
		 * SL calibration table for 8PSK+QPSK 3/4.
		 *
		 * set as {start SL, end SL, calibration SL}
		 */
		{22, 22, 31},
		{23, 23, 36},
		{24, 26, 41},
		{27, 27, 46},
		{28, 30, 51},
		{31, 43, 56},
		{44, 44, 61},
		{45, 45, 63},
		{46, 53, 66},
		{54, 57, 67},
		{58, 58, 68},
		{59, 59, 69},
		{60, 60, 70},
		{61, 61, 71},
		{62, 62, 72},
		{63, 64, 73},
		{65, 66, 74},
		{67, 67, 76},
		{68, 68, 77},
		{69, 71, 78},
		{72, 78, 79},
		{79, 83, 80},
		{84, 85, 81},
		{86, 86, 82},
		{87, 89, 83},
		{90, 90, 84},
		{91, 91, 85},
		{92, 93, 86},
		{94, 97, 87},
		{98, 99, 88},
		{100, 100, 90},
};
static CK_SL_CALI_TABLE_S cali_sl_table2[] = {
		/* 
		 * SL calibration table for QPSK 2/3, 5/6, 7/8
		 *
		 * set as {start SL, end SL, calibration SL}
		 */
		{29, 30, 31},
		{31, 31, 36},
		{32, 32, 41},
		{33, 33, 46},
		{34, 34, 51},
		{35, 41, 56},
		{42, 42, 61},
		{43, 43, 63},
		{44, 52, 66},
		{54, 57, 67},
		{58, 58, 68},
		{59, 59, 69},
		{60, 60, 70},
		{61, 61, 71},
		{62, 62, 72},
		{63, 64, 73},
		{65, 66, 74},
		{67, 67, 76},
		{68, 68, 77},
		{69, 71, 78},
		{72, 78, 79},
		{79, 83, 80},
		{84, 85, 81},
		{86, 86, 82},
		{87, 89, 83},
		{90, 90, 84},
		{91, 91, 85},
		{92, 93, 86},
		{94, 97, 87},
		{98, 99, 88},
		{100, 100, 90},
};
static CK_SL_CALI_TABLE_S cali_sl_table3[] = {
		/* 
		 * SL calibration table for 8PSK+QPSK 3/4, H, SR3744 ~ 3758
		 *
		 * set as {start SL, end SL, calibration SL}
		 */
		{1, 3, 51},
		{4, 43, 56},
		{44, 44, 61},
		{45, 45, 63},
		{46, 53, 66},
		{54, 57, 67},
		{58, 58, 68},
		{59, 59, 69},
		{60, 60, 70},
		{61, 61, 71},
		{62, 62, 72},
		{63, 64, 73},
		{65, 66, 74},
		{67, 67, 76},
		{68, 68, 77},
		{69, 71, 78},
		{72, 78, 79},
		{79, 83, 80},
		{84, 85, 81},
		{86, 86, 82},
		{87, 89, 83},
		{90, 90, 84},
		{91, 91, 85},
		{92, 93, 86},
		{94, 97, 87},
		{98, 99, 88},
		{100, 100, 90},
};
static CK_SL_CALI_TABLE_S cali_sl_table4[] = {
		/* 
		 * SL calibration table for 8PSK+QPSK 3/4, V, SR4568 ~ 4578
		 *
		 * set as {start SL, end SL, calibration SL}
		 */
		{30, 35, 31},
		{36, 36, 36},
		{37, 37, 41},
		{38, 38, 46},
		{39, 39, 51},
		{40, 40, 56},
		{41, 41, 61},
		{42, 42, 63},
		{43, 53, 66},
		{54, 57, 67},
		{58, 58, 68},
		{59, 59, 69},
		{60, 60, 70},
		{61, 61, 71},
		{62, 62, 72},
		{63, 64, 73},
		{65, 66, 74},
		{67, 67, 76},
		{68, 68, 77},
		{69, 71, 78},
		{72, 78, 79},
		{79, 83, 80},
		{84, 85, 81},
		{86, 86, 82},
		{87, 89, 83},
		{90, 90, 84},
		{91, 91, 85},
		{92, 93, 86},
		{94, 97, 87},
		{98, 99, 88},
		{100, 100, 90},
};
static void CK_CommonAPI_Tools_calibration_SL(s_CKTUNERStatus *status)
{
	int i;
	CK_SL_CALI_TABLE_S *used_cali_table = NULL;
	unsigned int low_SL_th = 0, table_size;
	BOOL need_cali = FALSE;

	CKGUICOMTOOLSDBG("ori SL : %d\n", status->RFLevel);

	if (status->DemodType == CK_DEMOD_DVBS) {
		CKGUICOMTOOLSDBG("Fun=[%s] Line=[%d] ==> Demod is DVBS, Code Rate : %d\n", __FUNCTION__, __LINE__, status->DemodCR);
		CKGUICOMTOOLSDBG("CK_DEMODCR : %d\n", status->DemodCR);
		
		switch (status->DemodCR)	{
			case CK_DEMODCR_2_3:
			case CK_DEMODCR_5_6:
			case CK_DEMODCR_7_8:
			{
				need_cali = TRUE;
				low_SL_th = cali_sl_table2[0].level_start;
				used_cali_table = &cali_sl_table2[0];
				table_size = sizeof(cali_sl_table2) / sizeof(CK_SL_CALI_TABLE_S);
			}
			break;
			
			case CK_DEMODCR_3_4:
			{
				need_cali = TRUE;
				if ((status->SymbolRate >= 3744 && status->SymbolRate <= 3758) && status->Polarity == CK_POLA_HORIZATION) {
					low_SL_th = cali_sl_table3[0].level_start;
					used_cali_table = &cali_sl_table3[0];
					table_size = sizeof(cali_sl_table3) / sizeof(CK_SL_CALI_TABLE_S);
				}
				else if ((status->SymbolRate >= 4568 && status->SymbolRate <= 4578) && status->Polarity == CK_POLA_VERTICAL) {
					low_SL_th = cali_sl_table4[0].level_start;
					used_cali_table = &cali_sl_table4[0];
					table_size = sizeof(cali_sl_table4) / sizeof(CK_SL_CALI_TABLE_S);
				}
				else {
					low_SL_th = cali_sl_table1[0].level_start;
					used_cali_table = &cali_sl_table1[0];
					table_size = sizeof(cali_sl_table1) / sizeof(CK_SL_CALI_TABLE_S);
				}
			}
			break;

			case CK_DEMODCR_1_2:
			case CK_DEMODCR_6_7:
			default:
			{
				need_cali = FALSE;
				//status->RFLevel = 0;
			}
			break;
		}
	}
	else if(status->DemodType == CK_DEMOD_DVBS2)
	{
		CKGUICOMTOOLSDBG("Fun=[%s] Line=[%d] ==> Demod is DVBS2, Code Rate : %d\n", __FUNCTION__, __LINE__, status->DemodCR);
		switch( status->DemodCR)
		{
			case CK_DEMODCR_2_3:
			case CK_DEMODCR_5_6:
			case CK_DEMODCR_3_5:
			{
				need_cali = TRUE;
				low_SL_th = cali_sl_table1[0].level_start;
				used_cali_table = &cali_sl_table1[0];
				table_size = sizeof(cali_sl_table1) / sizeof(CK_SL_CALI_TABLE_S);
			}
			break;

			case CK_DEMODCR_3_4:
			{
				need_cali = TRUE;
				if ((status->SymbolRate >= 3744 && status->SymbolRate <= 3758) && status->Polarity == CK_POLA_HORIZATION) {
					low_SL_th = cali_sl_table3[0].level_start;
					used_cali_table = &cali_sl_table3[0];
					table_size = sizeof(cali_sl_table3) / sizeof(CK_SL_CALI_TABLE_S);
				}
				else if ((status->SymbolRate >= 4568 && status->SymbolRate <= 4578) && status->Polarity == CK_POLA_VERTICAL) {
					low_SL_th = cali_sl_table4[0].level_start;
					used_cali_table = &cali_sl_table4[0];
					table_size = sizeof(cali_sl_table4) / sizeof(CK_SL_CALI_TABLE_S);
				}
				else {
					low_SL_th = cali_sl_table1[0].level_start;
					used_cali_table = &cali_sl_table1[0];
					table_size = sizeof(cali_sl_table1) / sizeof(CK_SL_CALI_TABLE_S);
				}
			}
			break;

			case CK_DEMODCR_1_2:
			case CK_DEMODCR_1_4:
			case CK_DEMODCR_1_3:
			case CK_DEMODCR_2_5:
			case CK_DEMODCR_4_5:
			case CK_DEMODCR_8_9:
			case CK_DEMODCR_9_10:
			default:
			{
				need_cali = FALSE;
				//status->RFLevel = 0;
			}
			break;
		}
	}

	if (need_cali) {
		if (status->RFLevel < low_SL_th) {		//low signal strength case
			if (status->SignalQuality >= 30)
				status->RFLevel = 30;
			else
				status->RFLevel = status->SignalQuality;
		}
		else {
			for (i = table_size - 1; i >= 0; i--) {
				if (status->RFLevel >= used_cali_table[i].level_start && status->RFLevel <= used_cali_table[i].level_end) {
					status->RFLevel = used_cali_table[i].cali_res;
					break;
				}
			}
		}
	}
	
	CKGUICOMTOOLSDBG("cali SL : [%d] \n\n", status->RFLevel);
}
#endif 
/********************************************************************
* Description: 
*
* Author: 	Derek
*
**********************************************************************/
void CK_CommonAPI_Tools_Tune_GetTunerStatus(TUNER_TYPE tuner_type, s_CKTUNERStatus* status)
{

	if(tunerStatus_mutex==NULL)
	{
		CK_PortingInterface_CreateMutex(&tunerStatus_mutex);
	}

	if(tunerStatus_mutex)
	{
		CK_PortingInterface_MutexLock(tunerStatus_mutex);
	}

	CK_PortingInterface_TUNER_GetTunerStatus(tuner_type.tuner_port,tuner_type.tuner_sys, status);
#ifdef GUICTBR
	//calibration signal quality
	CK_CommonAPI_Tools_calibration_SQ(status);
	//calibration singal strength
	CK_CommonAPI_Tools_calibration_SL(status);
#endif
	//CK_CommonAPI_Tools_LockLed(status); //Joe marked, cause Heron do not want led flash when no signal


	if(tunerStatus_mutex)
	{
		CK_PortingInterface_MutexUnLock(tunerStatus_mutex);
	}
}
