//============================================================================//
// File:         canopen_main.c                                               //
// Description:  Simple "CANopen Master" example                              //
//                                                                            //
// Copyright 2017 MicroControl GmbH & Co. KG                                  //
// 53844 Troisdorf - Germany                                                  //
// www.microcontrol.net                                                       //
//                                                                            //
//----------------------------------------------------------------------------//
// Redistribution and use in source and binary forms, with or without         //
// modification, are permitted provided that the following conditions         //
// are met:                                                                   //
// 1. Redistributions of source code must retain the above copyright          //
//    notice, this list of conditions, the following disclaimer and           //
//    the referenced file 'LICENSE'.                                          //
// 2. Redistributions in binary form must reproduce the above copyright       //
//    notice, this list of conditions and the following disclaimer in the     //
//    documentation and/or other materials provided with the distribution.    //
// 3. Neither the name of MicroControl nor the names of its contributors      //
//    may be used to endorse or promote products derived from this software   //
//    without specific prior written permission.                              //
//                                                                            //
// Licensed under the Apache License, Version 2.0 (the "License");            //
// you may not use this file except in compliance with the License.           //
// You may obtain a copy of the License at                                    //
//                                                                            //
//    http://www.apache.org/licenses/LICENSE-2.0                              //
//                                                                            //
// Unless required by applicable law or agreed to in writing, software        //
// distributed under the License is distributed on an "AS IS" BASIS,          //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   //
// See the License for the specific language governing permissions and        //
// limitations under the License.                                             //
//============================================================================//


/*----------------------------------------------------------------------------*\
** Include files                                                              **
**                                                                            **
\*----------------------------------------------------------------------------*/



#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "canopen_master.h"

/*----------------------------------------------------------------------------*\
** Module variables                                                           **
**                                                                            **
\*----------------------------------------------------------------------------*/

static uint32_t      ulCounterOneSecondS = 0;
static uint8_t       ubProgRunS;
static ComNode_ts    atsComNodeS[126];

//-------------------------------------------------------------------
// configuration objects
//
static CoObject_ts   atsCoObjectS[10];
static uint32_t      ulObjCountS;
static uint32_t      ulIdx1016_S;
static uint8_t       ubIdx5FF5_S;

/*----------------------------------------------------------------------------*\
** Structures / Enumerations                                                  **
**                                                                            **
\*----------------------------------------------------------------------------*/

enum ComDemoNodeCmd_e {
   eDEMO_NODE_CMD_WRITE_CONFIG = 0x01

};


//----------------------------------------------------------------------------//
// sig_handler_time()                                                         //
// timer                                                                      //
//----------------------------------------------------------------------------//
void sig_handler_time(int slSignalV)
{
   static uint32_t ulCounterTenMillisecondsS = 0;


   if(slSignalV == SIGALRM)
   {
      ulCounterTenMillisecondsS++;
      if(ulCounterTenMillisecondsS == 100)
      {
         ulCounterOneSecondS++;
         ulCounterTenMillisecondsS = 0;
      }
      ComMgrTimerEvent();
   }
}

//----------------------------------------------------------------------------//
// sig_handler_quit()                                                         //
// quit program                                                               //
//----------------------------------------------------------------------------//
void sig_handler_quit(int slSignalV)
{
   if(slSignalV == SIGINT)
   {
      ubProgRunS = 0;
   }
}

//----------------------------------------------------------------------------//
// init_signal_handler()                                                      //
// install signal handler for CTRL-C (SIGINT) and alarm (SIGALRM)             //
//----------------------------------------------------------------------------//
void init_signal_handler(void)
{
   struct sigaction   tsSigActionT;
   struct itimerval   tsTimerValT;

   tsSigActionT.sa_handler = sig_handler_quit;
   sigemptyset(&tsSigActionT.sa_mask);
   tsSigActionT.sa_flags = 0;
   sigaction(SIGINT, &tsSigActionT, 0);

   tsSigActionT.sa_handler = sig_handler_time;
   sigemptyset(&tsSigActionT.sa_mask);
   tsSigActionT.sa_flags = 0;
   sigaction(SIGALRM, &tsSigActionT, 0);

   //----------------------------------------------------------------
   // setup a 10 ms timer interval
   //
   tsTimerValT.it_value.tv_sec  = 1;
   tsTimerValT.it_value.tv_usec = 0;
   tsTimerValT.it_interval.tv_sec  = 0;
   tsTimerValT.it_interval.tv_usec = 10000;
   setitimer(ITIMER_REAL, &tsTimerValT, NULL);

}



//----------------------------------------------------------------------------//
// ComDemoAppInit()                                                           //
//                                                                            //
//----------------------------------------------------------------------------//
void ComDemoAppInit(uint8_t ubNetV)
{
   uint8_t  ubCntT;

   //----------------------------------------------------------------
   // add structures for node-ID 1 to node-ID 126
   //
   for(ubCntT = 1; ubCntT < 127; ubCntT++)
   {
      ComNodeSetDefault(&atsComNodeS[ubCntT - 1]);
      ComMgrNodeAdd(ubNetV, ubCntT, &atsComNodeS[ubCntT - 1]);
   }

   //----------------------------------------------------------------
   // Setup the internal object table, which has the following
   // entries:
   // 1016h:01h - ulIdx1016_S  - write operation
   //

   //----------------------------------------------------------------
   // module heartbeat consumer time is 2500 ms (0x09C4), test
   // for node-ID 127 (0x7F)
   //
   ulIdx1016_S = 0x007F09C4;

   //----------------------------------------------------------------
   // object 1016h:01h
   //
   atsCoObjectS[0].uwIndex    = 0x1016;
   atsCoObjectS[0].ubSubIndex = 0x01;
   atsCoObjectS[0].ubMarker   = eDEMO_NODE_CMD_WRITE_CONFIG;
   atsCoObjectS[0].ulDataSize = 4;
   atsCoObjectS[0].pvdData    = &(ulIdx1016_S);

   //----------------------------------------------------------------
   // port direction: 0 .. 3 outputs
   //
   ubIdx5FF5_S = 0x0F;

   //----------------------------------------------------------------
   // object 5FF5h:00h
   //
   atsCoObjectS[1].uwIndex    = 0x5FF5;
   atsCoObjectS[1].ubSubIndex = 0x00;
   atsCoObjectS[1].ubMarker   = eDEMO_NODE_CMD_WRITE_CONFIG;
   atsCoObjectS[1].ulDataSize = 1;
   atsCoObjectS[1].pvdData    = &(ubIdx5FF5_S);

}


//----------------------------------------------------------------------------//
// ComDemoWriteModuleConfiguration()                                          //
// print some information about the CANopen device                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComDemoWriteModuleConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   uint8_t        ubSdoT;     // SDO client

   printf("                ");
   printf("Write module configuration ...\n");


   //----------------------------------------------------------------
   // get and check SDO client index
   //
   ubSdoT = ComSdoGetClient(ubNetV);
   if(ubSdoT >= COM_SDO_CLIENT_MAX)
   {
      return(-eCOM_ERR_SDO_CLIENT_VALUE);
   }
   ComSdoSetTimeout(ubNetV, ubSdoT, 10000);

   //----------------------------------------------------------------
   // perform the SDO write operation
   //
   ulObjCountS = 2;

   ComSdoWriteObject(ubNetV, ubSdoT, ubNodeIdV,
                     &atsCoObjectS[0],
                     &ulObjCountS);

   return(eCOM_ERR_OK);
}

//----------------------------------------------------------------------------//
// ComDemoSetupPdoConfiguration()                                             //
//                                                                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComDemoSetupPdoConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   printf("                ");
   printf("Setup PDO configuration - TPDO %03Xh - RPDO %03Xh \n",
         0x0180 + ubNodeIdV, 0x0200 + ubNodeIdV);

   ComPdoConfig(ubNetV, ubNodeIdV, ePDO_DIR_RCV, 0x0180 + ubNodeIdV, 1, ePDO_TYPE_EVENT_PROFILE, 0);
   ComPdoEnable(ubNetV, ubNodeIdV, ePDO_DIR_RCV, 1);

   ComPdoConfig(ubNetV, ubNodeIdV, ePDO_DIR_TRM, 0x0200 + ubNodeIdV, 1, ePDO_TYPE_EVENT_PROFILE, 0);
   ComPdoEnable(ubNetV, ubNodeIdV, ePDO_DIR_TRM, 1);

   return(eCOM_ERR_OK);
}

//----------------------------------------------------------------------------//
// main()                                                                     //
//                                                                            //
//----------------------------------------------------------------------------//
int main(int argc, char *argv[])
{
   uint32_t ulTickOneSecondT = 0;
   ubProgRunS = 1;

   //----------------------------------------------------------------
   // Print application information
   //
   printf("\n\n");
   printf("###############################################################################\n");
   printf("# uMIC.200 CANopen Master Example                                             #\n");
   printf("###############################################################################\n");
   printf("%s\n", ComMgrGetVersionString(eCOM_VERSION_STACK));
   printf("Use CTRL-C to quit this demo.\n");
   printf("\n");


   //----------------------------------------------------------------
   // Initialise the signal handler for timer and CTRL-C
   //
   init_signal_handler();

   ComTmrSetPeriod(10000);

   //----------------------------------------------------------------
   // Initialise the CANopen master stack
   // The bitrate value is a dummy here, since the bitrate is
   // set via the socketcan configuration file.
   //
   ComMgrInit( eCP_CHANNEL_1, eCOM_NET_1, eCP_BITRATE_500K,
               127, eCOM_MODE_NMT_MASTER);


   //----------------------------------------------------------------
   // Initialise application variables
   //
   ComDemoAppInit(eCOM_NET_1);


   //----------------------------------------------------------------
   // start the CANopen master stack
   //
   ComMgrStart(eCOM_NET_1);


   //----------------------------------------------------------------
   // start master detection procedure
   //
   ComNmtMasterDetection(eCOM_NET_1, 1);

   //----------------------------------------------------------------
   // CANopen master is heartbeat producer, 2s
   //
   ComNmtSetHbProdTime(eCOM_NET_1, 2000);


   //----------------------------------------------------------------
   // this is the main loop of the application
   //
   while(ubProgRunS)
   {
      while(ulTickOneSecondT == ulCounterOneSecondS)
      {
         sleep(10);
      }
      ulTickOneSecondT = ulCounterOneSecondS;
   }

   //----------------------------------------------------------------
   // reset all nodes when master quits
   //
   ComNmtSetNodeState(eCOM_NET_1, 0, eCOM_NMT_STATE_RESET_COM);

   printf("\n");
   printf("Quit CANopen Master demo.\n");
   return(0);
}




