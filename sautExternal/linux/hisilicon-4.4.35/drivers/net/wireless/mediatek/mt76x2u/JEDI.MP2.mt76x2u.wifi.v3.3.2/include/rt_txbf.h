/*
  * Copyright (c) 2016 MediaTek Inc.  All rights reserved.
  *
  * This software is available to you under a choice of one of two
  * licenses.  You may choose to be licensed under the terms of the GNU
  * General Public License (GPL) Version 2, available from the file
  * COPYING in the main directory of this source tree, or the
  * BSD license below:
  *
  *	Redistribution and use in source and binary forms, with or
  *	without modification, are permitted provided that the following
  *	conditions are met:
  *
  *	 - Redistributions of source code must retain the above
  *	   copyright notice, this list of conditions and the following
  *	   disclaimer.
  *
  *	 - Redistributions in binary form must reproduce the above
  *	   copyright notice, this list of conditions and the following
  *	   disclaimer in the documentation and/or other materials
  *	   provided with the distribution.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */


#ifndef _RT_TXBF_H_
#define _RT_TXBF_H_

#ifdef TXBF_SUPPORT

#ifdef MT76x2
/* Divider phase calibration closed loop definition */
#define RX0TX0	   0
#define RX1TX1	   5

#define ADC0_RX0_2R   8
#define ADC1_RX1_2R   8
#endif /* endif */

/* #define MRQ_FORCE_TX		 //Force MRQ regardless the capability of the station */

/* TxSndgPkt Sounding type definitions */
#define SNDG_TYPE_DISABLE	0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP		2

/* Explicit TxBF feedback mechanism */
#define ETXBF_FB_DISABLE	0
#define ETXBF_FB_CSI		1
#define ETXBF_FB_NONCOMP	2
#define ETXBF_FB_COMP		4

/* #define MRQ_FORCE_TX		 //Force MRQ regardless the capability of the station */

/*
	eTxBfEnCond values:
	 0:no etxbf,
	 1:etxbf update periodically,
	 2:etxbf updated if mcs changes in RateSwitchingAdapt() or APQuickResponeForRateUpExecAdapt().
	 3:auto-selection: if mfb changes or timer expires, then send sounding packets <------not finished yet!!!
	 note:
		when = 1 or 3, NO_SNDG_CNT_THRD controls the frequency to update the
		matrix(ETXBF_EN_COND=1) or activate the whole bf evaluation process(not defined)
*/

/* Defines to include optional code. */
/* NOTE: Do not define these options. ETxBfEnCond==3 and */
/* MCS Feedback are not fully implemented */
/* #define ETXBF_EN_COND3_SUPPORT	 // Include ETxBfEnCond==3 code */
/* #define MFB_SUPPORT				 // Include MCS Feedback code */

/* MCS FB definitions */
#define MSI_TOGGLE_BF		6
#define TOGGLE_BF_PKTS		5	/* the number of packets with inverted BF status */

/* TXBF State definitions */
#define READY_FOR_SNDG0		0	/* jump to WAIT_SNDG_FB0 when channel change or periodically */
#define WAIT_SNDG_FB0		1	/* jump to WAIT_SNDG_FB1 when bf report0 is received */
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5
/* send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec.
* If this =0,bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt()
*/
#define NO_SNDG_CNT_THRD   0

/* ------------ BEAMFORMING PROFILE HANDLING ------------ */

#define IMP_MAX_BYTES		14	/* Implicit: 14 bytes per subcarrier */
#define IMP_MAX_BYTES_ONE_COL	7	/* Implicit: 7 bytes per subcarrier, when reading first column */
#define EXP_MAX_BYTES		18	/* Explicit: 18 bytes per subcarrier */
#ifdef MT76x2
#define MAX_BYTES	     2	/* 2 bytes per subcarrier for implicit and explicit TxBf */
#endif /* endif */
#define IMP_COEFF_SIZE		 9	/* 9 bits/coeff */
#define IMP_COEFF_MASK		0x1FF

#define PROFILE_MAX_CARRIERS_20		56	/* Number of subcarriers in 20 MHz mode */
#define PROFILE_MAX_CARRIERS_40		114	/* Number of subcarriers in 40 MHz mode */
#ifdef MT76x2
#define PROFILE_MAX_CARRIERS_80		242	/* Number of subcarriers in 80 MHz mode */
#endif /* endif */

#ifdef MT76x2
#define NUM_CHAIN			 3
#endif /* endif */

/* Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz */
typedef struct {
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;

typedef struct {
	BOOLEAN impProfile;
	BOOLEAN fortyMHz;
	int rows, columns;
	int grouping;
	UCHAR tag[EXP_MAX_BYTES];
	UCHAR data[PROFILE_MAX_CARRIERS_40][EXP_MAX_BYTES];
} PROFILE_DATA;

extern PROFILE_DATA profData;

#ifdef MT76x2
typedef struct {
	UCHAR ng, cw, nrow, ncol, LM;
	UCHAR LD, EO, IO, I_E;
	UCHAR DMAC[6];
	UCHAR Tx1_scale_2ss, Tx0_scale_2ss, Tx1_scale_1ss, Tx0_scale_1ss;
	UCHAR STS1_SNR, STS0_SNR;
	USHORT timeout;
	UCHAR validFlg;
	UCHAR CMDInIdx;
} PFMU_PROFILE;

typedef struct {
	UCHAR dCMDInIdx;
	UCHAR psi21, phill;
	UCHAR data[PROFILE_MAX_CARRIERS_80][MAX_BYTES];
} PFMU_DATA;
#endif /* endif */

typedef
	struct {

#ifdef MT76x2
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
#endif				/* endif */
} ITXBF_PHASE_PARAMS;		/* ITxBF BBP reg phase calibration parameters */

typedef
	struct {

#ifdef MT76x2
	UCHAR E1gBeg[3];
	UCHAR E1gEnd[3];
	UCHAR E1aHighBeg[3];
	UCHAR E1aHighEnd[3];
	UCHAR E1aLowBeg[3];
	UCHAR E1aLowEnd[3];
	UCHAR E1aMidBeg[3];
	UCHAR E1aMidMid[3];
	UCHAR E1aMidEnd[3];
#endif				/* endif */
} ITXBF_LNA_PARAMS;		/* ITxBF BBP reg LNA calibration parameters */

typedef
	struct {

#ifdef MT76x2
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
#endif				/* endif */
} ITXBF_DIV_PARAMS;		/* ITxBF Divider Calibration parameters */

void ITxBFGetEEPROM(IN RTMP_ADAPTER * pAd,
		    IN ITXBF_PHASE_PARAMS * phaseParams,
		    IN ITXBF_LNA_PARAMS * lnaParams, IN ITXBF_DIV_PARAMS * divParams);

#ifdef MT76x2
INT ITxBFDividerCalibrationStartUp(IN RTMP_ADAPTER * pAd,
				   IN int calFunction, IN int calMethod, OUT UCHAR *divPhase);
#endif /* endif */

INT ITxBFDividerCalibration(IN RTMP_ADAPTER * pAd,
			    IN int calFunction, IN int calMethod, OUT UCHAR *divPhase);

VOID ITxBFLoadLNAComp(IN RTMP_ADAPTER * pAd);

int ITxBFLNACalibration(IN RTMP_ADAPTER * pAd,
			IN int calFunction, IN int calMethod, IN BOOLEAN gBand);

#ifdef MT76x2
INT mt76x2_ITxBFDividerCalibration(IN RTMP_ADAPTER * pAd,
				   IN int calFunction, IN int calMethod, OUT UCHAR *divPhase);

VOID mt76x2_ITxBFLoadLNAComp(IN RTMP_ADAPTER * pAd);

int mt76x2_ITxBFLNACalibration(IN RTMP_ADAPTER * pAd,
			       IN int calFunction, IN int calMethod, IN BOOLEAN gBand);

INT ITxBFLNACalibrationStartUp(IN RTMP_ADAPTER * pAd,
			       IN INT calFunction, IN INT calMethod, IN BOOLEAN gBand);

INT ITxBFPhaseCalibrationStartUp(IN RTMP_ADAPTER * pAd, IN INT calFunction, IN USHORT ch);

INT ITxBFPhaseCalibration(IN RTMP_ADAPTER * pAd, IN INT calFunction, IN USHORT ch);
#endif /* endif */

void Read_TxBfProfile(IN RTMP_ADAPTER * pAd,
		      IN PROFILE_DATA *prof, IN int profileNum, IN BOOLEAN implicitProfile);

void Write_TxBfProfile(IN RTMP_ADAPTER * pAd, IN PROFILE_DATA *prof, IN int profileNum);

void Read_TagField(IN PRTMP_ADAPTER pAd, IN UCHAR *row, IN int profileNum);

/* Write_TagField - write a profile tagfield */
void Write_TagField(IN RTMP_ADAPTER * pAd, IN UCHAR *row, IN int profileNum);

#ifdef MT76x2
INT TxBfProfileTagRead(IN PRTMP_ADAPTER pAd, IN PFMU_PROFILE * prof, IN UCHAR profileIdx);

INT TxBfProfileTagWrite(IN PRTMP_ADAPTER pAd, IN PFMU_PROFILE * prof, IN UCHAR profileIdx);

INT TxBfProfileDataRead(IN PRTMP_ADAPTER pAd,
			IN PFMU_DATA * pData, IN UCHAR profileIdx, IN UCHAR subcarrierIdx);

INT TxBfProfileDataWrite(IN PRTMP_ADAPTER pAd,
			 IN PFMU_DATA * pData, IN UCHAR profileIdx, IN UCHAR subcarrierIdx);

INT TxBfProfileTagValid(IN PRTMP_ADAPTER pAd, IN PFMU_PROFILE * prof, IN UCHAR profileIdx);
#endif /* endif */

/* displayTagfield - display one tagfield */
void displayTagfield(IN RTMP_ADAPTER * pAd, IN int profileNum, IN BOOLEAN implicitProfile);

/* Unpack an ITxBF matrix element from a row of bytes */
int Unpack_IBFValue(IN UCHAR *row, IN int elemNum);

int iCalcCalibration(IN RTMP_ADAPTER * pAd, IN int calParams[2], IN int profileNum);

void ITxBFSetEEPROM(IN RTMP_ADAPTER * pAd,
		    IN ITXBF_PHASE_PARAMS * phaseParams,
		    IN ITXBF_LNA_PARAMS * lnaParams, IN ITXBF_DIV_PARAMS * divParams);

#endif /* TXBF_SUPPORT // */

#endif /* _RT_TXBF_H_ */
