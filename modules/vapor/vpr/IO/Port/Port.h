/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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

#ifndef _VPR_PORT_H_
#define _VPR_PORT_H_

#include <string>

#include <IO/BlockIO.h>


namespace vpr {

// ----------------------------------------------------------------------------
//: A cross-platform interface to using a computer's I/O ports (serial,
//+ parallel, IR, etc.).
// ----------------------------------------------------------------------------
//! PUBLIC_API:
class Port : public BlockIO {
public:

protected:
    // ------------------------------------------------------------------------
    //: Constructor.
    //
    //! PRE: None.
    //! POST: The given port name is passed on to the vpr::BlockIO
    //+       constructor.
    //
    //! ARGS: port_name - The name of the port in use.
    // ------------------------------------------------------------------------
    Port (const std::string& port_name)
        : BlockIO(port_name)
    {
        /* Do nothing. */ ;
    }

    // ------------------------------------------------------------------------
    //: Destructor.
    // ------------------------------------------------------------------------
    virtual ~Port(void) {
        /* Do nothing. */ ;
    }
};

}; // End of vpr namespace


#endif	/* _VPR_PORT_H_ */
