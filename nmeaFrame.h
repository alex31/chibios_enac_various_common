#ifndef __NMEA_FRAME__
#define __NMEA_FRAME__

#include "ch.h"
#include "hal.h"



/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/


#define MAX_NUM_OF_FIELD 22
#define LINK_TIMOUT	 100 // in milliseconds

struct _NmeaBinder;
typedef struct _NmeaBinder NmeaBinder;

typedef enum {
  NS_WAIT_BEGIN, NS_WAIT_CHKSUM, NS_WAIT_CTRL1,  NS_WAIT_CTRL2
}  NmeaState;

typedef struct  {
  char      buffer[SERIAL_BUFFERS_SIZE];
  char      receivedChksm1;
  char      receivedChksm2;
  uint8_t   currentChksm;
  uint8_t   writeIndex;
  NmeaState   state;
}  NmeaStateMachine ;



/*
#                 _ __            _       _    _                 
#                | '_ \          | |     | |  (_)                
#                | |_) |  _   _  | |__   | |   _     ___         
#                | .__/  | | | | | '_ \  | |  | |   / __|        
#                | |     | |_| | | |_) | | |  | |  | (__         
#                |_|      \__,_| |_.__/  |_|  |_|   \___|        
*/


// error type is one of error callback parameter
typedef enum {
  NMEA_CHKSUM_ERR, NMEA_TIMOUT_ERR, NMEA_OVERLENGTH_ERR,  NMEA_OVERFIELD_ERR
}  NmeaError;

// static typing for binding of the message,
typedef enum {
  NMEA_INT, NMEA_LONG, NMEA_FLOAT, NMEA_DOUBLE, NMEA_CHAR, NMEA_STRING
}  NmeaType;


// type of the error callback function
typedef void (*ErrorCallback) (const NmeaError error, const void * const userData,
			       const char * const msg) ;

// static definition of field for binding
typedef struct  {
  const char *    fieldName; // name of field, for debug and readability
  const NmeaType    fieldType; // type of the field, value will be converted according to this type
  const uint8_t     fieldIndex; // index of the field in the NMEA message
} NmeaFieldDesc;

// Nmea field after parsing furnished to callback 
// callback can dynamically verify that there is not type mismatching beetween 
// binding declaration, and callback management.
typedef struct  {
  const NmeaFieldDesc *fieldDesc;	// field description
  const char        *f_raw;		// NMEA field, string raw value
  union {				// union with converted value according 
					// to given type fieldType
    char	   f_c;				// when char
    const char * f_s;				// when string
    int32_t	   f_i;				// when int32_t
    long	   f_l;				// when long
    float  	   f_f;				// when float (32 bits)
    double 	   f_d;				// when double (64 bits)
  };
} NmeaParam;

// type of the callback function
typedef void (*MsgCallback) (const void * const userData, 
			     const uint32_t argc, const NmeaParam const * argv) ;

// binding data
struct  _NmeaBinder {
  const char* const   fieldClass;		// NMEA message, begining by '$'
  const MsgCallback   msgCb;			// Callback function for these class of NMEA message
  const NmeaFieldDesc field [MAX_NUM_OF_FIELD]; // description for all the binded fields
};



void feedNmea (const NmeaBinder *nbs, NmeaStateMachine *sm, 
	       const void * const  userData, char c, ErrorCallback error_cb);
void initStateMachine (NmeaStateMachine *sm);



/*
#                                                   _ __    _                 
#                                                  | '_ \  | |                
#                  ___  __  __    ___   _ __ ___   | |_) | | |    ___         
#                 / _ \ \ \/ /   / _ \ | '_ ` _ \  | .__/  | |   / _ \        
#                |  __/  >  <   |  __/ | | | | | | | |     | |  |  __/        
#                 \___| /_/\_\   \___| |_| |_| |_| |_|     |_|   \___|        
#                          __         
#                         / _|        
#                  ___   | |_         
#                 / _ \  |  _|        
#                | (_) | | |          
#                 \___/  |_|          
#                                             
#                                             
#                 _   _   ___     ___         
#                | | | | / __|   / _ \        
#                | |_| | \__ \  |  __/        
#                 \__,_| |___/   \___|        
*/

/*

// for testing purpose and example
void msgA_cb (const unsigned int argc, const NmeaParam const * argv);
void msgB_cb (const unsigned int argc, const NmeaParam const * argv);
void error_cb (const NmeaError error, const char * const fieldClass);


static const NmeaBinder nbs[] = {
  // all $MSGA message will be managed by msgA_cb callback function
  // the callback will receive the three fields 1,3,5 in original message 
  {.fieldClass = "$MSGA", .msgCb = &msgA_cb,
   .field = {
      {.fieldName = "vitesse",     .fieldType = NMEA_INT,    .fieldIndex = 1},
      {.fieldName = "champ texte", .fieldType = NMEA_STRING, .fieldIndex = 3},
      {.fieldName = "intensite",   .fieldType = NMEA_DOUBLE, .fieldIndex = 5},
      // *MANDATORY* marker of end of list
      {.fieldIndex = 0}
    }
  },
  // all $GPGGA message will be managed by msgB_cb callback function
  // the callback will receive the  fields  in original message 
  {.fieldClass = "$GPGGA", .msgCb = &msgB_cb,
   .field = {
      {.fieldName = "utc time",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 1},
      {.fieldName = "latitude",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 2},
      {.fieldName = "nord/sud",     .fieldType = NMEA_CHAR,    .fieldIndex = 3},
      {.fieldName = "longitude",    .fieldType = NMEA_DOUBLE,  .fieldIndex = 4},
      {.fieldName = "est/ouest",    .fieldType = NMEA_CHAR,    .fieldIndex = 5},
      {.fieldName = "nb sat",       .fieldType = NMEA_INT,     .fieldIndex = 7},
      {.fieldName = "precision",    .fieldType = NMEA_FLOAT,   .fieldIndex = 8},
      {.fieldName = "Altitude",     .fieldType = NMEA_FLOAT,   .fieldIndex = 9},
      // *MANDATORY* marker of end of list
       {.fieldIndex = 0}
    }
  },      
  // *MANDATORY* marker of end of list
   {.fieldClass = NULL}
};

// for example
void msgA_cb (const unsigned int argc, const NmeaParam const * argv)
{
  // should verify that type of arg is what you use
  // tu be sure not to use bad field in the union
  assert (argv[0].fieldDesc->fieldType == NMEA_INT);
  assert (argv[1].fieldDesc->fieldType == NMEA_STRING);
  assert (argv[2].fieldDesc->fieldType == NMEA_DOUBLE);

  // use converted value according to type declared in binding
  printf ("cbA binded args[%d] = %d %s %f\n", argc, argv[0].f_i, argv[1].f_s, argv[2].f_d);
}

void msgB_cb (const unsigned int argc, const NmeaParam const * argv)
{
  // should verify that type of arg is what you use
  // tu be sure not to use bad field in the union
  assert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE);
  assert (argv[1].fieldDesc->fieldType == NMEA_DOUBLE);
  assert (argv[2].fieldDesc->fieldType == NMEA_CHAR  );
  assert (argv[3].fieldDesc->fieldType == NMEA_DOUBLE);
  assert (argv[4].fieldDesc->fieldType == NMEA_CHAR  );
  assert (argv[5].fieldDesc->fieldType == NMEA_INT   );
  assert (argv[6].fieldDesc->fieldType == NMEA_FLOAT );
  assert (argv[7].fieldDesc->fieldType == NMEA_FLOAT );

  // use converted value according to type declared in binding
  printf ("cbB binded args[%d] : %s=%f %s=%f %s=%c\n%s=%f %s=%c %s=%d\n%s=%f %s=%f\n",
	  argc,
	  argv[0].fieldDesc->fieldName, argv[0].f_d,
	  argv[1].fieldDesc->fieldName, argv[1].f_d,
	  argv[2].fieldDesc->fieldName, argv[2].f_c,
	  argv[3].fieldDesc->fieldName, argv[3].f_d,
	  argv[4].fieldDesc->fieldName, argv[4].f_c,
	  argv[5].fieldDesc->fieldName, argv[5].f_i,
	  argv[6].fieldDesc->fieldName, argv[6].f_f,
	  argv[7].fieldDesc->fieldName, argv[7].f_f);
}

void error_cb (const NmeaError error, const char * const msg) 
{
  printf ("error of type %s occurs on message %s\n",
	  error ==  NMEA_CHKSUM_ERR ? "Checksum" : "Timeout",
	  msg);
}



main (...)
{

  NmeaStateMachine sm;
  initStateMachine (&sm);
 
  feedNmea (nbs, &sm, getCharFromSerial() , error_cb);

}

*/

#endif // __NMEA_FRAME

