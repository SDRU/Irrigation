/****************************************************************************
 *   FILE -      protocol.h
 *
 *   DESCRIPTION -
 *
 *       These are the prototypes of functions used when interfacing to the
 *       Tescom protocol layers.
 *
 ****/

/******** TYPEDEFS ********/

typedef unsigned char   UCHAR;        /* 8 bits */
//typedef unsigned int    UINT;   /* Actually 32 bits */
typedef unsigned short int    UINT16;   /* 16 bits */

/******** DEFINES ********/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define LO_BYTE(x) ((UCHAR)( (x) & 0xff ) )
#define HI_BYTE(x) ( (UCHAR) ( (x) >> 8 ) )

/******** STRUCTURES ********/

struct segment_struct {  /* Used for reading/writing profile segments */
    char  type ;
    UINT16  variable1 ;
    UINT16  variable2 ;
};

/******** GLOBALS *********/
extern UINT16 Retries;
extern UINT16 ComPort;

/******** PROTOTYPES ********/

int  _stdcall Startup(UINT16, UINT16);
int  _stdcall Shutdown(void);
UINT16 _stdcall ReadNetVar(UINT16, UINT16, UINT16 *);
UINT16 _stdcall WriteNetVar(UINT16, UINT16, UINT16);
UINT16 _stdcall WriteProfileSegment(UINT16, struct segment_struct *, UINT16);
UINT16 _stdcall ReadProfileSegment(UINT16, struct segment_struct *, UINT16);

