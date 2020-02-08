//====================================================================================================================//
// File:          co_master_demo.cpp                                                                                  //
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





/*--------------------------------------------------------------------------------------------------------------------*\
** Include files                                                                                                      **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>

#include "canopen_master.h"
#include "qco_event.hpp"
#include "co_master_demo.hpp"


/*--------------------------------------------------------------------------------------------------------------------*\
** Definitions                                                                                                        **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/

#define  TIMER_CYCLE_PERIOD         ((uint32_t)     10)        // timer period in milli-seconds



#ifndef  VERSION_MAJOR
#define  VERSION_MAJOR                       1
#endif

#ifndef  VERSION_MINOR
#define  VERSION_MINOR                       0
#endif

#ifndef  VERSION_BUILD
#define  VERSION_BUILD                       0
#endif

//----------------------------------------------------------------------------//
// ComMgrUserInit()                                                           //
//                                                                            //
//----------------------------------------------------------------------------//
ComStatus_tv ComMgrUserInit(uint8_t ubNetV)
{
   if (ubNetV == eCOM_NET_1)
   {

   }



   return (eCOM_ERR_OK);
}


//--------------------------------------------------------------------------------------------------------------------//
// main()                                                                                                             //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
int main(int argc, char *argv[])
{
   QCoreApplication clAppT(argc, argv);
   QCoreApplication::setApplicationName("can-dump");
   
   //---------------------------------------------------------------------------------------------------
   // get application version 
   //
   QString clVersionT;
   clVersionT += QString("%1.").arg(VERSION_MAJOR);
   clVersionT += QString("%1.").arg(VERSION_MINOR, 2, 10, QLatin1Char('0'));
   clVersionT += QString("%1,").arg(VERSION_BUILD, 2, 10, QLatin1Char('0'));
   clVersionT += " build on ";
   clVersionT += __DATE__;
   QCoreApplication::setApplicationVersion(clVersionT);


   //---------------------------------------------------------------------------------------------------
   // create the main class
   //
   CoMasterDemo clMainT;

   
   //---------------------------------------------------------------------------------------------------
   // connect the signals
   //
   /*
   QObject::connect(&clMainT, SIGNAL(finished()),
                    &clAppT,  SLOT(quit()));
   
   QObject::connect(&clAppT, SIGNAL(aboutToQuit()),
                    &clMainT, SLOT(aboutToQuitApp()));
*/
   
   //---------------------------------------------------------------------------------------------------
   // This code will start the messaging engine in QT and in 10 ms it will start the execution of the
   // clMainT.runCmdParser() routine.
   //
   QTimer::singleShot(10, &clMainT, SLOT(runCmdParser()));

   clAppT.exec();
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::CoMasterDemo()                                                                                       //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
CoMasterDemo::CoMasterDemo()
{
   ubCanChannelP   = eCP_CHANNEL_1;
   ubNetworkP      = eCOM_NET_1;
   ubMasterNodeIdP = 127;

   //---------------------------------------------------------------------------------------------------
   // connect events of CANopen master library to server
   //
   connectComEvents();

   //---------------------------------------------------------------------------------------------------
   // connect the cyclic timer to the event handler
   //
   connect(&clTimerP, SIGNAL(timeout()), this, SLOT(onTimerEvent()));

   //---------------------------------------------------------------------------------------------------
   // Print application information
   //
   printf("\n\n");
   printf("###############################################################################\n");
   printf("# uMIC.200 CANopen Master Example                                             #\n");
   printf("###############################################################################\n");
   printf("\n");
   printf("Using library: %s\n", ComMgrGetVersionString(eCOM_VERSION_STACK));
   printf("Use CTRL-C to quit this demo.\n");
   printf("\n");


}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::~CoMasterDemo()                                                                                      //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
CoMasterDemo::~CoMasterDemo()
{
   qDebug() << "CoMasterDemo::~CoMasterDemo()";


}



//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::connectComEvents()                                                                                   //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::connectComEvents(void)
{
   qDebug() << "CoMasterDemo::connectComEvents()      :";

   QCoEvent *  pclCoEventT = QCoEvent::instance();

   connect(pclCoEventT, &QCoEvent::comEmcyConsEventReceive,     this, &CoMasterDemo::onEmcyConsEventReceive);

   connect(pclCoEventT, &QCoEvent::comLssEventReceive,          this, &CoMasterDemo::onLssEventReceive);

   connect(pclCoEventT, &QCoEvent::comMgrEventBus,              this, &CoMasterDemo::onMgrEventBus);

   connect(pclCoEventT, &QCoEvent::comNmtEventHeartbeat,        this, &CoMasterDemo::onNmtEventHeartbeat);

   connect(pclCoEventT, &QCoEvent::comNmtEventMasterDetection,  this, &CoMasterDemo::onNmtEventMasterDetection);

   connect(pclCoEventT, &QCoEvent::comNmtEventStateChange,      this, &CoMasterDemo::onNmtEventStateChange);

   connect(pclCoEventT, &QCoEvent::comPdoEventReceive,          this, &CoMasterDemo::onPdoEventReceive);

   connect(pclCoEventT, &QCoEvent::comPdoEventTimeout,          this, &CoMasterDemo::onPdoEventTimeout);

   connect(pclCoEventT, &QCoEvent::comSdoEventObjectReady,      this, &CoMasterDemo::onSdoEventObjectReady);

   connect(pclCoEventT, &QCoEvent::comSdoEventTimeout,          this, &CoMasterDemo::onSdoEventTimeout);
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onEmcyConsEventReceive()                                                                             //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onEmcyConsEventReceive(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   uint8_t  aubDataT[8];
   uint16_t uwEmcyCodeT;

   ComEmcyConsGetData(ubNetV,ubNodeIdV,&aubDataT[0]);
   uwEmcyCodeT = aubDataT[1];
   uwEmcyCodeT = uwEmcyCodeT << 8;
   uwEmcyCodeT = uwEmcyCodeT | aubDataT[0];

   fprintf(stdout, "can%d: NID %03d - EMCY code %04X, error register value %d\n",
           ubNetV, ubNodeIdV, uwEmcyCodeT, aubDataT[2]);
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onLssEventReceive()                                                                                  //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onLssEventReceive(uint8_t ubNetV, uint8_t ubLssProtocolV)
{
   Q_UNUSED(ubLssProtocolV);


}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onMgrEventBus()                                                                                      //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onMgrEventBus(uint8_t ubNetV, CpState_ts * ptsBusStateV)
{
   Q_UNUSED(ptsBusStateV);


}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onNmtEventActiveMaster()                                                                             //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onNmtEventActiveMaster( uint8_t ubNetV, uint8_t ubPriorityV, uint8_t ubNodeIdV)
{
   Q_UNUSED(ubPriorityV);
   Q_UNUSED(ubNodeIdV);


}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onNmtEventHeartbeat()                                                                                //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onNmtEventHeartbeat(uint8_t ubNetV, uint8_t ubNodeIdV)
{
   Q_UNUSED(ubNodeIdV);


}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onNmtEventIdCollision()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onNmtEventIdCollision(uint8_t ubNetV)
{
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onNmtEventMasterDetection()                                                                          //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onNmtEventMasterDetection(uint8_t ubNetV, uint8_t ubResultV)
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


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onNmtEventStateChange()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onNmtEventStateChange( uint8_t ubNetV, uint8_t ubNodeIdV, uint8_t ubNmtEventV)
{
   switch(ubNmtEventV)
   {
      case eCOM_NMT_STATE_BOOTUP:
         fprintf(stdout, "can%d: NID %03d - received boot-up message\n",            ubNetV, ubNodeIdV);

         break;

      case eCOM_NMT_STATE_PREOPERATIONAL:
         fprintf(stdout, "can%d: NID %03d - switched to pre-operational state\n",   ubNetV, ubNodeIdV);
         break;

      case eCOM_NMT_STATE_OPERATIONAL:
         fprintf(stdout, "can%d: NID %03d - switched to operational state\n",       ubNetV, ubNodeIdV);
         break;

      case eCOM_NMT_STATE_STOPPED:
         fprintf(stdout, "can%d: NID %03d - switched to stopped state\n",           ubNetV, ubNodeIdV);
         break;

      default:

         break;
   }
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onPdoEventReceive()                                                                                  //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onPdoEventReceive(uint8_t ubNetV, uint16_t uwPdoV)
{
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onPdoEventTimeout()                                                                                  //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onPdoEventTimeout(uint8_t ubNetV, uint16_t uwPdoNumV)
{

}

//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onSdoEventObjectReady()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onSdoEventObjectReady(uint8_t ubNetV, uint8_t ubNodeIdV, CoObject_ts * ptsCoObjV, 
                                          uint32_t * pulAbortV)
{

}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onSdoEventProgress()                                                                                 //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onSdoEventProgress(uint8_t ubNetV, uint8_t ubNodeIdV, uint16_t uwIndexV, uint8_t ubSubIndexV,
                                       uint32_t ulByteCntV)
{

}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onSdoEventTimeout()                                                                                  //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::onSdoEventTimeout(uint8_t ubNetV, uint8_t ubNodeIdV, uint16_t uwIndexV, uint8_t ubSubIndexV)
{

}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::onSdoEventTimeout()                                                                                  //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void CoMasterDemo::onTimerEvent(void)
{
   //---------------------------------------------------------------------------------------------------
   // Process CAN message handling by the CANopen stack
   //
   ComMgrProcess(ubNetworkP);
   ComMgrNetTimerEvent(ubNetworkP);
}


void  CoMasterDemo::runCmdParser(void)
{
   QCommandLineParser   clCmdParserT;

   //---------------------------------------------------------------------------------------------------
   // setup command line parser, options are added in alphabetical order
   //
   clCmdParserT.setApplicationDescription(tr("CANopen Master demo"));


   //---------------------------------------------------------------------------------------------------
   // command line option: -h, --help
   //
   clCmdParserT.addHelpOption();

   
   //---------------------------------------------------------------------------------------------------
   // argument <interface> is required
   //
   clCmdParserT.addPositionalArgument("interface", 
                                      tr("CAN interface, e.g. can1"));

   //---------------------------------------------------------------------------------------------------
   // command line option: -v, --version
   //
   clCmdParserT.addVersionOption();


   //---------------------------------------------------------------------------------------------------
   // get the instance of the main application
   //
   QCoreApplication * pclAppT = QCoreApplication::instance();


   //---------------------------------------------------------------------------------------------------
   // Process the actual command line arguments given by the user
   //
   clCmdParserT.process(*pclAppT);
   const QStringList clArgsT = clCmdParserT.positionalArguments();
   if (clArgsT.size() != 1) 
   {
      fprintf(stderr, "%s\n", 
              qPrintable(tr("Error: Must specify CAN interface.\n")));
      clCmdParserT.showHelp(0);
   }
      start();
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::start()                                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void  CoMasterDemo::start(void)
{
   //---------------------------------------------------------------------------------------------------
   // The function ComMgrNetTimerEvent() is called every 10 ms inside onTimerEvent(). We tell the 
   // CANopen master stack about this update period here in order to work with correkt timer
   // ticks inside the stack. Please note that the parameter defines the time in micro-seconds.
   //
   ComTmrSetPeriod(TIMER_CYCLE_PERIOD * 1000);
   clTimerP.start(TIMER_CYCLE_PERIOD);

   //---------------------------------------------------------------------------------------------------
   // Initialise the CANopen master stack
   // The bitrate value is a dummy here, since the bitrate is set via the CANpie server configuration 
   // file.
   //
   ComMgrInit(ubCanChannelP, ubNetworkP, eCP_BITRATE_500K, ubMasterNodeIdP, eCOM_MODE_NMT_MASTER);


   //---------------------------------------------------------------------------------------------------
   // start the CANopen master stack
   //
   ComMgrStart(ubNetworkP);


   //---------------------------------------------------------------------------------------------------
   // start master detection procedure
   //
   ComNmtMasterDetection(ubNetworkP, 1);


   //---------------------------------------------------------------------------------------------------
   // CANopen master is heartbeat producer, 500ms
   //
   ComNmtSetHbProdTime(ubNetworkP, 500);
}


//--------------------------------------------------------------------------------------------------------------------//
// CoMasterDemo::stop()                                                                                               //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//   
void CoMasterDemo::stop(void)
{

}