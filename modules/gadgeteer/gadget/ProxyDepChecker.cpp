/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2011 by Iowa State University
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

#include <gadget/gadgetConfig.h>

#include <gadget/Type/AnalogProxy.h>
#include <gadget/Type/DigitalProxy.h>
#include <gadget/Type/PositionProxy.h>
#include <gadget/Type/GloveProxy.h>
#include <gadget/Type/GestureProxy.h>
//#include <gadget/Type/EventWindowProxy.h>
#include <gadget/Type/KeyboardMouseProxy.h>
#include <gadget/Type/RumbleProxy.h>
#include <gadget/Type/HatProxy.h>
#include <gadget/ProxyDepChecker.h>
#include <jccl/Config/ConfigElement.h>

namespace gadget
{

bool ProxyDepChecker::canHandle(jccl::ConfigElementPtr element)
{
   std::string element_type(element->getID());

   bool ret_val;
   ret_val = ((element_type == AnalogProxy::getElementType()) ||
              (element_type == DigitalProxy::getElementType()) ||
              (element_type == GestureProxy::getElementType()) ||
              (element_type == GloveProxy::getElementType()) ||
              (element_type == KeyboardMouseProxy::getElementType()) ||
              (element_type == PositionProxy::getElementType()) ||
              (element_type == RumbleProxy::getElementType()) ||
              (element_type == HatProxy::getElementType()));

   return ret_val;
}


} // End of gadget namespace
