/*
 / _____)             _              | |
 ( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
 (______/|_____)_|_|_| \__)_____)\____)_| |_|
 (C)2013 Semtech

 Description: LoRaMac classA device implementation

 License: Revised BSD License, see LICENSE.TXT file include in the project

 Maintainer: Miguel Luis and Gregory Cristian
 */

/*! \file classA/SensorNode/main.c */

#include <string.h>
#include <math.h>
#include "board.h"

#include "LoRaMac.h"
#include "Region.h"
#include "Commissioning.h"

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            10000

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                        500

/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON                    false

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1

#if defined( REGION_EU868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        true

//#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          1

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )

#define LC4                { 867100000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, 0, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, 0, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT                            2

/*!
 * User application data buffer size
 */
#if defined( REGION_CN470 ) || defined( REGION_CN779 ) || defined( REGION_EU433 ) || defined( REGION_EU868 ) || defined( REGION_IN865 ) || defined( REGION_KR920 )

#define LORAWAN_APP_DATA_SIZE                       16

#elif defined( REGION_AS923 ) || defined( REGION_AU915 ) || defined( REGION_US915 ) || defined( REGION_US915_HYBRID )

#define LORAWAN_APP_DATA_SIZE                       11

#else

#error "Please define a region in the compiler options."

#endif

static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#if( OVER_THE_AIR_ACTIVATION == 0 )

static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif

/*!
 * Application port
 */
static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
 * User application data size
 */
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE                           242

/*!
 * User application data
 */
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

/*!
 * Device states
 */
static enum eDeviceState {
	DEVICE_STATE_INIT,
	DEVICE_STATE_JOIN,
	DEVICE_STATE_SEND,
	DEVICE_STATE_CYCLE,
	DEVICE_STATE_SLEEP
} DeviceState;

/*!
 * LoRaWAN compliance tests support data //合规性测试
 */
struct ComplianceTest_s {
	bool Running;
	uint8_t State;
	bool IsTxConfirmed;
	uint8_t AppPort;
	uint8_t AppDataSize;
	uint8_t *AppDataBuffer;
	uint16_t DownLinkCounter;
	bool LinkCheck;
	uint8_t DemodMargin;
	uint8_t NbGateways;
} ComplianceTest;

void dump_hex2str(uint8_t *buf, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		printf("%02X ", buf[i]);
	}
	printf("\r\n");
}

/*!
 * \brief   Prepares the payload of the frame
 */
uint8_t GPS_GETFAIL = FAIL;
void test_gps(void)
{
	  double  latitude, longitude = 0;
    int16_t altitudeGps = 0xFFFF;
    uint8_t ret;
	  ret = GpsGetLatestGpsPositionDouble( &latitude, &longitude );
		altitudeGps = GpsGetLatestGpsAltitude( );                           // in m
		printf("[Debug]: latitude: %f, longitude: %f , altitudeGps: %d \n", latitude, longitude, altitudeGps);	    	
}

static void PrepareTxFrame(uint8_t port) {
	double latitude, longitude = 0;
	int16_t altitudeGps = 0xFFFF;
//	int8_t tempr = 25;
	uint8_t ret;
	uint16_t vdd2;
	uint16_t vdd3;
	uint16_t vdd4;

	switch (port) {
		//https://mydevices.com/cayenne/docs/lora/#lora-cayenne-low-power-payload
		//cayenne LPP GPS
	case 2: {
		ret = GpsGetLatestGpsPositionDouble(&latitude, &longitude);
		altitudeGps = GpsGetLatestGpsAltitude(); // in m
		//printf("[Debug]: latitude: %f, longitude: %f , altitudeGps: %d \n", latitude, longitude, altitudeGps);
		printf("GpsGetLatestGpsPositionDouble ret = %d\r\n", ret);
		if (ret == SUCCESS) {
			AppData[0] = 0x01;	//Data Channel
			AppData[1] = 0x88;	//Type: 88 = GPSLocation
			AppData[2] = ((int32_t) (latitude * 10000) >> 16) & 0xFF;
			AppData[3] = ((int32_t) (latitude * 10000) >> 8) & 0xFF;
			AppData[4] = ((int32_t) (latitude * 10000)) & 0xFF;
			AppData[5] = ((int32_t) (longitude * 10000) >> 16) & 0xFF;
			AppData[6] = ((int32_t) (longitude * 10000) >> 8) & 0xFF;
			AppData[7] = ((int32_t) (longitude * 10000)) & 0xFF;
			AppData[8] = ((altitudeGps * 100) >> 16) & 0xFF;
			AppData[9] = ((altitudeGps * 100) >> 8) & 0xFF;
			AppData[10] = (altitudeGps * 100) & 0xFF;
			AppDataSize = 11;
		} else {
			AppDataSize = 0;
			GPS_GETFAIL = SUCCESS;
		}
	}
		break;
		//cayenne LPP Temp; Acceleration
//	case 3: {
//		AppData[0] = 0x02;	//Data Channel
//		AppData[1] = 0x67;	//Type: 67 = Temperature
//		LIS3DH_GetTempRaw(&tempr); //only tempr changed value
//		tempr = tempr + 20; // temprature should be calibration in a right temp for every device
//		AppData[2] = ((tempr * 10) >> 8) & 0xFF;
//		AppData[3] = (tempr * 10) & 0xFF;
//		printf("[Debug]: tempr: %d \r\n", tempr);
//		
//		AppData[4] = 0x03;	//Data Channel
//		AppData[5] = 0x71;	//Type: 71 = Accelerometer
//		for (uint8_t index = 0; index < 6; index++) {
//			LIS3DH_ReadReg(LIS3DH_OUT_X_L + index, AppData + 6 + index);
//			DelayMs(2);
//		}
//		AppDataSize = 12;
//		printf("[Debug]: ACC X:%04X Y:%04X Z:%04X\r\n",
//				AppData[7] << 8 | AppData[6], 
//				AppData[9] << 8 | AppData[8],
//				AppData[11] << 8 | AppData[10]);
//	}
//		break;
	//Customize:
	case 3:{
		vdd2 = CustomizeMeasureVolage(EXTERNAL_ADC_PA1);//绿色 PH
		vdd2 = vdd2*0.028;
		AppData[0] = 0x02;	//Data Channel
		AppData[1] = 0x03;	
		AppData[2] = (vdd2*10 >> 8) & 0xFF; //@Suvan|| ADC_CHANNEL_2 EXTERNAL_ADC_PA1
		AppData[3] = (vdd2*10) & 0xFF;
		
		vdd3 = CustomizeMeasureVolage(EXTERNAL_ADC_PB12); //浊度
		AppData[4] = 0x03;	//Data Channel
		AppData[5] = 0x03;	
		AppData[6] = (vdd3 >> 8) & 0xFF; //@Suvan|| ADC_CHANNEL_18 EXTERNAL_ADC_PB12
		AppData[7] = (vdd3) & 0xFF;
		
		vdd4 = CustomizeMeasureVolage(BAT_LEVEL_CHANNEL);//白色 温度
		vdd4 = vdd4*0.2-200;
		AppData[8] = 0x04;	//Data Channel
		AppData[9] = 0x03;	
		AppData[10] = (vdd4*10 >> 8) & 0xFF; //@Suvan|| ADC_CHANNEL_20 BAT_LEVEL_CHANNEL
		AppData[11] = (vdd4*10) & 0xFF; 
		AppDataSize = 12;
		printf("Turbidity: %.1f,   Temperature: %.1f C,    PH: %.1f \r\n", vdd3*0.1, vdd4*0.1, vdd2*0.1);
	}
    	break;
	case 224:
		if (ComplianceTest.LinkCheck == true) {
			ComplianceTest.LinkCheck = false;
			AppDataSize = 3;
			AppData[0] = 5;
			AppData[1] = ComplianceTest.DemodMargin;
			AppData[2] = ComplianceTest.NbGateways;
			ComplianceTest.State = 1;
		} else {
			switch (ComplianceTest.State) {
			case 4:
				ComplianceTest.State = 1;
				break;
			case 1:
				AppDataSize = 2;
				AppData[0] = ComplianceTest.DownLinkCounter >> 8;
				AppData[1] = ComplianceTest.DownLinkCounter;
				break;
			}
		}
		break;
	default:
		break;
	}
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame(void) {
	McpsReq_t mcpsReq;
	LoRaMacTxInfo_t txInfo;

	if (LoRaMacQueryTxPossible(AppDataSize, &txInfo) != LORAMAC_STATUS_OK) {
		// Send empty frame in order to flush MAC commands
		mcpsReq.Type = MCPS_UNCONFIRMED;
		mcpsReq.Req.Unconfirmed.fBuffer = NULL;
		mcpsReq.Req.Unconfirmed.fBufferSize = 0;
		mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
	} else {
		if (IsTxConfirmed == false) {
			mcpsReq.Type = MCPS_UNCONFIRMED;
			mcpsReq.Req.Unconfirmed.fPort = AppPort;
			mcpsReq.Req.Unconfirmed.fBuffer = AppData;
			mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
		} else {
			mcpsReq.Type = MCPS_CONFIRMED;
			mcpsReq.Req.Confirmed.fPort = AppPort;
			mcpsReq.Req.Confirmed.fBuffer = AppData;
			mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Confirmed.NbTrials = 8;
			mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
		}
	}

	if (LoRaMacMcpsRequest(&mcpsReq) == LORAMAC_STATUS_OK) {
		return false;
	}
	return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent(void) {
	MibRequestConfirm_t mibReq;
	LoRaMacStatus_t status;

	TimerStop(&TxNextPacketTimer);

	mibReq.Type = MIB_NETWORK_JOINED;
	status = LoRaMacMibGetRequestConfirm(&mibReq);

	if (status == LORAMAC_STATUS_OK) {
		if (mibReq.Param.IsNetworkJoined == true) {
			DeviceState = DEVICE_STATE_SEND;
			NextTx = true;
		} else {
			DeviceState = DEVICE_STATE_JOIN;
		}
	}
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent(void) {
	TimerStop(&Led1Timer);
	// Toggle LED 1
	GpioToggle(&Led1);
	TimerStart(&Led1Timer);
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent(void) {
	TimerStop(&Led2Timer);
	// Switch LED 2 OFF
	GpioWrite(&Led2, 1);
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm(McpsConfirm_t *mcpsConfirm) {
	if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
		switch (mcpsConfirm->McpsRequest) {
		case MCPS_UNCONFIRMED: {
			// Check Datarate
			// Check TxPower
			break;
		}
		case MCPS_CONFIRMED: {
			// Check Datarate
			// Check TxPower
			// Check AckReceived
			// Check NbTrials
			break;
		}
		case MCPS_PROPRIETARY: {
			break;
		}
		default:
			break;
		}

	}
	NextTx = true;
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication(McpsIndication_t *mcpsIndication) {
	if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK) {
		return;
	}

	switch (mcpsIndication->McpsIndication) {
	case MCPS_UNCONFIRMED: {
		break;
	}
	case MCPS_CONFIRMED: {
		break;
	}
	case MCPS_PROPRIETARY: {
		break;
	}
	case MCPS_MULTICAST: {
		break;
	}
	default:
		break;
	}

	// Check Multicast
	// Check Port
	// Check Datarate
	// Check FramePending
	// Check Buffer
	// Check BufferSize
	// Check Rssi
	// Check Snr
	// Check RxSlot

	if (ComplianceTest.Running == true) {
		ComplianceTest.DownLinkCounter++;
	}

	if (mcpsIndication->RxData == true) {
		switch (mcpsIndication->Port) {
		case 1: // The application LED can be controlled on port 1 or 2
		case 2:
			if (mcpsIndication->BufferSize == 1) {
				AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
				GpioWrite(&Led2, ((AppLedStateOn & 0x01) != 0) ? 0 : 1);
			}
			break;
		case 224:
			if (ComplianceTest.Running == false) {
				// Check compliance test enable command (i)
				if ((mcpsIndication->BufferSize == 4)
						&& (mcpsIndication->Buffer[0] == 0x01)
						&& (mcpsIndication->Buffer[1] == 0x01)
						&& (mcpsIndication->Buffer[2] == 0x01)
						&& (mcpsIndication->Buffer[3] == 0x01)) {
					IsTxConfirmed = false;
					AppPort = 224;
					AppDataSize = 2;
					ComplianceTest.DownLinkCounter = 0;
					ComplianceTest.LinkCheck = false;
					ComplianceTest.DemodMargin = 0;
					ComplianceTest.NbGateways = 0;
					ComplianceTest.Running = true;
					ComplianceTest.State = 1;

					MibRequestConfirm_t mibReq;
					mibReq.Type = MIB_ADR;
					mibReq.Param.AdrEnable = true;
					LoRaMacMibSetRequestConfirm(&mibReq);

#if defined( REGION_EU868 )
					LoRaMacTestSetDutyCycleOn( false );
#endif
					GpsStop();
				}
			} else {
				ComplianceTest.State = mcpsIndication->Buffer[0];
				switch (ComplianceTest.State) {
				case 0: // Check compliance test disable command (ii)
					IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
					AppPort = LORAWAN_APP_PORT;
					AppDataSize = LORAWAN_APP_DATA_SIZE;
					ComplianceTest.DownLinkCounter = 0;
					ComplianceTest.Running = false;

					MibRequestConfirm_t mibReq;
					mibReq.Type = MIB_ADR;
					mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
					LoRaMacMibSetRequestConfirm(&mibReq);
#if defined( REGION_EU868 )
					LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif
					GpsStart();
					break;
				case 1: // (iii, iv)
					AppDataSize = 2;
					break;
				case 2: // Enable confirmed messages (v)
					IsTxConfirmed = true;
					ComplianceTest.State = 1;
					break;
				case 3: // Disable confirmed messages (vi)
					IsTxConfirmed = false;
					ComplianceTest.State = 1;
					break;
				case 4: // (vii)
					AppDataSize = mcpsIndication->BufferSize;

					AppData[0] = 4;
					for (uint8_t i = 1;
							i < MIN(AppDataSize, LORAWAN_APP_DATA_MAX_SIZE);
							i++) {
						AppData[i] = mcpsIndication->Buffer[i] + 1;
					}
					break;
				case 5: // (viii)
				{
					MlmeReq_t mlmeReq;
					mlmeReq.Type = MLME_LINK_CHECK;
					LoRaMacMlmeRequest(&mlmeReq);
				}
					break;
				case 6: // (ix)
				{
					MlmeReq_t mlmeReq;

					// Disable TestMode and revert back to normal operation
					IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
					AppPort = LORAWAN_APP_PORT;
					AppDataSize = LORAWAN_APP_DATA_SIZE;
					ComplianceTest.DownLinkCounter = 0;
					ComplianceTest.Running = false;

					MibRequestConfirm_t mibReq;
					mibReq.Type = MIB_ADR;
					mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
					LoRaMacMibSetRequestConfirm(&mibReq);
#if defined( REGION_EU868 )
					LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
#endif
					GpsStart();

					mlmeReq.Type = MLME_JOIN;

					mlmeReq.Req.Join.DevEui = DevEui;
					mlmeReq.Req.Join.AppEui = AppEui;
					mlmeReq.Req.Join.AppKey = AppKey;
					mlmeReq.Req.Join.NbTrials = 3;

					LoRaMacMlmeRequest(&mlmeReq);
					DeviceState = DEVICE_STATE_SLEEP;
				}
					break;
				case 7: // (x)
				{
					if (mcpsIndication->BufferSize == 3) {
						MlmeReq_t mlmeReq;
						mlmeReq.Type = MLME_TXCW;
						mlmeReq.Req.TxCw.Timeout = (uint16_t)(
								(mcpsIndication->Buffer[1] << 8)
										| mcpsIndication->Buffer[2]);
						LoRaMacMlmeRequest(&mlmeReq);
					} else if (mcpsIndication->BufferSize == 7) {
						MlmeReq_t mlmeReq;
						mlmeReq.Type = MLME_TXCW_1;
						mlmeReq.Req.TxCw.Timeout = (uint16_t)(
								(mcpsIndication->Buffer[1] << 8)
										| mcpsIndication->Buffer[2]);
						mlmeReq.Req.TxCw.Frequency =
								(uint32_t) ((mcpsIndication->Buffer[3] << 16)
										| (mcpsIndication->Buffer[4] << 8)
										| mcpsIndication->Buffer[5]) * 100;
						mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
						LoRaMacMlmeRequest(&mlmeReq);
					}
					ComplianceTest.State = 1;
				}
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	// Switch LED 1 ON for each received downlink
	GpioWrite(&Led1, 0);
	TimerStart(&Led1Timer);
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm(MlmeConfirm_t *mlmeConfirm) {
	switch (mlmeConfirm->MlmeRequest) {
	case MLME_JOIN: {
		if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
			// Status is OK, node has joined the network
			DeviceState = DEVICE_STATE_SEND;
			printf("OTAA Join Success \r\n");
			// Switch LED 1 ON
			GpioWrite(&Led1, 0);
			TimerStart(&Led1Timer);
		} else {
			// Join was not successful. Try to join again
			DeviceState = DEVICE_STATE_JOIN;
		}
		break;
	}
	case MLME_LINK_CHECK: {
		if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
			// Check DemodMargin
			// Check NbGateways
			if (ComplianceTest.Running == true) {
				ComplianceTest.LinkCheck = true;
				ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
				ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
			}
		}
		break;
	}
	default:
		break;
	}
	NextTx = true;
}

/**
 * Main application entry point.
 */
int main(void) {
//		uint16_t vdd2;
//	uint16_t vdd3;
//	uint16_t vdd4;
	LoRaMacPrimitives_t LoRaMacPrimitives;
	LoRaMacCallback_t LoRaMacCallbacks;
	MibRequestConfirm_t mibReq;

	BoardInitMcu();
	BoardInitPeriph();

	DeviceState = DEVICE_STATE_INIT;
	printf("RAK811 BreakBoard soft version: 1.0.3\r\n");

	while (1) {
		switch (DeviceState) {
		case DEVICE_STATE_INIT: {
			LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
			LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
			LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
			LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
#if defined( REGION_AS923 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_AS923 );
#elif defined( REGION_AU915 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_AU915 );
#elif defined( REGION_CN470 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_CN470 );
#elif defined( REGION_CN779 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_CN779 );
#elif defined( REGION_EU433 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_EU433 );
#elif defined( REGION_EU868 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_EU868 );
#elif defined( REGION_IN865 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_IN865 );
#elif defined( REGION_KR920 )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_KR920 );
#elif defined( REGION_US915 )
			LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks,
					LORAMAC_REGION_US915);
#elif defined( REGION_US915_HYBRID )
			LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_US915_HYBRID );
#else
#error "Please define a region in the compiler options."
#endif
			TimerInit(&TxNextPacketTimer, OnTxNextPacketTimerEvent);

			TimerInit(&Led1Timer, OnLed1TimerEvent);
			TimerSetValue(&Led1Timer, 1000);

			TimerInit(&Led2Timer, OnLed2TimerEvent);
			TimerSetValue(&Led2Timer, 150);

			mibReq.Type = MIB_ADR;
			mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
			LoRaMacMibSetRequestConfirm(&mibReq);

			mibReq.Type = MIB_PUBLIC_NETWORK;
			mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
			LoRaMacMibSetRequestConfirm(&mibReq);

			DeviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN: {
#if( OVER_THE_AIR_ACTIVATION != 0 )
			MlmeReq_t mlmeReq;

			// Initialize LoRaMac device unique ID
			//BoardGetUniqueId( DevEui );

			printf("OTAA: \r\n");
			printf("Dev_EUI: ");
			dump_hex2str(DevEui, 8);
			printf("AppEui: ");
			dump_hex2str(AppEui, 8);
			printf("AppKey: ");
			dump_hex2str(AppKey, 16);

			mlmeReq.Type = MLME_JOIN;

			mlmeReq.Req.Join.DevEui = DevEui;
			mlmeReq.Req.Join.AppEui = AppEui;
			mlmeReq.Req.Join.AppKey = AppKey;
			mlmeReq.Req.Join.NbTrials = 3;

#if defined ( REGION_US915 )  // Used in TTN US915 band.
			uint16_t ch_mask[5];
			ch_mask[0] = 0xff00;
			ch_mask[1] = 0x0000;
			ch_mask[2] = 0x0000;
			ch_mask[3] = 0x0000;
			ch_mask[4] = 0x0000;

			mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
			mibReq.Param.ChannelsDefaultMask = ch_mask;
			LoRaMacMibSetRequestConfirm(&mibReq);

			mibReq.Type = MIB_CHANNELS_MASK;
			mibReq.Param.ChannelsDefaultMask = ch_mask;
			LoRaMacMibSetRequestConfirm(&mibReq);
#endif

			if (NextTx == true) {
				LoRaMacMlmeRequest(&mlmeReq);
				printf("OTAA Join Start... \r\n");
			}
			DeviceState = DEVICE_STATE_SLEEP;

#else
			// Choose a random device address if not already defined in Commissioning.h
			if( DevAddr == 0 )
			{
				// Random seed initialization
				srand1( BoardGetRandomSeed( ) );

				// Choose a random device address
				DevAddr = randr( 0, 0x01FFFFFF );
			}

			printf("ABP: \r\n");
			printf("Dev_EUI: ");
			dump_hex2str(DevEui , 8);
			printf("DevAddr: %08X\r\n", DevAddr);
			printf("NwkSKey: ");
			dump_hex2str(NwkSKey , 16);
			printf("AppSKey: ");
			dump_hex2str(AppSKey , 16);

			mibReq.Type = MIB_NET_ID;
			mibReq.Param.NetID = LORAWAN_NETWORK_ID;
			LoRaMacMibSetRequestConfirm( &mibReq );

			mibReq.Type = MIB_DEV_ADDR;
			mibReq.Param.DevAddr = DevAddr;
			LoRaMacMibSetRequestConfirm( &mibReq );

			mibReq.Type = MIB_NWK_SKEY;
			mibReq.Param.NwkSKey = NwkSKey;
			LoRaMacMibSetRequestConfirm( &mibReq );

			mibReq.Type = MIB_APP_SKEY;
			mibReq.Param.AppSKey = AppSKey;
			LoRaMacMibSetRequestConfirm( &mibReq );

			mibReq.Type = MIB_NETWORK_JOINED;
			mibReq.Param.IsNetworkJoined = true;
			LoRaMacMibSetRequestConfirm( &mibReq );

			// Switch LED 1 ON
			GpioWrite( &Led1, 0 );
			TimerStart( &Led1Timer );
			DeviceState = DEVICE_STATE_SEND;
#endif
			break;
		}
		case DEVICE_STATE_SEND: {
			if (NextTx == true) {
				PrepareTxFrame(AppPort);

				if (GPS_GETFAIL == SUCCESS) {
					GPS_GETFAIL = FAIL;
				} else {
					NextTx = SendFrame();
				}

				AppPort++;
				if (AppPort >= 4) {
					AppPort = 2;
				}
			}
			if (ComplianceTest.Running == true) {
				// Schedule next packet transmission
				TxDutyCycleTime = 3000; // 5000 ms
			} else {
				// Schedule next packet transmission
				TxDutyCycleTime = APP_TX_DUTYCYCLE
						+ randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
			}
			DeviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE: {
			DeviceState = DEVICE_STATE_SLEEP;

			// Schedule next packet transmission
			TimerSetValue(&TxNextPacketTimer, TxDutyCycleTime);
			TimerStart(&TxNextPacketTimer);
			break;
		}
		case DEVICE_STATE_SLEEP: {
			// Wake up through events
			TimerLowPowerHandler();
			break;
		}
		default: {
			DeviceState = DEVICE_STATE_INIT;
			break;
		}
		}
		if (GpsGetPpsDetectedState() == true) {
			// Switch LED 2 ON
			GpioWrite(&Led2, 0);
			TimerStart(&Led2Timer);
		}
//		if (Lis3dhGetIntState() == true) {
//			Lis3dh_IntEventClear();
//			for (uint8_t index = 0; index < 6; index++) {
//				LIS3DH_ReadReg(LIS3DH_OUT_X_L + index, AppData + 2 + index);
//				DelayMs(1);
//			}
//			//printf("[Debug]: ACC X:%04X Y:%04X Z:%04X\r\n", AppData[3]<<8 | AppData[2], AppData[5]<<8 | AppData[4], AppData[7]<<8 | AppData[6]);
//		}
	}
}
