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

#ifndef _CUBES_APP_
#define _CUBES_APP_

#include <vrj/vrjConfig.h>
#include <vpr/vpr.h>

#ifdef VPR_OS_Darwin
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <vector>

#include <vrj/Kernel/Kernel.h>
#include <vrj/Draw/OpenGL/App.h>
#include <vrj/Draw/OpenGL/ContextData.h>
#include <vrj/Draw/OpenGL/DrawManager.h>

#include <gmtl/Vec.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <vrj/Util/Debug.h>

#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>
#include <gadget/Type/DigitalInterface.h>

#include <vrj/Kernel/UserPtr.h>

#include <vpr/Util/DurationStatCollector.h>
#include <vpr/Perf/ProfileManager.h>



// Class to hold all context specific data
class ContextData
{
public:
   ContextData()
   {
      dlIndex  = -1;
      maxIndex = -1;
   }

public:
   int   dlIndex;
   int   maxIndex;     // For debugging purposes only!
};

class ContextTimingData
{
public:
   ContextTimingData()
      : dlist_wait(vpr::Interval::Usec, 400000)
   {;}

   vpr::DurationStatCollector dlist_wait;
};

// Class to hold all data for a specific user
class UserData
{
public:
   // Constructor
   // Takes the string names of the devices to use
   // NOTE: This means that we cannot construct a user until the input manager is loaded
   //       Ex. The Init function
   UserData(vrj::UserPtr user, std::string wandName, std::string incButton,
            std::string decButton, std::string stopButton,
            std::string toggleButton)
   {
      mCurVelocity = 0.0;
      gmtl::identity(mNavMatrix);

      mUser = user;
      // Initialize devices
      mWand.init(wandName);
      mIncVelocityButton.init(incButton);
      mDecVelocityButton.init(decButton);
      mStopButton.init(stopButton);
      mToggleButton.init(toggleButton);
   }

   // Update the navigation matrix
   //
   // Uses a quaternion to do rotation in the environment
   void updateNavigation();

public:
      // Devices to use for the given user
   gadget::PositionInterface mWand;              /**< The wand */
   gadget::DigitalInterface  mIncVelocityButton; /**< Button for velocity */
   gadget::DigitalInterface  mDecVelocityButton;
   gadget::DigitalInterface  mStopButton;        /**< Button to stop */
   gadget::DigitalInterface  mToggleButton;      /**< Button to toggle shape */

   // Navigation info for the user
   float                mCurVelocity;  // The current velocity
   gmtl::Matrix44f      mNavMatrix;    // Matrix for navigation in the application

   vrj::UserPtr         mUser;         // The user we hold data for
};

//--------------------------------------------------
// Demonstration OpenGL application class
//
// This application simply renders a field of cubes.
//---------------------------------------------------
class cubesApp : public vrj::opengl::App
{
public:
   cubesApp(vrj::Kernel* kern)
      : vrj::opengl::App(kern)
      , mProgram(0)
      , muMVMatrixHandle(0)
      , muPMatrixHandle(0)
      , muTranslationHandle(0)
      , maVertexCoordHandle(0)
      , maVertexNormalHandle(0)
      , muVertexColorHandle(0)
      , mVertexArrayBufferID(0)
      , mVertexCoordBufferID(0)
      , mVertexNormalBufferID(0)
      , mIndexBufferID(0)
      , mCurFrameNum(0)
   {
      /* Do nothing. */ ;
   }

   virtual ~cubesApp()
   {
      /* Do nothing. */ ;
   }

   // Execute any initialization needed before the API is started.  Put device
   // initialization here.
   virtual void init();

   // Execute any initialization needed <b>after</b> API is started
   //  but before the drawManager starts the drawing loops.
   virtual void apiInit()
   {
      vprDEBUG(vprDBG_ALL, vprDBG_VERB_LVL)
         << "---- cubesApp::apiInit() ----\n" << vprDEBUG_FLUSH;
   }

   // Called immediately upon opening a new OpenGL context.  This is called
   // once for every display window that is opened.  Put OpenGL resource
   // allocation here.
   virtual void contextInit();

   // Called immediately upon closing an OpenGL context
   // (called for every window that is closed)
   // put your opengl deallocation here...
   virtual void contextClose();

   /**   name Drawing Loop Functions
    *
    *  The drawing loop will look similar to this:
    *
    *  while (drawing)
    *  {
    *        preFrame();
    *       draw();
    *        intraFrame();      // Drawing is happening while here
    *       sync();
    *        postFrame();      // Drawing is now done
    *
    *       UpdateTrackers();
    *  }
    *
    */

   // Function called after tracker update but before start of drawing.  Do
   // calculations and state modifications here.
   virtual void preFrame()
   {
      VPR_PROFILE_GUARD("cubesApp::preFrame");
       vprDEBUG(vprDBG_ALL, vprDBG_HVERB_LVL) << "cubesApp::preFrame()"
                                              << std::endl << vprDEBUG_FLUSH;

       for ( unsigned int i = 0; i < mUserData.size(); ++i )
       {
          mUserData[i]->updateNavigation();     // Update the navigation matrix
       }

       ++mCurFrameNum;     // Goto next frame
   }

   virtual void pipePreDraw()
   {
      if((mCurFrameNum % 50) == 0)
      { 
         ContextTimingData* timing_data = &(*mContextTiming);
         double mean = timing_data->dlist_wait.getMean();
         vprDEBUG(vprDBG_ALL, vprDBG_VERB_LVL) << "dlist wait: " << mean
                                               << std::endl << vprDEBUG_FLUSH;
      }
   }

   // Function to draw the scene.  Put OpenGL draw functions here.
   //
   // PRE: OpenGL state has correct transformation and buffer selected
   // POST: The current scene has been drawn
   virtual void draw()
   {
      VPR_PROFILE_GUARD("cubesApp::draw");

      vrj::opengl::ExtensionLoaderGL& gl =
         vrj::opengl::DrawManager::instance()->getGL();
      gl.Clear(GL_DEPTH_BUFFER_BIT);

      //initGLState();    // This should really be in another function

      myDraw(vrj::opengl::DrawManager::instance()->currentUserData()->getUser());
   }

   // Clear the buffer each frame.
   virtual void bufferPreDraw()
   {
      vrj::opengl::ExtensionLoaderGL& gl =
         vrj::opengl::DrawManager::instance()->getGL();
      gl.ClearColor(0.0, 0.0, 0.0, 0.0);
      gl.Clear(GL_COLOR_BUFFER_BIT);
   }

   /// Function called after drawing has been triggered but BEFORE it completes.
   virtual void intraFrame()
   {
      VPR_PROFILE_GUARD("cubesApp::intraFrame");
      vprDEBUG(vprDBG_ALL, vprDBG_HVERB_LVL) << "cubesApp::intraFrame()" << std::endl
                           << vprDEBUG_FLUSH;
   }

   // Function called before updating trackers but after the frame is drawn.
   // Do calculations here.
   virtual void postFrame()
   {
      vprDEBUG(vprDBG_ALL, vprDBG_HVERB_LVL)
         << "cubesApp::postFrame" << std::endl << vprDEBUG_FLUSH;
   }

   // Make sure that all our dependencies are satisfied.
   // Make sure that there are Users registered with the system.
   virtual bool depSatisfied()
   {
      // We can't start until there are users registered wth the system
      // We rely upon users to keep track of the multi-user data structure
      size_t num_users = vrj::Kernel::instance()->getUsers().size();
      return (num_users > 0);
   }

   virtual void exit()
   {
      VPR_PROFILE_RESULTS();
   }

private:

   //----------------------------------------------
   //  Draw the scene.  A bunch of boxes of differing color and stuff
   //----------------------------------------------
   void myDraw(vrj::UserPtr user);
   void initGLState();

private:
   static GLfloat sVertexData[];
   static GLfloat sNormalData[];
   static GLushort sIndices[];

   GLuint mProgram;
   GLint muMVMatrixHandle;
   GLint muPMatrixHandle;
   GLint muTranslationHandle;
   GLint maVertexCoordHandle;
   GLint maVertexNormalHandle;
   GLint muVertexColorHandle;
   GLuint mVertexArrayBufferID;
   GLuint mVertexCoordBufferID;
   GLuint mVertexNormalBufferID;
   GLuint mIndexBufferID;

public:
   std::vector<UserData*>           mUserData;    // All the users in the program
   vpr::TSObjectProxy<ContextTimingData>  mContextTiming;

   long                             mCurFrameNum;     // Current frame number count
};


#endif

