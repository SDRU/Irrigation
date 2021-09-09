/****************************************************************************
 *   FILE -      prot_mid.c
 *
 *   DESCRIPTION -
 *
 *       This program implements the mid-level of the Tescom protocol.
 *
 ****/


/******** INCLUDES ********/

#include <windows.h>
#include "protocol.h"
#include "prot_lo.h"
#include "prot_mid.h"


/******** DEFINES ********/

#define MASTER_ADDRESS    0
#define BROADCAST_ADDRESS 255

#define TIMEOUT     25

#define GET_ADDRESS     0
#define GET_LENGTH      GET_ADDRESS+1
#define GET_DATA        GET_LENGTH+1
#define GET_CRC_HIGH    GET_DATA+1
#define GET_CRC_LOW     GET_CRC_HIGH+1

#define HEADER_LENGTH   2
#define CRC_LENGTH      2

#define WAIT_TIME       1  /* Wait time at end of packet */
#define RESYNC_TIME     5  /* Time for controller to resync after error */
#define BYTE_TIME       2  /* ms. per byte sent allowed before response */

#define MAX_PACKET_SIZE 259

#define ADDRESS_BYTE    0

/******** GLOBALS ********/

USHORT crc_sum ;
int retries_remaining ;

/******** PROTOTYPES ********/

void init_crc( void ) ;
void add_crc( UCHAR current_byte ) ;
UCHAR packet_transceive_single( UCHAR *transmit_buffer,
                                UCHAR *packet, int length ) ;


/****************************************************************************
 * FUNCTION : mid_startup
 *
 *      Network startup procedure for mid layer.
 */

UCHAR  mid_startup( void )
{
    return lo_startup();

} /* end of mid_startup() */


/****************************************************************************
 * FUNCTION : mid_shutdown
 *
 *      Network shutdown procedure for mid layer.
 */

UCHAR mid_shutdown( void )
{
    return lo_shutdown();

} /* end of mid_shutdown() */


/****************************************************************************
 * FUNCTION : packet_transceive
 *
 *      Sends a packet over the network and receives a response.  Returns
 *      TRUE if successful.  FALSE if timeout or bad CRC received.  Uses
 *      global variable "Retries" to determine maximum number of attempts.
 */

UCHAR packet_transceive( UCHAR *packet )
{
    UCHAR   status ;
    UCHAR   transmit_buffer[ MAX_PACKET_SIZE ] ;
    UCHAR   address ;
    UCHAR   length ;
    UCHAR   *data ;
    int     byte_count ;

    retries_remaining = Retries+1 ;

    data = packet ;
    address = *data++;
    length  = *data++;

    /* Construct Transmit packet */
    init_crc() ;
    byte_count = 0 ;
    transmit_buffer[ byte_count++ ] = address ;
    add_crc( address ) ;
    transmit_buffer[ byte_count++ ] = length ;
    add_crc( length ) ;
    do {
        transmit_buffer[ byte_count++ ] = *data ;
        add_crc( *data++ ) ;
    } while( byte_count < length+HEADER_LENGTH ) ;
    transmit_buffer[ byte_count++ ] = HI_BYTE( crc_sum ) ;
    transmit_buffer[ byte_count++ ] = LO_BYTE( crc_sum ) ;

    do {
        status = packet_transceive_single( transmit_buffer, packet , length ) ;
        retries_remaining-- ;
    } while( retries_remaining && !status ) ;

    return status ;

} /* end of packet_transceive() */


/****************************************************************************
 * FUNCTION : packet_transceive_single
 *
 *      Sends a single packet over the network and receives a response.  
 *      Returns TRUE if successful.  FALSE if timeout or bad CRC received.
 */

UCHAR packet_transceive_single( UCHAR *transmit_buffer,
                                UCHAR *packet, int length )
{
    UCHAR   address ;
    int     byte_count ;
    char    recv_state ;
    USHORT  recv_crc ;
    UINT16    timeout ;
    int     rcv_length ;

    /* Clear receive buffer of UART */
    while( recv_byte() ) {
        get_byte() ;
    }

    address = transmit_buffer[ ADDRESS_BYTE ] ;

    packet_transmit( transmit_buffer , length + HEADER_LENGTH + CRC_LENGTH ) ;

    if( address == BROADCAST_ADDRESS )
        return TRUE ;

    init_timer() ;
    timeout = ( length + HEADER_LENGTH + CRC_LENGTH ) * BYTE_TIME + TIMEOUT;


    /* Wait for return packet */

    init_crc() ;
    byte_count = 0 ;
    recv_state = GET_ADDRESS ;

    do {
    do {
    	/* Poll serial port */
    	if( recv_byte() ) {
    	    switch( recv_state ) {
    
    	    case GET_ADDRESS:
                address = get_byte() ;
                *packet = address ;
                recv_state = GET_LENGTH ;
                add_crc( *packet ) ;
                packet++ ;
                init_timer() ;
                timeout = TIMEOUT ;
                break ;
    
    	    case GET_LENGTH:
                rcv_length = get_byte() ;
                *packet = rcv_length ;
                add_crc( *packet ) ;
                packet++ ;
                if( rcv_length == 0 )
                    recv_state = GET_CRC_HIGH ;
                else
                    recv_state = GET_DATA ;
                init_timer() ;
                break ;

    	    case GET_DATA:
                *packet = get_byte() ;
                byte_count++ ;
                add_crc( *packet ) ;
                packet++ ;
                if( byte_count >= rcv_length ) {
                    recv_state = GET_CRC_HIGH ;
                }
                init_timer() ;
                break ;

            case GET_CRC_HIGH:
                recv_crc = get_byte() ;
                recv_state = GET_CRC_LOW ;
                init_timer() ;
                break;

            default:  /* Normally GET_CRC_LOW */
                recv_crc = (recv_crc<<8) + get_byte() ;

                init_timer() ;
                while( check_time() < WAIT_TIME ) {} ;

                if( ( recv_crc == crc_sum )
                     && ( address == MASTER_ADDRESS ) )
                    return TRUE ;
                else {
                    init_timer() ;
		            while( check_time() < RESYNC_TIME ) {} ;
                    return FALSE ;
                }
            }
        }
    } while( check_time() < timeout ) ;
    } while( recv_byte() ) ;

    init_timer() ;
    while( check_time() < RESYNC_TIME ) {} ;
    return FALSE ;

} /* end of packet_transceive_single() */


/****************************************************************************
 * FUNCTION : init_crc
 *      This function inititializes the crc value for transmitting
 */

void init_crc( void )
{
    crc_sum = 0xffff ;

} /* end of init_crc() */


/****************************************************************************
 * FUNCTION : add_crc
 *      This function adds the newly received byte to the crc.
 *
 *       See "C Programmer's Guide to Serial Communications" (Sams),
 *       pp. 767-784 for information on the CRC calculation .
 */

void add_crc( UCHAR current_byte )
{
    int idx ;

    crc_sum = ( crc_sum ^ current_byte << 8 ) ;

    for( idx=0 ; idx<8 ; idx++ ) {
        if( crc_sum & 0x8000)
            crc_sum = ( crc_sum <<= 1 ) ^ 0x1021 ;
        else
            crc_sum <<= 1 ;
    }

} /* end of add_crc() */

