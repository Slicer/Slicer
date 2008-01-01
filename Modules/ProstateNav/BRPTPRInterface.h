// BRPTPRInterface.h version 1.0: July 17, 2007 2:42pm

#ifndef _BRPTPRInterface_h_
#define _BRPTPRInterface_h_

 
// Workflow values: "workphase" attribute
#define BRPTPR_START_UP     "START_UP"
#define BRPTPR_PLANNING     "PLANNING"
#define BRPTPR_CALIBRATION  "CALIBRATION"
#define BRPTPR_TARGETING    "TARGETING"
#define BRPTPR_MANUAL       "MANUAL"
#define BRPTPR_EMERGENCY    "EMERGENCY"
 
// Command values: "command" attribute
#define BRPTPR_ZFRAME       "ZFRAME"
#define BRPTPR_TARGET       "TARGET"
#define BRPTPR_POSITION     "POSITION"
 
// Status values: "status" attribute
#define BRPTPR_Idle         "Idle" //before all
#define BRPTPR_Initializing "Initializing" //after Start Up
#define BRPTPR_Uncalibrated "Uncalibrated"// after init
#define BRPTPR_Ready        "Ready" //Calibration after receiving Z fRAME 
//                                    and after oving to position
#define BRPTPR_Moving       "Moving" //after Sending Traget
#define BRPTPR_Manual       "Manual" //after going to Manual and  breaking the axes
#define BRPTPR_Error        "Error"
#define BRPTPR_EStop        "EStop"


#endif // _BRPTPRInterface_h_
