/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2010 by Iowa State University
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _GADGET_X_IST_H_
#define _GADGET_X_IST_H_

#include <gadget/Devices/DriverConfig.h>
#include <string>
#include <vector>

#include <gadget/Type/Input.h>
#include <gadget/Type/Analog.h>
#include <gadget/Type/InputBaseTypes.h>
#include <drivers/noDNA/X-IST/X-ISTStandalone.h>


namespace gadget
{

/**
 * Software interface to noDNA X-IST hardware.
 */
class X_IST : public input_analog_t
{
   int mGloveNumber;
public:
   /** Construct */
   X_IST();

   /** Destroy the glove */
   virtual ~X_IST();

   virtual bool config(jccl::ConfigElementPtr e);
   static std::string getElementType();
   virtual bool startSampling();
   virtual bool stopSampling();
   virtual bool sample();
   virtual void updateData();

private:
   /** The main control loop for the object. */
   void controlLoop();
   void copyDataFromGlove();

   X_ISTStandalone* mGlove;           /**< The actual glove */

   std::vector<AnalogData> mAnalogData;   /**< analogs for each finger */
   bool mExitFlag;
};

} // End of gadget namespace


#endif   /* _GADGET_X_IST_H_ */
