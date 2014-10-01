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

#include <vrj/Draw/OpenGL/Config.h>

#include <gmtl/Matrix.h>
#include <gmtl/Generate.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Output.h>

#include <jccl/Config/ConfigElement.h>

#include <vrj/Kernel/User.h>

#include <vrj/Draw/OpenGL/DrawManager.h>

#include <vrj/Display/DisplayManager.h>
#include <vrj/Display/Display.h>
#include <vrj/Display/Projection.h>
#include <vrj/Display/Viewport.h>
#include <vrj/Display/SimViewport.h>
#include <vrj/Display/SurfaceViewport.h>

#include <vrj/Draw/OpenGL/DrawHeadFunctors.h>
#include <vrj/Draw/OpenGL/DrawWandFunctors.h>
#include <vrj/Draw/OpenGL/DrawAxesFunctors.h>
#include <vrj/Draw/OpenGL/SimInterfaceFactory.h>
#include <vrj/Draw/OpenGL/BasicSimulator.h>

#if defined(VPR_OS_Darwin) && defined(VRJ_USE_COCOA)
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

namespace vrj
{

namespace opengl
{

VRJ_REGISTER_GL_SIM_INTERFACE_CREATOR(BasicSimulator);

BasicSimulator::BasicSimulator()
{
   ;
}

BasicSimulator::~BasicSimulator()
{
   if ( NULL != mDrawWandFunctor )
   {
      delete mDrawWandFunctor;
      mDrawWandFunctor = NULL;
   }

   if ( NULL != mDrawHeadFunctor )
   {
      delete mDrawHeadFunctor;
      mDrawHeadFunctor = NULL;
   }

   if ( NULL != mDrawAxesFunctor )
   {
      delete mDrawAxesFunctor;
      mDrawAxesFunctor = NULL;
   }
}

// Configure the basic simulator config
bool BasicSimulator::config(jccl::ConfigElementPtr element)
{
   vprASSERT(element.get() != NULL);
   vprASSERT(element->getID() == std::string("default_simulator"));

   std::string camera_proxy_str = element->getProperty<std::string>("camera_pos");
   std::string wand_proxy_str = element->getProperty<std::string>("wand_pos");

   mCamera.init(camera_proxy_str);
   mWand.init(wand_proxy_str);      // Configure the wand to use

   if ( ! mCamera.isConnected() )
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << clrOutNORM(clrRED, "ERROR")
         << ": [vrj::opengl::BasicSimulator::config()] Fatal Error: Camera "
         << "not found named: " << camera_proxy_str << vprDEBUG_FLUSH;
      vprASSERT(false);
   }

   // Get drawing parameters
   mDrawProjections = element->getProperty<bool>("draw_projections");
   mSurfaceColor[0] = element->getProperty<float>("surface_color", 0);
   mSurfaceColor[1] = element->getProperty<float>("surface_color", 1);
   mSurfaceColor[2] = element->getProperty<float>("surface_color", 2);

   return true;
}

void BasicSimulator::contextInit()
{
   ExtensionLoaderGL& gl = DrawManager::instance()->getGL();
   if (gl.CreateShader != NULL)
   {
      setDrawHeadFunctor(new DrawEllipsoidHeadCoreFunctor());
      setDrawWandFunctor(new DrawRightAngleWandCoreFunctor());
      setDrawAxesFunctor(new DrawAxesCoreFunctor());
   }
   else
   {
      setDrawHeadFunctor(new DrawEllipsoidHeadFunctor());
      setDrawWandFunctor(new DrawRightAngleWandFunctor());
      setDrawAxesFunctor(new DrawAxesFunctor());
   }

   initSimulator();
}

/**
 * Draws this sim device using the given information about the Window it
 * will be drawing into.
 */
void BasicSimulator::draw(const float scaleFactor)
{
   drawSimulator(scaleFactor);
}

/**
 * Sets the keyboard/mouse device the simulator can use to get input from the
 * user.
 */
void BasicSimulator::setKeyboardMouse(gadget::KeyboardMouseInterface)
{
   /* Do nothing. */ ;
}

void BasicSimulator::updateProjectionData(const float positionScale,
                                          ProjectionPtr leftProj,
                                          ProjectionPtr rightProj)
{
   updateInternalData(positionScale);

   gmtl::Matrix44f camera_pos = getCameraPos();
   gmtl::Vec3f camera_trans = gmtl::makeTrans<gmtl::Vec3f>(camera_pos);

   //mCameraProj->calcViewMatrix(camera_pos);

   gmtl::Matrix44f left_eye_pos, right_eye_pos;     // NOTE: Eye coord system is -z forward, x-right, y-up

   // -- Calculate camera (eye) Positions -- //
   vprDEBUG(vprDBG_ALL, vprDBG_HEX_LVL)
      << "[vrj::BasicSimulator::updateProjectionData()] Getting cam position"
      << std::endl << vprDEBUG_FLUSH;
   vprDEBUG(vprDBG_ALL, vprDBG_HEX_LVL) << "CamPos:" << camera_trans
                                        << std::endl << vprDEBUG_FLUSH;

   // Compute location of left and right eyes
   float interocular_dist = mSimViewport->getUser()->getInterocularDistance();
   interocular_dist *= positionScale;               // Scale into correct units
   float eye_offset = interocular_dist / 2.0f;      // Distance to move eye

   left_eye_pos = camera_pos * gmtl::makeTrans<gmtl::Matrix44f>( gmtl::Vec3f(-eye_offset, 0.0f, 0.0f) );
   right_eye_pos = camera_pos * gmtl::makeTrans<gmtl::Matrix44f>( gmtl::Vec3f(eye_offset, 0.0f, 0.0f) );

   // NOTE: Eye coord system is -z forward, x-right, y-up
   const gmtl::Point3f left_eye_pt(
      gmtl::makeTrans<gmtl::Point3f>(left_eye_pos)
   );
   const gmtl::Point3f right_eye_pt(
      gmtl::makeTrans<gmtl::Point3f>(right_eye_pos)
   );
   leftProj->calcViewMatrix(left_eye_pos, left_eye_pt, positionScale);
   rightProj->calcViewMatrix(right_eye_pos, right_eye_pt, positionScale);
}

/**  Update internal simulator data */
void BasicSimulator::updateInternalData(const float positionScale)
{
   mHeadPos = mSimViewport->getUser()->getHeadPosProxy()->getData(positionScale);
   mWandPos = mWand->getData(positionScale);

   mCameraPos = mCamera->getData(positionScale);
   gmtl::invert(mCameraPos);
}

// Draw any objects that we need to display in the scene
// from the system.  (i.e. Gloves, etc)
// XXX: Performance Critical problems here
void BasicSimulator::drawObjects()
{
   ExtensionLoaderGL& gl = DrawManager::instance()->getGL();
   gl.PushAttrib( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT );
   {
      gl.Disable(GL_LIGHTING);
      gl.Disable(GL_BLEND);
      //vjInputManager*  input_mgr = Kernel::instance()->getInputManager();

      // Draw all glove Proxies that have drawing flag set
      /* XXX: Broken because we don't have list anymore
      GloveProxy* cur_glove_proxy;
      for (int glv = 0; glv < input_mgr->getNumGloveProxies(); ++glv)    // For each glove in system
      {
         cur_glove_proxy = input_mgr->getGloveProxy(glv);         // Get the glove proxy
         if (cur_glove_proxy->isVisible())                         // If flag set
            drawGlove( cur_glove_proxy );                           // draw it
      }
      */

      // Draw any other object that need to be seen
   }
   gl.PopAttrib();
}

/**
 * Draw the projections.
 *
 * @post Draws the projections.
 *       If withApex, then it draws the frustums with different colors.
 *       If !withApex, then just draws the surfaces in all white.
 */
void BasicSimulator::drawProjections(const bool drawFrustum,
                                     const gmtl::Vec3f& surfColor,
                                     const float scaleFactor)
{
#if defined(SKIP_THIS)
   const float ALPHA_VALUE(0.25f);

   DisplayManager* display_man = 
      vrj::opengl::DrawManager::instance()->getDisplayManager();
   display_man->updateProjections(scaleFactor);                     // Update all projections for drawing

   std::vector<vrj::DisplayPtr> disps = display_man->getAllDisplays();

   gmtl::Vec3f apex, ur, lr, ul, ll;
   ProjectionPtr proj;

   for ( unsigned int i = 0; i < disps.size(); ++i )
   {
      for ( unsigned int v = 0; v < disps[i]->getNumViewports(); ++v )
      {
         ViewportPtr view_port = disps[i]->getViewport(v);

         if ( view_port->isSurface() )
         {
            // Get a pointer to the surface viewport.
            SurfaceViewportPtr surf_vp =
               boost::dynamic_pointer_cast<SurfaceViewport>(view_port);
            vprASSERT(surf_vp.get() != NULL);

            for ( unsigned int proj_num = 0; proj_num < 2; ++proj_num )
            {
               if ( 0 == proj_num )
               {
                  proj = surf_vp->getLeftProj();
               }
               else
               {
                  proj = surf_vp->getRightProj();
               }

               // Create color values that are unique
               // Basically count in binary (skipping 0), and use the first 3
               // digits.  That will give six colors
               const int red_on = i & 0x1;
               const int green_on = (i >> 1) & 0x1;
               const int blue_on = (i >> 2) & 0x1;

               float red(0.0f), green(0.0f), blue(0.0f);
               if ( red_on > 0 )
               {
                  red = 1.0f;
               }
               if ( green_on > 0 )
               {
                  green = 1.0f;
               }
               if ( blue_on > 0 )
               {
                  blue = 1.0f;
               }

               if ( ! red_on && ! blue_on && ! green_on )      // Case of 0's (black is bad)
               {
                  red = blue = green = 0.75f;
               }

               gmtl::Vec3f surf_color;
               if ( drawFrustum )
               {
                  surf_color = gmtl::Vec3f(red, blue, green);
               }
               else
               {
                  surf_color = surfColor;
               }

               gmtl::Vec3f apex_color(surf_color);

               if ( 1 == proj_num )  // Right eye
               {
                  apex_color = gmtl::Vec3f(1.0f, 1.0f, 1.0f) - apex_color;    // Invert it
               }

               // Compute scaled colors for the corners.
               // The lower left is going to be lighter, and the upper right
               // is going to be darker.
               const float ll_scale(0.10f);
               const float ul_scale(0.55f);
               const float ur_scale(1.0f);
               const gmtl::Vec4f ll_clr(ll_scale * surf_color[0],
                                        ll_scale * surf_color[1],
                                        ll_scale * surf_color[2],
                                        ALPHA_VALUE);
               const gmtl::Vec4f ul_clr(ul_scale * surf_color[0],
                                        ul_scale * surf_color[1],
                                        ul_scale * surf_color[2],
                                        ALPHA_VALUE);
               const gmtl::Vec4f lr_clr(ul_scale * surf_color[0],
                                        ul_scale * surf_color[1],
                                        ul_scale * surf_color[2],
                                        ALPHA_VALUE);
               const gmtl::Vec4f ur_clr(ur_scale * surf_color[0],
                                        ur_scale * surf_color[1],
                                        ur_scale * surf_color[2],
                                        ALPHA_VALUE);

               // Draw the thingy
               proj->getFrustumApexAndCorners(apex, ur, lr, ul, ll);
               vprDEBUG(vrjDBG_DRAW_MGR, vprDBG_STATE_LVL)
                  << "apex: " << apex << std::endl << vprDEBUG_FLUSH;

               glColor4fv(&apex_color[0]);
               glPushMatrix();
                  if ( drawFrustum )
                  {
                     drawLine(apex, ur);
                     drawLine(apex, lr);
                     drawLine(apex, ul);
                     drawLine(apex, ll);
                  }

                  glColor4fv(&ur_clr[0]);
                  // Draw the outline
                  drawLine(ur, lr);
                  drawLine(lr, ll);
                  drawLine(ll, ul);
                  drawLine(ul, ur);

                  // Draw the surface
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  glBegin(GL_TRIANGLES);
                  glColor4fv(ll_clr.mData);  glVertex3fv(ll.mData);
                  glColor4fv(lr_clr.mData);  glVertex3fv(lr.mData);
                  glColor4fv(ur_clr.mData);  glVertex3fv(ur.mData);

                  glColor4fv(ur_clr.mData); glVertex3fv(ur.mData);
                  glColor4fv(ul_clr.mData); glVertex3fv(ul.mData);
                  glColor4fv(ll_clr.mData); glVertex3fv(ll.mData);
                  glEnd();
                  glDisable(GL_BLEND);
               glPopMatrix();
            }
         }  // if surface
      }  // for viewports
   }  // for disps
#endif
}

void BasicSimulator::initSimulator()
{
   mDrawHeadFunctor->contextInit();
   mDrawWandFunctor->contextInit();
   mDrawAxesFunctor->contextInit();
}

/**
 * Draws a simulator using OpenGL commands.
 * @note This is called internally by the library.
 */
void BasicSimulator::drawSimulator(const float scaleFactor)
{
   ExtensionLoaderGL& gl = DrawManager::instance()->getGL();

#if defined(SKIP_THIS)
   glPushAttrib(GL_ALL_ATTRIB_BITS);
   {
      // Test to see whether there is lighting active. If there is, we will
      // use it.
      GLboolean lighting_on;
      glGetBooleanv(GL_LIGHTING, &lighting_on);

      //-----------------set up materials....
      const GLfloat mat_ambient[]   = { 0.1f, 0.1f, 0.1f, 1.0f };
      const GLfloat mat_shininess[] = { 50.0f };
      const GLfloat mat_diffuse[]   = { 0.7f, 0.7f, 0.7f, 1.0f };
      const GLfloat mat_specular[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
      //-----------------Call Materials.....
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
      glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
      //----------------Enable Materials.....
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      glShadeModel(GL_SMOOTH);

      glDisable(GL_TEXTURE_2D);
      glDisable(GL_TEXTURE_1D);

      // Draw base coordinate axis
      ///*
      if ( lighting_on == GL_TRUE )
      {
         gl.Disable(GL_LIGHTING);
      }
#endif

      gl.PushMatrix();
         gl.LoadIdentity();
         mDrawAxesFunctor->setScaleFactor(scaleFactor);
         mDrawAxesFunctor->draw(mSimViewport->getUser());
      gl.PopMatrix();
      //*/

#if defined(SKIP_THIS)
      const GLfloat light0_ambient[]  = { 0.1f,  0.1f,  0.1f, 1.0f };
      const GLfloat light0_diffuse[]  = { 0.8f,  0.8f,  0.8f, 1.0f };
      const GLfloat light0_specular[] = { 1.0f,  1.0f,  1.0f, 1.0f };
      const GLfloat light0_position[] = { 0.0f, 0.75f, 0.75f, 0.0f };

      glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
      glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
      glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

      // At this point, lighting is disabled. We need it back on for the
      // draw functors.
      gl.Enable(GL_LIGHTING);
      gl.Enable(GL_LIGHT0);
#endif

      // Enable depth testing so that the rendered simulator objects look
      // correct. For example, in the case of the default head draw functor,
      // without this, we end up being able to see the eyes when looking at
      // the back of the head.
      gl.Enable(GL_DEPTH_TEST);

      // Draw the user's head
      gl.PushMatrix();
         gl.LoadIdentity();
         gl.MultMatrix(getHeadPos());
         gl.Scale(scaleFactor, scaleFactor, scaleFactor);
         //glEnable(GL_NORMALIZE);
         mDrawHeadFunctor->draw(mSimViewport->getUser());
         mDrawAxesFunctor->setScaleFactor(0.15f);
         mDrawAxesFunctor->draw(mSimViewport->getUser());
      gl.PopMatrix();

      // Draw the wand
      gl.PushMatrix();
         gl.LoadIdentity();
         gl.MultMatrix(getWandPos());
         gl.Scale(scaleFactor, scaleFactor, scaleFactor);
         //glEnable(GL_NORMALIZE);
         mDrawWandFunctor->draw(mSimViewport->getUser());
         mDrawAxesFunctor->setScaleFactor(0.15f);
         mDrawAxesFunctor->draw(mSimViewport->getUser());
      gl.PopMatrix();

#if defined(SKIP_THIS)
      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING);

      // Draw the display surfaces.
      glPushMatrix();
         glLoadIdentity();
         drawProjections(shouldDrawProjections(), getSurfaceColor(),
                         scaleFactor);
      glPopMatrix();
   }
   glPopAttrib();
#endif
}

#if defined(SKIP_THIS)
void BasicSimulator::drawLine(const gmtl::Vec3f& start,
                              const gmtl::Vec3f& end)
{
   glBegin(GL_LINES);
      glVertex3fv(start.mData);
      glVertex3fv(end.mData);
   glEnd();
}

void BasicSimulator::drawBox(const float size, const GLenum type)
{
  static GLfloat n[6][3] =
  {
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0}
  };

  static GLint faces[6][4] =
  {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };
  GLfloat v[8][3];
  GLint i;

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

  for (i = 5; i >= 0; i--) {
    glBegin(type);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}


void BasicSimulator::drawWireCube(const float size)
{
  drawBox(size, GL_LINE_LOOP);
}

void BasicSimulator::drawSolidCube(const float size)
{
  drawBox(size, GL_QUADS);
}
#endif

/*
// Draw a glove outline for the user
// This may be ugly for now.
// For each finger, step down it's xforms drawing the finger
// links as you go.
void BasicSimulator::drawGlove(gadget::GloveProxy* gloveProxy)
{
   gmtl::Matrix44f    base_glove_pos = gloveProxy->getPos();    // Get the location of the base coord system
   gadget::GloveData gd = gloveProxy->getData();               // Get the glove data
   gmtl::Vec3f      origin(0.0f,0.0f,0.0f);

   //glPushAttrib( GL_ENABLE_BIT | GL_LIGHTING_BIT );
   {
      //-----------------set up materials....
      float mat_ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
      float mat_shininess[] = {50.0f};
      float mat_diffuse[] = {.7f, .7f, .7f, 1.0f};
      float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
      //-----------------Call Materials.....
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
      glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
      //----------------Enable Materials.....
      //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
      //glEnable(GL_COLOR_MATERIAL);
      glDisable(GL_COLOR_MATERIAL);

      //-----------------set up a light....
      //GLfloat light_ambient[] = { 0.1f,  0.1f,  0.1f,  1.0f};
      //GLfloat light_diffuse[] = { 0.8f,  0.8f,  0.8f,  1.0f};
      //GLfloat light_specular[] = { 1.0f,  1.0f,  1.0f,  1.0f};
      //GLfloat light_position[] = {0.0f, 0.75f, 0.75f, 0.0f};
      //-----------------Call the light....
      //glLightfv(GL_LIGHT7, GL_AMBIENT,  light_ambient);
      //glLightfv(GL_LIGHT7, GL_DIFFUSE,  light_diffuse);
      //glLightfv(GL_LIGHT7, GL_SPECULAR,  light_specular);
      //glLightfv(GL_LIGHT7, GL_POSITION,  light_position);

      //glEnable(GL_LIGHTING);
      //glEnable(GL_LIGHT7);

      glPushMatrix();
      {
         glColor3f(0.76f, 0.80f, 0.95f);
         glLineWidth(3.0f);
         // Get to hand coord system to start drawing
         glMultMatrixf(base_glove_pos.mData);

         // Draw PALM
         glPushMatrix();
         {
            glScalef(1.0f, 0.75f, 0.1f);
            drawSphere((1.5f/12.0f), 12, 6);

            //drawSolidCube((1.0f/12.0f));      // 1 in. cube
         }
         glPopMatrix();

         // Draw INDEX finger
         glPushMatrix();
         {
            glMultMatrixf(gd.xforms[gadget::GloveData::INDEX][gadget::GloveData::MPJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::INDEX][gadget::GloveData::PIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::INDEX][gadget::GloveData::PIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::INDEX][gadget::GloveData::DIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::INDEX][gadget::GloveData::DIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::INDEX][gadget::GloveData::DIJ+1]);
         }
         glPopMatrix();

         // Draw MIDDLE finger
         glPushMatrix();
         {
            glMultMatrixf(gd.xforms[gadget::GloveData::MIDDLE][gadget::GloveData::MPJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::MIDDLE][gadget::GloveData::PIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::MIDDLE][gadget::GloveData::PIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::MIDDLE][gadget::GloveData::DIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::MIDDLE][gadget::GloveData::DIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::MIDDLE][gadget::GloveData::DIJ+1]);
         }
         glPopMatrix();

         // Draw RING finger
         glPushMatrix();
         {
            glMultMatrixf(gd.xforms[gadget::GloveData::RING][gadget::GloveData::MPJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::RING][gadget::GloveData::PIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::RING][gadget::GloveData::PIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::RING][gadget::GloveData::DIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::RING][gadget::GloveData::DIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::RING][gadget::GloveData::DIJ+1]);
         }
         glPopMatrix();

         // Draw PINKY finger
         glPushMatrix();
         {
            glMultMatrixf(gd.xforms[gadget::GloveData::PINKY][gadget::GloveData::MPJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::PINKY][gadget::GloveData::PIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::PINKY][gadget::GloveData::PIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::PINKY][gadget::GloveData::DIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::PINKY][gadget::GloveData::DIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::PINKY][gadget::GloveData::DIJ+1]);
         }
         glPopMatrix();

         // Draw THUMB
         glPushMatrix();
         {
            glMultMatrixf(gd.xforms[gadget::GloveData::THUMB][gadget::GloveData::MPJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::THUMB][gadget::GloveData::PIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::THUMB][gadget::GloveData::PIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::THUMB][gadget::GloveData::DIJ]);
            glMultMatrixf(gd.xforms[gadget::GloveData::THUMB][gadget::GloveData::DIJ].mData);
            drawSphere((0.1f/12.0f), 4, 4);
            drawLine(origin, gd.dims[gadget::GloveData::THUMB][gadget::GloveData::DIJ+1]);
         }
         glPopMatrix();
      }
      glPopMatrix();
   }
   //glPopAttrib();
}
*/

}

}