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


#include "canopen_master.h"

#include <stdio.h>


/*----------------------------------------------------------------------------*\
** Variables                                                                  **
**                                                                            **
\*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*\
** Functions                                                                  **
**                                                                            **
\*----------------------------------------------------------------------------*/

extern void         ComDemoAddDevice(uint8_t ubNetV, uint8_t ubNodeIdV);
extern void         ComDemoShowDeviceInfo(uint8_t ubNetV, uint8_t ubNodeIdV);
extern ComStatus_tv ComDemoSetupEmcyConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV);
extern ComStatus_tv ComDemoSetupPdoConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV);
extern ComStatus_tv ComDemoWriteModuleConfiguration(uint8_t ubNetV, uint8_t ubNodeIdV);

//----------------------------------------------------------------------------//
// ComEmcyConsEventReceive()                                                  //
//                                                                            //
//----------------------------------------------------------------------------//
void ComEmcyConsEventReceive(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   uint8_t  aubDataT[8];
   uint16_t uwEmcyCodeT;

   ComEmcyConsGetData(ubNetV,ubNodeIdV,&aubDataT[0]);
   uwEmcyCodeT = aubDataT[1];
   uwEmcyCodeT = uwEmcyCodeT << 8;
   uwEmcyCodeT = uwEmcyCodeT | aubDataT[0];

   printf("can%d: NID %03d - EMCY code %04X, error register value %d\n",
           ubNetV, ubNodeIdV, uwEmcyCodeT, aubDataT[2]);
}


//----------------------------------------------------------------------------//
// ComLssEventReceive()                                                       //
// Function handler for LSS reception                                         //
//----------------------------------------------------------------------------//
void ComLssEventReceive(uint8_t __attribute__((unused)) ubNetV,
                        uint8_t __attribute__((unused)) ubLssProtocolV)
{

}



//----------------------------------------------------------------------------//
// ComMgrEventBus()                                                           //
// Handler for Bus events                                                     //
//----------------------------------------------------------------------------//
void ComMgrEventBus(uint8_t ubNetV, CpState_ts * ptsBusStateV)
{
   //----------------------------------------------------------------
   // Initialise user parameter
   //
   switch (ubNetV)
   {
      case eCOM_NET_1 :
         if(ptsBusStateV->ubCanErrState == eCP_STATE_BUS_OFF)
         {
            //---------------------------------------------
            // handle bus-off condition
            //
         }
         break;


      default:

         break;
   }

}


//----------------------------------------------------------------------------//
// ComUserInit()                                                              //
//                                                                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComMgrMasterInit(uint8_t __attribute__((unused)) ubNetV)
{

   return(eCOM_ERR_OK);
}

ComStatus_tv   ComMgrUserInit(uint8_t __attribute__((unused)) ubNetV)
{
    return(eCOM_ERR_OK);
}

//----------------------------------------------------------------------------//
// ComNmtEventActiveMaster()                                                  //
// Handler for Active Master detection                                        //
//----------------------------------------------------------------------------//
void ComNmtEventActiveMaster( uint8_t __attribute__((unused)) ubNetV,
                              uint8_t __attribute__((unused)) ubPriorityV,
                              uint8_t __attribute__((unused)) ubNodeIdV)
{

}


//----------------------------------------------------------------------------//
// ComNmtEventHeartbeat()                                                     //
// Handler for NMT events                                                     //
//----------------------------------------------------------------------------//
void ComNmtEventHeartbeat(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   printf("can%d: NID %03d - No heartbeat received\n", ubNetV, ubNodeIdV);
}


//----------------------------------------------------------------------------//
// ComNmtEventIdCollision()                                                   //
// Handler for NMT events                                                     //
//----------------------------------------------------------------------------//
void ComNmtEventIdCollision(uint8_t ubNetV)
{
   printf("can%d: COB-ID collision.\n", ubNetV);
}


//----------------------------------------------------------------------------//
// ComNmtEventMasterDetection()                                               //
// Handler for NMT master detection events                                    //
//----------------------------------------------------------------------------//
void ComNmtEventMasterDetection(uint8_t ubNetV, uint8_t ubResultV)
{
   //----------------------------------------------------------------
   // In case of timeout: we are the active CANopen master
   //
   if(ubResultV == eCOM_NMT_DETECT_TIMEOUT)
   {
      printf("I am the active Master.\n");

      //----------------------------------------------------------------
      // reset all nodes
      //
      ComNmtSetNodeState(ubNetV, 0, eCOM_NMT_STATE_RESET_COM);

      //----------------------------------------------------------------
      // set the cycle time to 500 ms
      //
      ComSyncSetCycleTime(ubNetV, 500000);
      ComSyncEnable(ubNetV, 1);
   }
}


//----------------------------------------------------------------------------//
// ComNmtEventResetCommunication()                                            //
// Handler for NMT events                                                     //
//----------------------------------------------------------------------------//
void ComNmtEventResetCommunication(uint8_t __attribute__((unused)) ubNetV)
{

}


//----------------------------------------------------------------------------//
// ComNmtEventResetNode()                                                     //
// Handler for NMT events                                                     //
//----------------------------------------------------------------------------//
void ComNmtEventResetNode(uint8_t __attribute__((unused)) ubNetV)
{

}


//----------------------------------------------------------------------------//
// ComNmtEventStateChange()                                                   //
// Handler for NMT events                                                     //
//----------------------------------------------------------------------------//
void ComNmtEventStateChange(uint8_t ubNetV, uint8_t ubNodeIdV,
                            uint8_t ubNmtEventV)
{
   switch(ubNmtEventV)
   {
      case eCOM_NMT_STATE_BOOTUP:
        printf("can%d: NID %03d - received boot-up message\n",
              ubNetV, ubNodeIdV);

        ComDemoAddDevice(ubNetV, ubNodeIdV);

        break;

      case eCOM_NMT_STATE_PREOPERATIONAL:
        printf("can%d: NID %03d - switched to pre-operational state\n",
              ubNetV, ubNodeIdV);
        break;

      case eCOM_NMT_STATE_OPERATIONAL:
        printf("can%d: NID %03d - switched to operational state\n",
              ubNetV, ubNodeIdV);
        break;

      default:

        break;
   }
}


//----------------------------------------------------------------------------//
// ComPdoEventReceive()                                                       //
// Function handler for PDO Receive                                           //
//----------------------------------------------------------------------------//
void ComPdoEventReceive(uint8_t ubNetV, uint16_t uwPdoNumV)
{
   uint8_t  aubPdoDataT[8];

   ComPdoGetData(ubNetV, uwPdoNumV, ePDO_DIR_RCV, &aubPdoDataT[0]);
}


//----------------------------------------------------------------------------//
// ComPdoEventTimeout()                                                       //
// ComPdoEventTimeout                                                         //
//----------------------------------------------------------------------------//
void ComPdoEventTimeout(uint8_t  __attribute__((unused)) ubNetV,
                        uint16_t __attribute__((unused)) uwPdoV)
{

}


//----------------------------------------------------------------------------//
// ComSdoSrvBlkUpObjectSize()                                                 //
// Function handler for SDO server block transfer                             //
//----------------------------------------------------------------------------//
uint32_t ComSdoSrvBlkUpObjectSize(uint8_t  __attribute__((unused)) ubNetV,
                                  uint16_t __attribute__((unused)) uwIndexV,
                                  uint8_t  __attribute__((unused)) ubSubIndexV)
{
   uint32_t ulObjSizeT = 0;


   return ulObjSizeT;
}


//----------------------------------------------------------------------------//
// ComSdoEventProgress()                                                      //
// Function handler for SDO progress                                          //
//----------------------------------------------------------------------------//
void  ComSdoEventProgress(uint8_t  __attribute__((unused)) ubNetV,
                          uint8_t  __attribute__((unused)) ubNodeIdV,
                          uint16_t __attribute__((unused)) uwIndexV,
                          uint8_t  __attribute__((unused)) ubSubIndexV,
                          uint32_t __attribute__((unused)) ulByteCntV)
{

}


//----------------------------------------------------------------------------//
// ComSdoEventTimeout()                                                       //
// Function handler for SDO timeout                                           //
//----------------------------------------------------------------------------//
void  ComSdoEventTimeout(uint8_t __attribute__((unused)) ubNetV,
                         uint8_t ubNodeIdV,
                         uint16_t uwIndexV, uint8_t ubSubIndexV)
{
   printf("SDO timeout: NID %d - object %04Xh:%02Xh\n", ubNodeIdV, uwIndexV, ubSubIndexV);
}


//----------------------------------------------------------------------------//
// ComSdoEventObjectReady()                                                   //
// Function handler for SDO transfer success                                  //
//----------------------------------------------------------------------------//
void ComSdoEventObjectReady(uint8_t ubNetV, uint8_t ubNodeIdV,
                            CoObject_ts * ptsCoObjV,
                            uint32_t  __attribute__((unused)) * pulAbortV)
{
   uint16_t uwHeartbeatTimeT = 50;    // heartbeat time in ms

   switch (ptsCoObjV->ubMarker)
   {
      case eCOM_SDO_MARKER_NODE_GET_INFO:
         ComDemoShowDeviceInfo(ubNetV, ubNodeIdV);
         ComNodeSetHbProdTime(ubNetV,  ubNodeIdV, uwHeartbeatTimeT);
         break;

      case eCOM_SDO_MARKER_NODE_SET_HEARTBEAT:
         ComNmtSetHbConsTime(ubNetV, ubNodeIdV, uwHeartbeatTimeT * 4);
         ComDemoSetupEmcyConfiguration(ubNetV, ubNodeIdV);

         //----------------------------------------------------------------
         // configure default PDOs
         //
         ComDemoSetupPdoConfiguration(ubNetV, ubNodeIdV);

         //----------------------------------------------------------------
         // set node to operational
         //
         ComNmtSetNodeState(ubNetV, ubNodeIdV, eCOM_NMT_STATE_OPERATIONAL);
         break;

      default:

         break;
   }

}


