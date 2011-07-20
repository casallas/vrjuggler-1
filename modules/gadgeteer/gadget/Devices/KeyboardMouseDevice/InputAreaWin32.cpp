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

#include <gadget/gadgetConfig.h>

#include <iomanip>
#include <sstream>
#include <cctype>

#include <vpr/vpr.h>
#include <vpr/Sync/Guard.h>

#include <gadget/Type/KeyboardMouse/KeyEvent.h>
#include <gadget/Type/KeyboardMouse/MouseEvent.h>
#include <gadget/Util/Debug.h>

#include <gadget/Devices/KeyboardMouseDevice/InputAreaWin32.h> // my header

#ifndef GET_X_LPARAM
#  define GET_X_LPARAM(lp)   ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#  define GET_Y_LPARAM(lp)   ((int)(short)HIWORD(lp))
#endif

#ifndef WM_MOUSEWHEEL
// Message ID for IntelliMouse wheel
#define WM_MOUSEWHEEL WM_MOUSELAST + 1
#endif


namespace gadget
{

InputAreaWin32::InputAreaWin32()
   : mWinHandle(NULL)
   , mWidth(0)
   , mHeight(0)
   , mPrevX(0)
   , mPrevY(0)
{;}

InputAreaWin32::~InputAreaWin32()
{;}

/**
 * Do not call from any other thread than controlLoop()!!!!
 * Peek and GetMessage retrieves messages for any window
 * that belongs to the calling thread...
 */
void InputAreaWin32::handleEvents()
{
   MSG msg;
   bool have_events_to_check = false;
   if ( mBlocking )
   {
      // block until message received.
      int retval = ::GetMessage(&msg, mWinHandle, 0, 0);
      vprASSERT(retval != -1 && "invalid mWinHandle window handle or invalid lpMsg pointer");
      if ( retval == -1 ) return; // for opt mode...
      have_events_to_check = true;
   }
   // check for event, if none, then exit.
   else if ( ::PeekMessage(&msg, mWinHandle, 0, 0, PM_REMOVE) )
   {
      have_events_to_check = true;
   }
   else
   {
      have_events_to_check = false;
      return; // don't wait on the lock since there is nothing
              // afterwards that will be called when this is false.
   }

   // GUARD mKeys for duration of loop.
   // Doing it here gives makes sure that we process all events and don't get
   // only part of them for an update.
   // In order to copy data over to the mCurKeys array,
   // Lock access to the mKeys array for the duration of this function.
   vpr::Guard<vpr::Mutex> guard(mKeysLock);

   while ( have_events_to_check )
   {
      // Test if quit.
      if (msg.message == WM_QUIT)
      {
         break;
      }

      // Since we have no accelerators, no need to call
      // TranslateAccelerator here.
      ::TranslateMessage(&msg);

      if ( NULL != mKeyboardMouseDevice )
      {
         // do our own special handling of kb/mouse...
         updKeys(msg);
      }

      // send the message to the registered event handler
      ::DispatchMessage(&msg);

      // see if there is more messages immediately waiting
      // (don't block), process them all at once...
      int retval = ::PeekMessage(&msg, mWinHandle, 0, 0, PM_REMOVE);
      if ( retval != 0 ) // messages != 0, nomessages == 0
      {
         have_events_to_check = true;
      }
      else
      {
         have_events_to_check = false;
      }
   }
}

/**
 * UpdKeys: translates windows message into key updates
 * The WNDPROC uses its USERDATA pointer to the keyboard
 * to forward on messages to be handled from in the keyboard object.
 */
void InputAreaWin32::updKeys(const MSG& message)
{
/*
   vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
      << mInstName << ": KeyWin32::updKeys: Processing keys.\n"
      << vprDEBUG_FLUSH;
*/

   gadget::Keys key;

   switch ( message.message )
   {
      case WM_SIZE:
         if ( message.lParam )
         {
            InvalidateRect(mWinHandle, NULL, TRUE);
         }
         InputAreaWin32::resize(LOWORD(message.lParam), HIWORD(message.lParam));
         break;
      case WM_ACTIVATE:
         // if was activated, and not minimized
         if ( (WA_ACTIVE == LOWORD(message.wParam) ||
               WA_CLICKACTIVE == LOWORD(message.wParam)) &&
              0 == HIWORD(message.wParam) )
         {
            // and was previously locked...
            if ( Unlocked != mLockState )
            {
               lockMouseInternal();
            }
         }
         // if was deactivated and minimized
         // otherwise let CAPTURECHANGED handle the lose capture case
         else if ( WA_INACTIVE == LOWORD(message.wParam) &&
                   0 != HIWORD(message.wParam) )
         {
            // and was locked...
            if ( Unlocked != mLockState )
            {
               // we will leave mLockState in the locked state so ACTIVATE
               // puts it back.
               unlockMouseInternal();
            }
         }
         break;

         // sent to the window that is losing the mouse capture
      case WM_CAPTURECHANGED:
         // if locked, unlock it
         if ( Unlocked != mLockState )
         {
            // we will leave mLockState in the locked state so ACTIVATE puts
            // it back.
            unlockMouseInternal();
         }
         break;

         // keys and buttons
         // When we hit a key (or button), then set the mKey and mRealkey
         // When release, only set real so that we don't lose a press of the
         // button then when the updateData happens, the framekeys will be set
         // to non-pressed from real_keys

         // press
      case WM_SYSKEYDOWN:  //Need WM_SYSKEYDOWN to capture ALT key
      case WM_KEYDOWN:
         {
            // collect data about the keypress.
            key = this->VKKeyToKey(message.wParam);

            addKeyEvent(key, gadget::KeyPressEvent, message);

            // get the repeat count in case the key was pressed
            // multiple times since last we got this message
            int repeat_count = message.lParam & 0x0000ffff;

            // check if the key was down already
            // this indicates key repeat, which we [may] want to ignore.
            bool was_down = (message.lParam & 0x40000000) == 0x40000000;
            if ( was_down )
            {
               break;
            }

            // process the keypress.
            mKeyboardMouseDevice->mKeys[key] += repeat_count;
            mKeyboardMouseDevice->mRealkeys[key] += 1;

            if ( mAllowMouseLocking )
            {
               // Check for Escape from bad state
               // this provides a sort of failsafe to
               // get out of the locked state...
               // @todo this is sort of hard coded, do we want it this way?
               if ( key == gadget::KEY_ESC )
               {
                  if ( mLockState != Unlocked )
                  {
                     mLockState = Unlocked;
                     unlockMouseInternal();
                  }
               }
               else if ( mLockState == Unlocked )
               {
                  if ( key != mLockToggleKey &&
                       (gadget::KEY_ALT == key || gadget::KEY_CTRL == key ||
                        gadget::KEY_SHIFT == key) )
                  {
                     mLockState = Lock_KeyDown; // Switch state
                     mLockStoredKey = key;      // Store the VJ key that is down
                     lockMouseInternal();
                  }
                  else if ( key == mLockToggleKey )
                  {
                     mLockState = Lock_LockKey;
                     lockMouseInternal();
                  }
               }
               // Just switch the current locking state
               else if ( mLockState == Lock_KeyDown && key == mLockToggleKey )
               {
                  mLockState = Lock_LockKey;
               }
               else if ( mLockState == Lock_LockKey && key == mLockToggleKey )
               {
                  mLockState = Unlocked;
                  unlockMouseInternal();
               }
            }
         }
         break;

         // release
      case WM_SYSKEYUP:  //Need WM_SYSKEYUP to capture ALT key
      case WM_KEYUP:
         key = VKKeyToKey(message.wParam);

         mKeyboardMouseDevice->mRealkeys[key] = 0;

         /*
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << mInstName << ": WM_KEYUP: " << key << std::endl
            << vprDEBUG_FLUSH;
         */

         addKeyEvent(key, gadget::KeyReleaseEvent, message);

         // -- Update lock state -- //
         // lock_keyDown[key==storedKey]/unlockMouse -> unlocked
         if ( mAllowMouseLocking && mLockState == Lock_KeyDown &&
              key == mLockStoredKey )
         {
            mLockState = Unlocked;
            unlockMouseInternal();
         }
         break;

         // press...
      case WM_LBUTTONDOWN:
         ::SetCapture(mWinHandle);
         clipCursorArea();
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON1] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON1] += 1;

         addMouseButtonEvent(gadget::MBUTTON1, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "LeftButtonDown\n" << vprDEBUG_FLUSH;
         break;
      case WM_MBUTTONDOWN:
         ::SetCapture(mWinHandle);
         clipCursorArea();
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON2] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON2] += 1;

         addMouseButtonEvent(gadget::MBUTTON2, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "MiddleButtonDown\n" << vprDEBUG_FLUSH;
         break;
      case WM_RBUTTONDOWN:
         ::SetCapture(mWinHandle);
         clipCursorArea();
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON3] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON3] += 1;

         addMouseButtonEvent(gadget::MBUTTON3, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "RightButtonDown\n" << vprDEBUG_FLUSH;
         break;

         // release...
      case WM_LBUTTONUP:
         ::ReleaseCapture();
         ::ClipCursor(NULL);
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON1] = 0;

         addMouseButtonEvent(gadget::MBUTTON1,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "LeftButtonUp\n" << vprDEBUG_FLUSH;
         break;
      case WM_MBUTTONUP:
         ::ReleaseCapture();
         ::ClipCursor(NULL);
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON2] = 0;

         addMouseButtonEvent(gadget::MBUTTON2,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "MiddleButtonUp\n" << vprDEBUG_FLUSH;
         break;
      case WM_RBUTTONUP:
         ::ReleaseCapture();
         ::ClipCursor(NULL);
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON3] = 0;

         addMouseButtonEvent(gadget::MBUTTON3,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "RightButtonUp\n" << vprDEBUG_FLUSH;
         break;

      case WM_MOUSEWHEEL:
         // Check for handling of the deprecated scrolling behavior.
         if ( mKeyboardMouseDevice->useButtonsForScrolling() )
         {
            // Mouse wheel events are interpreted as the pressing and
            // releasing of either Button 4 or Button 5. This is the behavior
            // with the X Window System.
            gadget::Keys gadget_button;

            // A positive value in the Z delta indicates that the wheel was
            // rotated forward.
            if ( GET_WHEEL_DELTA_WPARAM(message.wParam) > 0 )
            {
               gadget_button = mKeyboardMouseDevice->getScrollUpButton();
            }
            // A negative value in the Z delta indicates that the wheel was
            // rotated backward.
            else
            {
               gadget_button = mKeyboardMouseDevice->getScrollDownButton();
            }

            // First, we treat this event as a mouse button press.
            mKeyboardMouseDevice->mRealkeys[gadget_button] = 1;
            mKeyboardMouseDevice->mKeys[gadget_button] += 1;

            addMouseButtonEvent(gadget_button, gadget::MouseButtonPressEvent,
                                message);

            // Then we treat it as a mouse button release. Again, this is to
            // be consistent with the behavior seen with X11.
            mKeyboardMouseDevice->mRealkeys[gadget_button] = 0;
            mKeyboardMouseDevice->mKeys[gadget_button] += 1;

            addMouseButtonEvent(gadget_button, gadget::MouseButtonReleaseEvent,
                                message);
         }
         // Use the new scrolling behavior.
         else
         {
            const short delta(GET_WHEEL_DELTA_WPARAM(message.wParam));

            gadget::Keys scroll_key = delta > 0 ? gadget::MOUSE_SCROLL_UP
                                                : gadget::MOUSE_SCROLL_DOWN;

            // This is not how the wheel delta is supposed to be interpreted
            // according to the API documentation. However, we are doing it
            // this way to be consistent with Cocoa. See the following for
            // more information:
            //
            // http://msdn.microsoft.com/en-us/library/ms645617(VS.85).aspx
            const float delta_y(
               static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA)
            );

            mKeyboardMouseDevice->mRealkeys[scroll_key] = 1;
            mKeyboardMouseDevice->mKeys[scroll_key] += 1;

            addMouseScrollEvent(0.0f, delta_y, message);
         }

         break;

      // Horizontal scrolling requires Windows Vista (NT 6.0) or newer.
#if _WIN32_WINNT >= 0x0600
      case WM_MOUSEHWHEEL:
         // Check for handling of the deprecated scrolling behavior.
         if ( mKeyboardMouseDevice->useButtonsForScrolling() )
         {
            // Mouse wheel events are interpreted as the pressing and
            // releasing of either Button 4 or Button 5. This is the behavior
            // with the X Window System.
            gadget::Keys gadget_button;

            // A positive value in the Z delta indicates that the wheel was
            // rotated to the right.
            if ( GET_WHEEL_DELTA_WPARAM(message.wParam) > 0 )
            {
               gadget_button = mKeyboardMouseDevice->getScrollRightButton();
            }
            // A negative value in the Z delta indicates that the wheel was
            // rotated to the left.
            else
            {
               gadget_button = mKeyboardMouseDevice->getScrollLeftButton();
            }

            // First, we treat this event as a mouse button press.
            mKeyboardMouseDevice->mRealkeys[gadget_button] = 1;
            mKeyboardMouseDevice->mKeys[gadget_button] += 1;

            addMouseButtonEvent(gadget_button, gadget::MouseButtonPressEvent,
                                message);

            // Then we treat it as a mouse button release. Again, this is to
            // be consistent with the behavior seen with X11.
            mKeyboardMouseDevice->mRealkeys[gadget_button] = 0;
            mKeyboardMouseDevice->mKeys[gadget_button] += 1;

            addMouseButtonEvent(gadget_button, gadget::MouseButtonReleaseEvent,
                                message);
         }
         // Use the new scrolling behavior.
         else
         {
            const short delta(GET_WHEEL_DELTA_WPARAM(message.wParam));

            gadget::Keys scroll_key = delta > 0 ? gadget::MOUSE_SCROLL_RIGHT
                                                : gadget::MOUSE_SCROLL_LEFT;

            // This is not how the wheel delta is supposed to be interpreted
            // according to the API documentation. However, we are doing it
            // this way to be consistent with Cocoa. See the following for
            // more information:
            //
            // http://msdn.microsoft.com/en-us/library/ms645614(VS.85).aspx
            const float delta_x(
               static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA)
            );

            mKeyboardMouseDevice->mRealkeys[scroll_key] = 1;
            mKeyboardMouseDevice->mKeys[scroll_key] += 1;

            addMouseScrollEvent(delta_x, 0.0f, message);
         }

         break;
#endif

         // mouse movement
      case WM_MOUSEMOVE:
         {
            int cur_x, cur_y, dx, dy;

            // Determine how far the mouse pointer moved since the last event.
            // x & y are relative to the x window
            cur_x = GET_X_LPARAM(message.lParam);
            cur_y = GET_Y_LPARAM(message.lParam);

            vprDEBUG(vprDBG_ALL,vprDBG_HVERB_LVL) << "MotionNotify: x:"
               << std::setw(6) << cur_x
               << "  y:" << std::setw(6) << cur_y << std::endl
               << vprDEBUG_FLUSH;

            if ( mLockState == Unlocked )
            {
               dx = cur_x - mPrevX;
               dy = cur_y - mPrevY;

               mPrevX = cur_x;
               mPrevY = cur_y;
            }
            else
            {
               int win_center_x(mLockXCenter);
               int win_center_y(mLockYCenter);

               dx = cur_x - win_center_x; // Base delta off of center of window
               dy = cur_y - win_center_y;
               mPrevX = win_center_x;    // Must do this so if state changes, we have accurate dx,dy next time
               mPrevY = win_center_y;

               // Warp back to center, IF we are not there already
               if ( (dx != 0) || (dy != 0) )
               {
                  vprDEBUG(vprDBG_ALL,vprDBG_HVERB_LVL) << "CORRECTING: x:"
                     << std::setw(6) << dx << "  y:"
                     << std::setw(6) << dy
                     << std::endl << vprDEBUG_FLUSH;

                  // convert windows coords to screen coords.
                  POINT pt;
                  pt.x = win_center_x;
                  pt.y = win_center_y;
                  ::ClientToScreen(mWinHandle, &pt);
                  ::SetCursorPos(pt.x, pt.y);
               }
            }

            // Update mKeys based on key pressed and store in the key array
            if ( dx > 0 )
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_POSX] += dx;     // Positive movement in the x direction.
            }
            else
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_NEGX] += -dx;    // Negative movement in the x direction.
            }

            if ( dy > 0 )
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_POSY] += dy;     // Positive movement in the y direction.
            }
            else
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_NEGY] += -dy;    // Negative movement in the y direction.
            }

            // dy has to be negated to change y origin from top to bottom.
            // This is for consistency to make mouse origin in lower left
            // corner.
            addMouseMoveEvent(dx, -dy, message);
         }
         break;
   } // end of switch...
}

void InputAreaWin32::lockMouseInternal()
{
   ::POINT pt;
   ::GetCursorPos(&pt);
   ::ScreenToClient(mWinHandle, &pt);
   mLockXCenter = pt.x;
   mLockYCenter = pt.y;

   // capture the mouse for this window...
   HWND previous_capture = ::SetCapture(mWinHandle);

   while (ShowCursor(FALSE) > -1)
   {
      ;
   }
}

void InputAreaWin32::unlockMouseInternal()
{
   ::ReleaseCapture();
   
   while (ShowCursor(TRUE) < 0)
   {
      ;
   }
}

void InputAreaWin32::clipCursorArea()
{
   RECT rcClient;       // client area rectangle 
   POINT ptClientUL;    // client upper left corner 
   POINT ptClientLR;    // client lower right corner 

   // Retrieve the screen coordinates of the client area, 
   // and convert them into client coordinates. 
   ::GetClientRect(mWinHandle, &rcClient); 
   ptClientUL.x = rcClient.left; 
   ptClientUL.y = rcClient.top; 

   // Add one to the right and bottom sides, because the 
   // coordinates retrieved by GetClientRect do not 
   // include the far left and lowermost pixels. 
   ptClientLR.x = rcClient.right + 1; 
   ptClientLR.y = rcClient.bottom + 1; 
   ::ClientToScreen(mWinHandle, &ptClientUL); 
   ::ClientToScreen(mWinHandle, &ptClientLR); 

   // Copy the client coordinates of the client area 
   // to the rcClient structure. Confine the mouse cursor 
   // to the client area by passing the rcClient structure 
   // to the ClipCursor function. 
   ::SetRect(&rcClient, ptClientUL.x, ptClientUL.y, ptClientLR.x,
             ptClientLR.y); 
   ::ClipCursor(&rcClient); 
}

gadget::Keys InputAreaWin32::VKKeyToKey(const int vkKey)
{
   // The X Window System provides separate key codes for keys such as ':'
   // or '*' that result from pressing SHIFT and another key. Windows does
   // not do this. To be consistent, we will factor the modifier mask in
   // to the conversion performed here.
   // XXX: This could be very specific to U.S. keyboards.
   const int mask(getModifierMask());
   const bool shift_pressed(mask & gadget::KEY_SHIFT);

   switch ( vkKey )
   {
      case VK_UP      : return gadget::KEY_UP;
      case VK_DOWN    : return gadget::KEY_DOWN;
      case VK_LEFT    : return gadget::KEY_LEFT;
      case VK_RIGHT   : return gadget::KEY_RIGHT;
      case VK_CONTROL  : return gadget::KEY_CTRL;
      case VK_SHIFT   : return gadget::KEY_SHIFT;
      case VK_MENU     : return gadget::KEY_ALT;
      case /*VK_1*/0x31   : return (shift_pressed ? gadget::KEY_EXCLAM
                                                  : gadget::KEY_1);
      case VK_NUMPAD1   : return gadget::KEY_1;
      case /*VK_2*/0x32   : return (shift_pressed ? gadget::KEY_AT
                                                  : gadget::KEY_2);
      case VK_NUMPAD2   : return gadget::KEY_2;
      case /*VK_3*/0x33   : return (shift_pressed ? gadget::KEY_NUMBER_SIGN
                                                  : gadget::KEY_3);
      case VK_NUMPAD3   : return gadget::KEY_3;
      case /*VK_4*/0x34   : return (shift_pressed ? gadget::KEY_DOLLAR
                                                  : gadget::KEY_4);
      case VK_NUMPAD4   : return gadget::KEY_4;
      case /*VK_5*/0x35   : return (shift_pressed ? gadget::KEY_PERCENT
                                                  : gadget::KEY_5);
      case VK_NUMPAD5   : return gadget::KEY_5;
      case /*VK_6*/0x36   : return (shift_pressed ? gadget::KEY_ASCII_CIRCUM
                                                  : gadget::KEY_6);
      case VK_NUMPAD6   : return gadget::KEY_6;
      case /*VK_7*/0x37   : return (shift_pressed ? gadget::KEY_AMPERSAND
                                                  : gadget::KEY_7);
      case VK_NUMPAD7   : return gadget::KEY_7;
      case /*VK_8*/0x38   : return (shift_pressed ? gadget::KEY_ASTERISK
                                                  : gadget::KEY_8);
      case VK_NUMPAD8   : return gadget::KEY_8;
      case /*VK_9*/0x39   : return (shift_pressed ? gadget::KEY_PAREN_LEFT
                                                  : gadget::KEY_9);
      case VK_NUMPAD9   : return gadget::KEY_9;
      case /*VK_0*/0x30   : return (shift_pressed ? gadget::KEY_PAREN_RIGHT
                                                  : gadget::KEY_0);
      case VK_NUMPAD0   : return gadget::KEY_0;
      case VK_ADD         : return gadget::KEY_PLUS;
      case VK_MULTIPLY    : return gadget::KEY_ASTERISK;
      case VK_SUBTRACT    : return gadget::KEY_MINUS;
      case VK_DIVIDE      : return gadget::KEY_SLASH;
      case VK_DECIMAL     : return gadget::KEY_PERIOD;
      case /*VK_A*/0x41   : return gadget::KEY_A;
      case /*VK_B*/0x42   : return gadget::KEY_B;
      case /*VK_C*/0x43   : return gadget::KEY_C;
      case /*VK_D*/0x44   : return gadget::KEY_D;
      case /*VK_E*/0x45   : return gadget::KEY_E;
      case /*VK_F*/0x46   : return gadget::KEY_F;
      case /*VK_G*/0x47   : return gadget::KEY_G;
      case /*VK_H*/0x48   : return gadget::KEY_H;
      case /*VK_I*/0x49   : return gadget::KEY_I;
      case /*VK_J*/0x4a   : return gadget::KEY_J;
      case /*VK_K*/0x4b   : return gadget::KEY_K;
      case /*VK_L*/0x4c   : return gadget::KEY_L;
      case /*VK_M*/0x4d   : return gadget::KEY_M;
      case /*VK_N*/0x4e   : return gadget::KEY_N;
      case /*VK_O*/0x4f   : return gadget::KEY_O;
      case /*VK_P*/0x50   : return gadget::KEY_P;
      case /*VK_Q*/0x51   : return gadget::KEY_Q;
      case /*VK_R*/0x52   : return gadget::KEY_R;
      case /*VK_S*/0x53   : return gadget::KEY_S;
      case /*VK_T*/0x54   : return gadget::KEY_T;
      case /*VK_U*/0x55   : return gadget::KEY_U;
      case /*VK_V*/0x56   : return gadget::KEY_V;
      case /*VK_W*/0x57   : return gadget::KEY_W;
      case /*VK_X*/0x58   : return gadget::KEY_X;
      case /*VK_Y*/0x59   : return gadget::KEY_Y;
      case /*VK_Z*/0x5a   : return gadget::KEY_Z;

      case VK_ESCAPE  : return gadget::KEY_ESC;
      case VK_TAB     : return gadget::KEY_TAB;
//      case VK_BACKTAB : return gadget::KEY_BACKTAB;
      case VK_BACK    : return gadget::KEY_BACKSPACE;
      case VK_RETURN  : return gadget::KEY_RETURN;
      case VK_INSERT  : return gadget::KEY_INSERT;
      case VK_DELETE  : return gadget::KEY_DELETE;
      case VK_PAUSE   : return gadget::KEY_PAUSE;
//      case VK_???     : return gadget::KEY_SYSREQ;
      case VK_HOME    : return gadget::KEY_HOME;
      case VK_END     : return gadget::KEY_END;
      case VK_PRIOR   : return gadget::KEY_PRIOR;
      case VK_NEXT    : return gadget::KEY_NEXT;
      case VK_CAPITAL : return gadget::KEY_CAPS_LOCK;
      case VK_NUMLOCK : return gadget::KEY_NUM_LOCK;
      case VK_SCROLL  : return gadget::KEY_SCROLL_LOCK;

      case VK_F1  : return gadget::KEY_F1;
      case VK_F2  : return gadget::KEY_F2;
      case VK_F3  : return gadget::KEY_F3;
      case VK_F4  : return gadget::KEY_F4;
      case VK_F5  : return gadget::KEY_F5;
      case VK_F6  : return gadget::KEY_F6;
      case VK_F7  : return gadget::KEY_F7;
      case VK_F8  : return gadget::KEY_F8;
      case VK_F9  : return gadget::KEY_F9;
      case VK_F10 : return gadget::KEY_F10;
      case VK_F11 : return gadget::KEY_F11;
      case VK_F12 : return gadget::KEY_F12;
      case VK_F13 : return gadget::KEY_F13;
      case VK_F14 : return gadget::KEY_F14;
      case VK_F15 : return gadget::KEY_F15;
      case VK_F16 : return gadget::KEY_F16;
      case VK_F17 : return gadget::KEY_F17;
      case VK_F18 : return gadget::KEY_F18;
      case VK_F19 : return gadget::KEY_F19;
      case VK_F20 : return gadget::KEY_F20;
      case VK_F21 : return gadget::KEY_F21;
      case VK_F22 : return gadget::KEY_F22;
      case VK_F23 : return gadget::KEY_F23;
      case VK_F24 : return gadget::KEY_F24;

      case VK_HELP  : return gadget::KEY_HELP;
      case VK_SPACE : return gadget::KEY_SPACE;

      case 0xDB          : return (shift_pressed ? gadget::KEY_BRACE_LEFT
                                                 : gadget::KEY_BRACKET_LEFT);
      case 0xDD          : return (shift_pressed ? gadget::KEY_BRACE_RIGHT
                                                 : gadget::KEY_BRACKET_RIGHT);
      case 0xDC          : return (shift_pressed ? gadget::KEY_BAR
                                                 : gadget::KEY_BACKSLASH);
      case 0xC0          : return (shift_pressed ? gadget::KEY_ASCII_TILDE
                                                 : gadget::KEY_QUOTE_LEFT);
      case VK_OEM_COMMA  : return (shift_pressed ? gadget::KEY_LESS
                                                 : gadget::KEY_COMMA);
      case VK_OEM_PERIOD : return (shift_pressed ? gadget::KEY_GREATER
                                                 : gadget::KEY_PERIOD);
      case 0xBF          : return (shift_pressed ? gadget::KEY_QUESTION
                                                 : gadget::KEY_SLASH);
      case VK_OEM_PLUS   : return (shift_pressed ? gadget::KEY_PLUS
                                                 : gadget::KEY_EQUAL);
      case VK_OEM_MINUS  : return (shift_pressed ? gadget::KEY_UNDERSCORE
                                                 : gadget::KEY_MINUS);
      case VK_OEM_1      : return (shift_pressed ? gadget::KEY_COLON
                                                 : gadget::KEY_SEMICOLON);
      case 0xDE          : return (shift_pressed ? gadget::KEY_QUOTE_DBL
                                                 : gadget::KEY_APOSTROPHE);
      case VK_LWIN       : return gadget::KEY_SUPER_L;
      case VK_RWIN       : return gadget::KEY_SUPER_R;
      case VK_APPS       : return gadget::KEY_MENU;

      default: return gadget::KEY_UNKNOWN;
   }
}

char InputAreaWin32::getAsciiKey(const int vkKey, const gadget::Keys key)
{
   const int mask(getModifierMask());
   const bool shift_pressed(mask & gadget::KEY_SHIFT);

   switch ( key )
   {
      case gadget::KEY_1: return '1';
      case gadget::KEY_2: return '2';
      case gadget::KEY_3: return '3';
      case gadget::KEY_4: return '4';
      case gadget::KEY_5: return '5';
      case gadget::KEY_6: return '6';
      case gadget::KEY_7: return '7';
      case gadget::KEY_8: return '8';
      case gadget::KEY_9: return '9';
      case gadget::KEY_0: return '0';

      case gadget::KEY_A: return (shift_pressed ? 'A' : 'a');
      case gadget::KEY_B: return (shift_pressed ? 'B' : 'b');
      case gadget::KEY_C: return (shift_pressed ? 'C' : 'c');
      case gadget::KEY_D: return (shift_pressed ? 'D' : 'd');
      case gadget::KEY_E: return (shift_pressed ? 'E' : 'e');
      case gadget::KEY_F: return (shift_pressed ? 'F' : 'f');
      case gadget::KEY_G: return (shift_pressed ? 'G' : 'g');
      case gadget::KEY_H: return (shift_pressed ? 'H' : 'h');
      case gadget::KEY_I: return (shift_pressed ? 'I' : 'i');
      case gadget::KEY_J: return (shift_pressed ? 'J' : 'j');
      case gadget::KEY_K: return (shift_pressed ? 'K' : 'k');
      case gadget::KEY_L: return (shift_pressed ? 'L' : 'l');
      case gadget::KEY_M: return (shift_pressed ? 'M' : 'm');
      case gadget::KEY_N: return (shift_pressed ? 'N' : 'n');
      case gadget::KEY_O: return (shift_pressed ? 'O' : 'o');
      case gadget::KEY_P: return (shift_pressed ? 'P' : 'p');
      case gadget::KEY_Q: return (shift_pressed ? 'Q' : 'q');
      case gadget::KEY_R: return (shift_pressed ? 'R' : 'r');
      case gadget::KEY_S: return (shift_pressed ? 'S' : 's');
      case gadget::KEY_T: return (shift_pressed ? 'T' : 't');
      case gadget::KEY_U: return (shift_pressed ? 'U' : 'u');
      case gadget::KEY_V: return (shift_pressed ? 'V' : 'v');
      case gadget::KEY_W: return (shift_pressed ? 'W' : 'w');
      case gadget::KEY_X: return (shift_pressed ? 'X' : 'x');
      case gadget::KEY_Y: return (shift_pressed ? 'Y' : 'y');
      case gadget::KEY_Z: return (shift_pressed ? 'Z' : 'z');

      case gadget::KEY_SPACE         : return ' ';

      case gadget::KEY_EXCLAM        : return '!';
      case gadget::KEY_QUOTE_DBL     : return '"';
      case gadget::KEY_NUMBER_SIGN   : return '#';
      case gadget::KEY_DOLLAR        : return '$';
      case gadget::KEY_PERCENT       : return '%';
      case gadget::KEY_AMPERSAND     : return '&';
      case gadget::KEY_APOSTROPHE    : return '\'';
      case gadget::KEY_PAREN_LEFT    : return '(';
      case gadget::KEY_PAREN_RIGHT   : return ')';
      case gadget::KEY_ASTERISK      : return '*';
      case gadget::KEY_PLUS          : return '+';
      case gadget::KEY_COMMA         : return ',';
      case gadget::KEY_MINUS         : return '-';
      case gadget::KEY_PERIOD        : return '.';
      case gadget::KEY_SLASH         : return '/';
      case gadget::KEY_COLON         : return ':';
      case gadget::KEY_SEMICOLON     : return ';';
      case gadget::KEY_LESS          : return '<';
      case gadget::KEY_EQUAL         : return '=';
      case gadget::KEY_GREATER       : return '>';
      case gadget::KEY_QUESTION      : return '?';
      case gadget::KEY_AT            : return '@';
      case gadget::KEY_BRACKET_LEFT  : return '[';
      case gadget::KEY_BACKSLASH     : return '\\';
      case gadget::KEY_BRACKET_RIGHT : return ']';
      case gadget::KEY_ASCII_CIRCUM  : return '^';
      case gadget::KEY_UNDERSCORE    : return '_';
      case gadget::KEY_QUOTE_LEFT    : return '`';
      case gadget::KEY_BRACE_LEFT    : return '{';
      case gadget::KEY_BAR           : return '|';
      case gadget::KEY_BRACE_RIGHT   : return '}';
      case gadget::KEY_ASCII_TILDE   : return '~';

      default: return '\0';
   }
}

void InputAreaWin32::addKeyEvent(const gadget::Keys& key,
                                 const gadget::EventType& type,
                                 const MSG& msg)
{
   // Windows reports multiple key-down events when a modifier key is held
   // down, but the X Window System does not. For consistency, we ignore
   // repeated key-down events for modifier keys on Windows.
   // XXX: Putting this here is a bit of a hack, but the conditional logic
   // is much cleaner in this form.
   if ( type == gadget::KeyPressEvent &&
        (key == gadget::KEY_SHIFT || key == gadget::KEY_ALT ||
         key == gadget::KEY_CTRL) &&
        mKeyboardMouseDevice->mRealkeys[key] != 0 )
   {
      return;
   }

   // The X Window System returns ASCII key characters as lowercase letters,
   // so we force Windows to do the same for consistency.
   const char ascii_rep(getAsciiKey(msg.wParam, key));
   int mask = getModifierMask();

   vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
      << "[gadget::InputAreaWin32::addKeyEvent()] Key character '"
      << ascii_rep << "' with modifier mask " << mask << std::endl
      << vprDEBUG_FLUSH;

   // TODO: Deal with Unicode.
   gadget::EventPtr key_event(new gadget::KeyEvent(type, key, mask, msg.time,
                                                   this, ascii_rep,
                                                   ascii_rep));
   mKeyboardMouseDevice->addEvent(key_event);
}

void InputAreaWin32::resize(long width, long height)
{
   mWidth = width;
   mHeight = height;
}

void InputAreaWin32::addMouseButtonEvent(const gadget::Keys& button,
                                         const gadget::EventType& type,
                                         const MSG& msg)
{
   int state = getModifierMask() | getButtonMask();

   gadget::EventPtr mouse_event(
      new gadget::MouseEvent(type, button, GET_X_LPARAM(msg.lParam),
                             mHeight - GET_Y_LPARAM(msg.lParam), msg.pt.x,
                             GetSystemMetrics(SM_CYSCREEN) - msg.pt.y, 0.0f,
                             0.0f, state, msg.time, this)
   );
   mKeyboardMouseDevice->addEvent(mouse_event);
}

void InputAreaWin32::addMouseMoveEvent(const float deltaX, const float deltaY,
                                       const MSG& msg)
{
   int state = getModifierMask() | getButtonMask();

   gadget::EventPtr mouse_event(
      new gadget::MouseEvent(gadget::MouseMoveEvent, gadget::NO_MBUTTON,
                             GET_X_LPARAM(msg.lParam),
                             mHeight - GET_Y_LPARAM(msg.lParam), msg.pt.x,
                             GetSystemMetrics(SM_CYSCREEN) - msg.pt.y, deltaX,
                             deltaY, state, msg.time, this)
   );
   mKeyboardMouseDevice->addEvent(mouse_event);
}

void InputAreaWin32::addMouseScrollEvent(const float deltaX,
                                         const float deltaY,
                                         const MSG& msg)
{
   int state = getModifierMask() | getButtonMask();

   gadget::EventPtr mouse_event(
      new gadget::MouseEvent(gadget::MouseScrollEvent, gadget::NO_MBUTTON,
                             GET_X_LPARAM(msg.lParam),
                             mHeight - GET_Y_LPARAM(msg.lParam), msg.pt.x,
                             GetSystemMetrics(SM_CYSCREEN) - msg.pt.y, deltaX,
                             deltaY, state, msg.time, this)
   );
   mKeyboardMouseDevice->addEvent(mouse_event);
}

int InputAreaWin32::getModifierMask()
{
   // Build up the modifier mask based on what modifier keys are currently
   // pressed.
   int mask(0);
   if ( mKeyboardMouseDevice->mRealkeys[gadget::KEY_SHIFT] )
   {
      mask |= gadget::SHIFT_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::KEY_CTRL] )
   {
      mask |= gadget::CTRL_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::KEY_ALT] )
   {
      mask |= gadget::ALT_MASK;
   }
   return mask;
}

int InputAreaWin32::getButtonMask()
{
   // Build up the mouse button mask based on what mouse buttons are currently
   // pressed.
   int mask(0);

   if ( mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON1] )
   {
      mask |= gadget::BUTTON1_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON2] )
   {
      mask |= gadget::BUTTON2_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON3] )
   {
      mask |= gadget::BUTTON3_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON4] )
   {
      mask |= gadget::BUTTON4_MASK;
   }
   if ( mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON5] )
   {
      mask |= gadget::BUTTON5_MASK;
   }

   return mask;
}

void InputAreaWin32::doInternalError( const std::string& msg )
{
   // Get the last error code.
   DWORD dwErr = GetLastError();
   LPTSTR lpMsgBuf;

   // Get the string representation of the error code.
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  (LPTSTR) &lpMsgBuf, 0,NULL );

   // Create a stringstream to format the output.
   std::stringstream error_output;
   error_output << msg << std::endl
                << "Failed with error: " << dwErr << ": "
                << lpMsgBuf << std::flush;

   vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
      << clrOutNORM(clrRED,"ERROR") << ": " << error_output.str() << std::endl
      << vprDEBUG_FLUSH;

   // Display a message box to inform the user of the error.
   MessageBox(NULL, error_output.str().c_str(), "VR Juggler Error",
              MB_OK | MB_ICONINFORMATION);

   // Free memory allocated by FormatMessage.
   LocalFree(lpMsgBuf);
}

} // end namespace
