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

#ifndef _VRJ_OPENGL_EXTENSION_LOADER_WGL_H_
#define _VRJ_OPENGL_EXTENSION_LOADER_WGL_H_

#include <vrj/Draw/OpenGL/Config.h>

#include <boost/shared_ptr.hpp>

#include <vrj/Draw/OpenGL/ExtensionLoader.h>

// WGL Defines
#define WGL_CONTEXT_MAJOR_VERSION_ARB                 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB                 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB                   0x2093
#define WGL_CONTEXT_FLAGS_ARB                         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB                  0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                     0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB        0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB              0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB     0x00000002

// New error returned by GetLastError
#define	ERROR_INVALID_VERSION_ARB                    0x2095

namespace vrj
{

namespace opengl
{

/**
 * WGL-specific class for loading extensions.
 *
 * @note This class was renamed from vrj::GlExtensionLoaderWin32 in VR Juggler
 *       2.3.11.
 */
class ExtensionLoaderWGL : public vrj::opengl::ExtensionLoader
{
public:
   ExtensionLoaderWGL();

   /** Register common extensions that we may need to use. */
   virtual void registerExtensions();

public:
   /** @name NVidia swap control. */
   //@{
   
   /** Return true if we have support for NV swap group extensions. */
   bool hasSwapGroupNV();

   BOOL wglJoinSwapGroupNV(HDC hdc, unsigned int group);

   BOOL wglBindSwapBarrierNV(unsigned int group, unsigned int barrier);

   BOOL wglQuerySwapGroupNV(HDC hdc, unsigned int* group, unsigned int* barrier);

   BOOL wglQueryMaxSwapGroupsNV(HDC hdc, unsigned int* maxGroups,
                                unsigned int* maxBarriers);

   BOOL wglQueryFrameCountNV(HDC hdc, /*int screen,*/ unsigned int* count);

   BOOL wglResetFrameCountNV(HDC hdc /*, int screen*/);
   //@}

   /** @name Context Creation/ */
   //@{
   
   //** Return true if we have support for ARB context creation. */
   bool hasCreateContextARB();

   HGLRC wglCreateContextAttribsARB(HDC hdc, HGLRC hshareContext, const int *attribList);

   //@}

private:
   struct WglFuncs;
   bool   mExtensionsRegistered;

   boost::shared_ptr<WglFuncs>   mWglFuncs;  /** Pimpl struct for holding typed functions. */
};

}  // namespace opengl

}  // namespace vrj


#endif