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

#define  COM_NET		eCOM_NET_1
#define  COM_CHANNEL    eCP_CHANNEL_1

static uint32_t      ulCounterOneSecondS = 0;
static uint8_t       ubProgRunS;
static ComNode_ts    atsComNodeS[126];
static uint8_t       aubDeviceScanS[126];
static uint8_t 		 aubPdoDoNodeId[2];
static uint8_t       aubDataS[8];
static uint8_t 		 ubPdoRcvCntS;


//-------------------------------------------------------------------
// configuration objects
//
static CoObject_ts   atsCoObjectS[10];
static uint32_t      ulObjCountS;
static uint8_t       ubIdx5FF5_S;

/*----------------------------------------------------------------------------*\
** Structures / Enumerations                                                  **
**                                                                            **
\*----------------------------------------------------------------------------*/

enum ComDemoNodeCmd_e {
   eDEMO_NODE_CMD_READ_CONFIG  = 0x01,
   eDEMO_NODE_CMD_WRITE_CONFIG
};

enum ComDemoNodeScan_e {
   eDEMO_NODE_SCAN_EMPTY = 0x00,
   eDEMO_NODE_SCAN_QUEUE,
   eDEMO_NODE_SCAN_RUN,
   eDEMO_NODE_SCAN_DONE
};


void         ComDemoAppProcess(void);
ComStatus_tv ComDemoReadModuleConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV);
void         ComDemoShowDeviceInfo(uint8_t ubNetV, uint8_t ubNodeIdV);
void         setRunningLightData(uint16_t uwPdoNumV);

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
      ComDemoAppProcess();

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


void ComDemoAddDevice(uint8_t __attribute__((unused)) ubNetV, uint8_t ubNodeIdV)
{
   aubDeviceScanS[ubNodeIdV - 1] = eDEMO_NODE_SCAN_QUEUE;

}

void ComDemoRemoveDevice(uint8_t __attribute__((unused)) ubNetV, uint8_t ubNodeIdV)
{
   aubDeviceScanS[ubNodeIdV - 1] = eDEMO_NODE_SCAN_EMPTY;
   ComNodeSetDefault(&atsComNodeS[ubNodeIdV - 1]);
}

//----------------------------------------------------------------------------//
// ComDemoAppInit()                                                           //
//                                                                            //
//----------------------------------------------------------------------------//
void ComDemoAppInit(uint8_t ubNetV)
{
    uint8_t ubCntT;
    ubPdoRcvCntS = 0;



    for(ubCntT = 1; ubCntT < 8; ubCntT++)
    {
    	aubDataS[ubCntT] = 0;
    }

    for(ubCntT = 1; ubCntT < 127; ubCntT++)
    {
    	ComNodeSetDefault(&atsComNodeS[ubCntT - 1]);
        ComMgrNodeAdd(ubNetV, ubCntT, &atsComNodeS[ubCntT - 1]);
        aubDeviceScanS[ubCntT - 1] = eDEMO_NODE_SCAN_EMPTY;
    }

    aubPdoDoNodeId[0] = 0;
	aubPdoDoNodeId[1] = 0;
   //----------------------------------------------------------------
   // port direction: 0 .. 7 outputs
   //
   ubIdx5FF5_S = 0xFF;

   //----------------------------------------------------------------
   // object 5FF5h:00h
   //
   atsCoObjectS[0].uwIndex    = 0x5FF5;
   atsCoObjectS[0].ubSubIndex = 0x00;
   atsCoObjectS[0].ubMarker   = eDEMO_NODE_CMD_WRITE_CONFIG;
   atsCoObjectS[0].ulDataSize = 1;
   atsCoObjectS[0].pvdData    = &(ubIdx5FF5_S);

}


void ComDemoAppProcess(void)
{
   uint8_t ubScanInProgressT = 0;

   //----------------------------------------------------------------
   // add structures for node-ID 1 to node-ID 126
   //
   for(uint8_t ubCntT = 0; ubCntT < 126; ubCntT++)
   {
      if (aubDeviceScanS[ubCntT] == eDEMO_NODE_SCAN_RUN)
      {
         ubScanInProgressT = 1;
         break;
      }
   }

   //----------------------------------------------------------------
   // add structures for node-ID 1 to node-ID 126
   //
   if (ubScanInProgressT == 0)
   {
      for(uint8_t ubCntT = 0; ubCntT < 126; ubCntT++)
      {
         if (aubDeviceScanS[ubCntT] == eDEMO_NODE_SCAN_QUEUE)
         {
            ComDemoReadModuleConfiguration(eCOM_NET_1, ubCntT +1);
            aubDeviceScanS[ubCntT] = eDEMO_NODE_SCAN_RUN;
            ubScanInProgressT = 1;
            break;
         }
      }
   }

   if (ubScanInProgressT == 0)
   {
	   //Send PDO's 50 * 10ms = 500ms
       if(ubPdoRcvCntS == 50)
       {
		   if(aubPdoDoNodeId[0] > 0)
		   {
			   setRunningLightData((uint16_t)aubPdoDoNodeId[1]);
			   ComPdoSendAsync(COM_NET,(uint16_t) aubPdoDoNodeId[0]);
		   }
		   if(aubPdoDoNodeId[1] > 0)
		   {
			   setRunningLightData((uint16_t)aubPdoDoNodeId[1]);
			   ComPdoSendAsync(COM_NET,(uint16_t) aubPdoDoNodeId[1]);
		   }
		   ubPdoRcvCntS = 0;
       }
       else
       {
    	   ubPdoRcvCntS++;
       }
   }
}

void setRunningLightData(uint16_t uwPdoNumV)
{
      if(aubDeviceScanS[uwPdoNumV-1] == eDEMO_NODE_SCAN_DONE)
      {
		  //----------------------------------------------------------------
		  // write Data to PDO
		  //
		  if ((aubDataS[0] == 0) || (aubDataS[0] == 128))
		  {
			  aubDataS[0] = 1;
		  }
		  else
		  {
			  aubDataS[0] = aubDataS[0] << 1;
		  }

        ComPdoSetData(COM_NET, uwPdoNumV, ePDO_DIR_TRM, &aubDataS[0]);
      }
}

//----------------------------------------------------------------------------//
// ComDemoShowDeviceInfo()                                                    //
// print some information about the CANopen device                            //
//----------------------------------------------------------------------------//
void ComDemoShowDeviceInfo(uint8_t __attribute__((unused)) ubNetV, uint8_t ubNodeIdV)
{
   printf("                ");
   uint32_t ulProfileT = atsComNodeS[ubNodeIdV - 1].ulIdx1000_DT;
   ulProfileT = ulProfileT & 0x0000FFFF;  // mask the profile
   printf("Device profile  : %03d\n", ulProfileT);

   printf("                ");
   printf("Error code      : %02d\n", atsComNodeS[ubNodeIdV - 1].ubIdx1001_ER);

   printf("                ");
   printf("Vendor ID       : %d\n"  , atsComNodeS[ubNodeIdV - 1].ulIdx1018_VI);

   printf("                ");
   printf("Product code    : %d\n"  , atsComNodeS[ubNodeIdV - 1].ulIdx1018_PC);

   printf("                ");
   printf("Revision number : %d\n"  , atsComNodeS[ubNodeIdV - 1].ulIdx1018_RN);

   printf("                ");
   printf("Serial number   : %d\n"  , atsComNodeS[ubNodeIdV - 1].ulIdx1018_SN);

   printf("                ");
   printf("Device name     : %s\n"  , atsComNodeS[ubNodeIdV - 1].aubIdx1008_DN);

}

//----------------------------------------------------------------------------//
// ComDemoReadModuleConfiguration()                                           //
//                                                                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComDemoReadModuleConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   uint8_t        ubSdoT;     // SDO client

   printf("                ");
   printf("Read module configuration ...\n");

   //----------------------------------------------------------------
   // get and check SDO client index
   //
   ubSdoT = ComSdoGetClient(ubNetV);
   if(ubSdoT >= COM_SDO_CLIENT_MAX)
   {
      return(-eCOM_ERR_SDO_CLIENT_VALUE);
   }
   ComSdoSetTimeout(ubNetV, ubSdoT, 200);

   ComNodeGetInfo(ubNetV, ubNodeIdV);

   return(eCOM_ERR_OK);
}


//----------------------------------------------------------------------------//
// ComDemoWriteModuleConfiguration()                                          //
// print some information about the CANopen device                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComDemoWriteModuleConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   if (atsComNodeS[ubNodeIdV - 1].ulIdx1018_PC == 1286014)
   {
      printf("                ");
      printf("Write module configuration ...\n");


      //----------------------------------------------------------------
      // perform the SDO write operation
      //
      ulObjCountS = 1;

      ComSdoWriteObject(ubNetV, 0, ubNodeIdV,
                        &atsCoObjectS[0],
                        &ulObjCountS);

      aubPdoDoNodeId[0] = ubNodeIdV;
   }

   else
   {
	   return(eCOM_ERR_PARM);
   }

   return(eCOM_ERR_OK);
}

//----------------------------------------------------------------------------//
// ComDemoSetupEmcyConfiguration()                                            //
//                                                                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComDemoSetupEmcyConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   printf("                ");
   printf("Setup EMCY consumer \n");

   ComEmcyConsSetId(ubNetV,  ubNodeIdV, 0x80 +  ubNodeIdV);
   ComEmcyConsEnable(ubNetV, ubNodeIdV, 1);
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

   if (atsComNodeS[ubNodeIdV - 1].ulIdx1018_PC == 9918200)
   {
      ComPdoConfig(ubNetV, ubNodeIdV, ePDO_DIR_TRM, 0x0200 + ubNodeIdV, 2, ePDO_TYPE_EVENT_PROFILE, 0);

      aubPdoDoNodeId[1] = ubNodeIdV;
   }
   else
   {
	   ComPdoConfig(ubNetV, ubNodeIdV, ePDO_DIR_TRM, 0x0200 + ubNodeIdV, 1, ePDO_TYPE_EVENT_PROFILE, 0);
   }
   ComPdoEnable(ubNetV, ubNodeIdV, ePDO_DIR_TRM, 1);

   aubDeviceScanS[ubNodeIdV - 1] = eDEMO_NODE_SCAN_DONE;
   return(eCOM_ERR_OK);
}

//----------------------------------------------------------------------------//
// main()                                                                     //
//                                                                            //
//----------------------------------------------------------------------------//
int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
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


   //----------------------------------------------------------------
   // The function ComMgrTimerEvent() is called every 10 ms inside
   // sig_handler_time(). We tell the CANopen master stack about
   // this update period here in order to work with correkt timer
   // ticks inside the stack. Please note that the parameter defines
   // the time in micro-seconds.
   //
   ComTmrSetPeriod(10000);

   //----------------------------------------------------------------
   // Initialise the CANopen master stack
   // The bitrate value is a dummy here, since the bitrate is
   // set via the CANpie server configuration file.
   //
   ComMgrInit( COM_CHANNEL, COM_NET, eCP_BITRATE_500K,
               127, eCOM_MODE_NMT_MASTER);


   //----------------------------------------------------------------
   // Initialise application variables
   //
   ComDemoAppInit(COM_NET);


   //----------------------------------------------------------------
   // start the CANopen master stack
   //
   ComMgrStart(COM_NET);


   //----------------------------------------------------------------
   // start master detection procedure
   //
   ComNmtMasterDetection(COM_NET, 1);

   //----------------------------------------------------------------
   // CANopen master is heartbeat producer, 500ms
   //
   ComNmtSetHbProdTime(COM_NET, 500);


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
   ComNmtSetNodeState(COM_NET, 0, eCOM_NMT_STATE_RESET_COM);

   ComMgrRelease(COM_NET);

   printf("\n");
   printf("Quit CANopen Master demo.\n");
   return(0);
}




