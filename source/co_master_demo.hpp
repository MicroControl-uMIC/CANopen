//====================================================================================================================//
// File:          co_master_demo.hpp                                                                                  //
// Description:   CANopen master demo class                                                                           //
//                                                                                                                    //
// Copyright (C) MicroControl GmbH & Co. KG                                                                           //
// 53844 Troisdorf - Germany                                                                                          //
// www.microcontrol.net                                                                                               //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the   //
// following conditions are met:                                                                                      //
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions, the following   //
//    disclaimer and the referenced file 'LICENSE'.                                                                   //
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       //
//    following disclaimer in the documentation and/or other materials provided with the distribution.                //
// 3. Neither the name of MicroControl nor the names of its contributors may be used to endorse or promote products   //
//    derived from this software without specific prior written permission.                                           //
//                                                                                                                    //
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance     //
// with the License.                                                                                                  //
// You may obtain a copy of the License at                                                                            //
//                                                                                                                    //
//    http://www.apache.org/licenses/LICENSE-2.0                                                                      //
//                                                                                                                    //
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   //
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for  //
// the specific language governing permissions and limitations under the License.                                     //                                                                                  //
//                                                                                                                    //
//====================================================================================================================//


//------------------------------------------------------------------------------------------------------
/*!
** \file    co_master_demo.hpp
** \brief   CANopen master demo
**
*/
#ifndef CO_MASTER_DEMO_HPP_
#define CO_MASTER_DEMO_HPP_


/*--------------------------------------------------------------------------------------------------------------------*\
** Include files                                                                                                      **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include "canopen_master.h"

//-----------------------------------------------------------------------------------------------------------
/*!
** \class   CoMasterDemo
** \brief   CANopen Master Demo
**
*/
class CoMasterDemo : public QObject {

   Q_OBJECT

public:
   //--------------------------------------------------------------------------------------------------------
   CoMasterDemo();

   ~CoMasterDemo();

   void           start();

   void           stop();

private slots:

   void           onEmcyConsEventReceive(uint8_t ubNetV, uint8_t ubNodeIdV);

   void           onLssEventReceive(uint8_t ubNetV, uint8_t ubLssProtocolV);

   void           onMgrEventBus(uint8_t ubNetV, CpState_ts * ptsBusStateV);

   void           onNmtEventActiveMaster( uint8_t ubNetV, uint8_t ubPriorityV, uint8_t ubNodeIdV);

   void           onNmtEventHeartbeat(uint8_t ubNetV, uint8_t ubNodeIdV);

   void           onNmtEventIdCollision(uint8_t ubNetV);

   void           onNmtEventMasterDetection(uint8_t ubNetV, uint8_t ubResultV);

   //---------------------------------------------------------------------------------------------------
   /*!
   ** \param[in]  ubNetV      - CANopen Network channel
   ** \param[in]  ubNodeIdV   - Node-ID value
   ** \param[in]  ubNmtStateV - New NMT state
   **
   ** The slot handles the signal QCoEvent::comNmtEventStateChange(), issued by the
   ** CANopen FD Master callback ComNmtEventStateChange().
   */
   void           onNmtEventStateChange( uint8_t ubNetV, uint8_t ubNodeIdV, uint8_t ubNmtEventV);

   void           onPdoEventReceive(uint8_t ubNetV, uint16_t uwPdoV);

   void           onPdoEventTimeout(uint8_t ubNetV, uint16_t uwPdoNumV);

   void           onSdoEventObjectReady( uint8_t ubNetV, uint8_t ubNodeIdV, CoObject_ts * ptsCoObjV, uint32_t * pulAbortV);

   void           onSdoEventProgress(uint8_t ubNetV, uint8_t ubNodeIdV, uint16_t uwIndexV, uint8_t ubSubIndexV,
                          uint32_t ulByteCntV);

   void           onSdoEventTimeout(uint8_t ubNetV, uint8_t ubNodeIdV, uint16_t uwIndexV, uint8_t ubSubIndexV);

   void           onTimerEvent(void);

   void           runCmdParser(void);
   
private:

   void           connectComEvents(void);

   uint8_t        ubCanChannelP;
   uint8_t        ubNetworkP;
   uint8_t        ubMasterNodeIdP;

   QTimer         clTimerP;         // cyclic event timer
      
};


#endif /*CO_MASTER_DEMO_HPP_*/