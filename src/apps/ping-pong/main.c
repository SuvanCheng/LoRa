/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"


#if defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             14        // dBm

#define   USE_MODEM_LORA     1
#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/**
 * Main application entry point.
 */
int pingpong_test( void )
{
    bool isMaster = true; // bool isMaster = false;
    uint8_t i;

    // Target board initialization
    BoardInitMcu( );
		printf("Ping-Pong Test Start.\r\n");
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );
    //Radio.GetStatus();
    Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
																	 
    printf("You set the Frq:%d,Sf:%d,Pwr:%d,Bw:%d,Cr:%d\r\n",RF_FREQUENCY,LORA_SPREADING_FACTOR,TX_OUTPUT_POWER,LORA_BANDWIDTH,LORA_CODINGRATE);
		
#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

    Radio.Rx( RX_TIMEOUT_VALUE );

    while( 1 )
    {
        switch( State )
        {
        case RX:
            if( isMaster == true )
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                    {
                        // Indicates on a LED that the received frame is a PONG
                        GpioWrite( &Led1, GpioRead( &Led1 ) ^ 1 );

                        // Send the next PING frame
                        Buffer[0] = 'P';
                        Buffer[1] = 'I';
                        Buffer[2] = 'N';
                        Buffer[3] = 'G';
                        // We fill the buffer with numbers for the payload
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        DelayMs( 1 );
                        Radio.Send( Buffer, BufferSize );
                    }
                    else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    { // A master already exists then become a slave
                        isMaster = false;
                        GpioWrite( &Led2, 1 ); // Set LED off
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }
                    else // valid reception but neither a PING or a PONG message
                    {    // Set device as master ans start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }
                }
            }
            else
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    {
                        // Indicates on a LED that the received frame is a PING
                        GpioWrite( &Led1, GpioRead( &Led1 ) ^ 1 );

                        // Send the reply to the PONG string
                        Buffer[0] = 'P';
                        Buffer[1] = 'O';
                        Buffer[2] = 'N';
                        Buffer[3] = 'G';
                        // We fill the buffer with numbers for the payload
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        DelayMs( 1 );
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but not a PING as expected
                    {    // Set device as master and start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }
                }
            }
            State = LOWPOWER;
            break;
        case TX:
            // Indicates on a LED that we have sent a PING [Master]
            // Indicates on a LED that we have sent a PONG [Slave]
            GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            if( isMaster == true )
            {
                // Send the next PING frame
                Buffer[0] = 'P';
                Buffer[1] = 'I';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                DelayMs( 1 );
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                Radio.Rx( RX_TIMEOUT_VALUE );
            }
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }

        TimerLowPowerHandler( );

    }
}

bool RX_flg = false;
bool TX_flg = false;
bool lora_cli_loop();
int parse_args(char* str, char* argv[]);
int parase_str(char* str);
char    rx_buffer[64];

typedef struct _lorap2p_param{
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Spreading factor
     * [6, 7, 8, 9, 10, 11, 12]
     */
    uint8_t  Spreadfact;	
    /*!
     * Bandwidth
     * [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]                              
     */
    uint8_t  Bandwidth; 
    /*!
     * Coding Rate
     * [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     */
    uint8_t  Codingrate; 
    /*!
     * Preamble Length
     * [5-65535]
     */
    uint16_t  Preamlen; 
    /*!
     * Power dbm
     * [5-20]
     */
    uint8_t  Powerdbm;	
}S_LORAP2P_PARAM;

S_LORAP2P_PARAM sLoraP2PPram;

int lorap2p_test(void)
{
	 uint8_t i;

    // Target board initialization
    BoardInitMcu( );
		printf("LoRa2P2 Test Start.\r\n");
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );
    //Radio.GetStatus();
	  Radio.SetPublicNetwork(false);
	while (true)
    {
			while(lora_cli_loop());

			if (TX_flg)
			{
				
				SX1276SetChannel( sLoraP2PPram.Frequency );  
		    SX1276SetTxConfig( MODEM_LORA, sLoraP2PPram.Powerdbm, 0, sLoraP2PPram.Bandwidth,
                                   sLoraP2PPram.Spreadfact, sLoraP2PPram.Codingrate,
                                   sLoraP2PPram.Preamlen, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000000 );
				DelayMs(1000);
 
				 while (1)
				 {
					Buffer[0] = 'P';
					Buffer[1] = 'I';
					Buffer[2] = 'N';
					Buffer[3] = 'G';
					SX1276Send(Buffer,4);
					DelayMs(1000);
				 }
      }
			else if (RX_flg)
			{
				SX1276SetChannel( sLoraP2PPram.Frequency );  
				SX1276SetRxConfig( MODEM_LORA, sLoraP2PPram.Bandwidth, sLoraP2PPram.Spreadfact,
                                   sLoraP2PPram.Codingrate, 0, sLoraP2PPram.Preamlen,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
				SX1276SetMaxPayloadLength(MODEM_LORA,255);
				SX1276SetRx(0);
				while(1){
				   DelayMs(1000);
				}
			}
	 }																
}

bool lora_cli_loop(void)
{
    uint8_t cr = '\0';
    static int i = 0;

    if(UartMcuGetChar(&Uart1, &cr) == 0)
		{
			if (cr != '\0')
			{
				rx_buffer[i] = cr;
			  i++;
			}
			if (cr == '\n')
			{
				printf("%s",rx_buffer);
				i = 0;
        if(parase_str(rx_buffer) ==0) {
				  return false;
				} else {
					printf("Invalid  Please enter again ....\r\n");
				  return true;
				}
			}
		}
		else
		{
			return true;
		}
}

int parse_args(char* str, char* argv[])
{
    int i = 0;
    char* ch = str;

    while(*ch != '\0') {
        i++;
        /*Check if length exceeds*/
        if (i > 10) {
            return 0;
        }

        argv[i-1] = ch;
        		
        while(*ch != ',' && *ch != '\0' && *ch != '\r') {
                ch++;
        }
        if (*ch == '\r')
            break;
				
        if (*ch != '\0') {
            *ch = '\0';
					  printf("parm: i=%d, %s \r\n", i-1, argv[i-1]);
            ch++;
            while(*ch == ',') {
                ch++;
            }
        }
    }
    return i;
}


int parase_str(char* str)
{
    int argc;
    char* argv[10]={NULL};
		
    argc = parse_args(str, argv);
		
		for(int i=0; i<argc; i++)
		{
			DelayMs(200);
			switch(i) {
				case 0:
				{
				    if(0 == strcmp(argv[i], "tx"))
						{
						   TX_flg = 1;
							 RX_flg = 0;
						}	else if (0 == strcmp(argv[i], "rx"))
						{
							TX_flg = 0;
              RX_flg = 1;
						}	else {
								TX_flg = 0;
                RX_flg = 0;
						   return -1;
						}					
				}
			  break;
				case 1:  //freq
				{
				  sLoraP2PPram.Frequency = atoi(argv[i]);
					printf("freq: %d \r\n", sLoraP2PPram.Frequency );
				}
			  break;
				case 2: //sf
				{
				   sLoraP2PPram.Spreadfact = atoi(argv[i]);
					 printf("sf: %d [6, 7, 8, 9, 10, 11, 12] \r\n", sLoraP2PPram.Spreadfact );
				}
			  break;
				case 3:
				{
				   sLoraP2PPram.Bandwidth = atoi(argv[i]);
					 printf("Bandwidth: %d [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]\r\n", sLoraP2PPram.Bandwidth );
				}
			  break;
				case 4:
				{
				   sLoraP2PPram.Codingrate = atoi(argv[i]);
					 printf("Codingrate: %d [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]\r\n", sLoraP2PPram.Codingrate );
				}
			  break;
				case 5:
				{
				   sLoraP2PPram.Preamlen = atoi(argv[i]);
					 printf("Preamlen: %d [5-65535]\r\n", sLoraP2PPram.Preamlen );
				}
			  break;
				case 6:
				{
				   sLoraP2PPram.Powerdbm = atoi(argv[i]);
					 printf("Powerdbm: %d [5-20]\r\n", sLoraP2PPram.Powerdbm );
				}
			  break;
				default:
					break;
			}

		}
		
		if(argc == 7) {	
		  	return 0;
		}
			
		return -1;
}


int main (void)
{
	 lorap2p_test();
	 //pingpong_test();
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
    printf("OnTxDone\r\n");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    printf("OnRxDone rssi=%d snr=%d size=%d %s\r\n",RssiValue,SnrValue,BufferSize,Buffer);
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
    printf("OnTxTimeout\r\n");
}

void OnRxTimeout( void )
{
    State = RX_TIMEOUT;
    printf("OnRxTimeout\r\n");
}

void OnRxError( void )
{
    State = RX_ERROR;
    printf("OnRxError\r\n");
}


