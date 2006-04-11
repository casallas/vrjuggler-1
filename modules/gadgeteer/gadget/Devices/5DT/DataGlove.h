/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _GADGET_DATA_GLOVE_H_
#define _GADGET_DATA_GLOVE_H_

#include <gadget/gadgetConfig.h>
#include <string>

#include <gadget/Type/Input.h>
#include <gadget/Type/Glove.h>
#include <gadget/Type/Analog.h>
#include <gadget/Type/Digital.h>
#include <gadget/Devices/5DT/DataGloveStandalone.h>

namespace gadget
{

/**
 * Software interface to 5DT Dataglove hardware.
 *
 * Submitted by: Chang Kim
 *
 * @note The 5DT wireless DataGlove is a transmit-only device.
 * @note The glove continuously emits a glove data packet.
 * @note The packet has total 10 data including header.       
 *
 * @date 4-25-02
 */
class DataGlove : virtual public Input, public Glove, public Analog
{
public:
   /** Construct */
   DataGlove() : mGlove( NULL ), mAnalogData(9)
   {;}

   /** Destroy the glove */
   virtual ~DataGlove();
   virtual bool config( jccl::ConfigChunkPtr c );
   static std::string getChunkType() { return std::string("DataGlove");}
   virtual int startSampling();
   virtual int stopSampling();
   virtual int sample();
   virtual void updateData ();
   
protected:
   /** The main control loop for the object. */
   void controlLoop( void* nullParam );
   void copyDataFromGlove();

protected:
   float ch0, ch1, ch2, ch3, ch4, ch5, ch6;
   int ch7, ch8;

   DataGloveStandalone* mGlove;           /**< The actual glove */

   // Hand left, right;
   std::vector<AnalogData> mAnalogData;   /**< analogs for each finger */
   //std::vector<DigitalData> mDigitalData; /**< digital for gesture */
                                          
};

} // End of gadget namespace


#endif   /* _GADGET_DATA_GLOVE_H_ */
