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

#include <vprConfig.h>

#include <Sync/Mutex.h>
#include <Threads/Thread.h>
#include <Utils/StreamLock.h>
#include <Utils/Debug.h>


namespace vpr {

/*
Debug* Debug::_instance = NULL;
Mutex  Debug::_inst_lock;
*/

vprSingletonImp(Debug);


Debug::Debug()
{
   indentLevel = 0;     // Initialy don't indent
   debugLevel = 1;      // Should actually try to read env variable

   char* debug_lev = getenv("VPR_DEBUG_NFY_LEVEL");
   if(debug_lev != NULL)
   {
      debugLevel = atoi(debug_lev);
      std::cout << "VPR_DEBUG_NFY_LEVEL: Set to " << debugLevel << std::endl
                << std::flush;
   } else {
      std::cout << "VPR_DEBUG_NFY_LEVEL: Not found. " << std::endl
                << std::flush;
      std::cout << "VPR_DEBUG_NFY_LEVEL: Defaults to " << debugLevel
                << std::endl << std::flush;
   }

   setDefaultCategoryNames();
   getAllowedCatsFromEnv();
}

std::ostream& Debug::getStream(int cat, int level, bool show_thread_info,
                               bool use_indent, int indentChange,
                               bool lockStream)
{
   if(indentChange < 0)                // If decreasing indent
      indentLevel += indentChange;

   //cout << "VPR " << level << ": ";

   // Lock the stream
#ifdef LOCK_DEBUG_STREAM
   if(lockStream)
   {
      debugLock().acquire();     // Get the lock
   }
#endif

   // --- Create stream header --- //
   /*
   if(show_thread_info)
      std::cout << vprDEBUG_STREAM_LOCK << Thread::self() << " VPR:";
   else
      std::cout << vprDEBUG_STREAM_LOCK << "              ";
   */

   // Ouput thread info
   // If not, then output space if we are also using indent (assume this means
   // new line used)
   if(show_thread_info)
      std::cout << "[" << vpr::Thread::self() << "] VPR: ";
   else if(use_indent)
      std::cout << "                  ";


      // Insert the correct number of tabs into the stream for indenting
   if(use_indent)
   {
      for(int i=0;i<indentLevel;i++)
         std::cout << "\t";
   }

   if(indentChange > 0)             // If increasing indent
      indentLevel += indentChange;

   return std::cout;
}

void Debug::addCategoryName(std::string name, int cat)
{
   mCategoryNames[name] = cat;
}

void Debug::addAllowedCategory(int cat)
{
   if((int)mAllowedCategories.size() < (cat+1))
      growAllowedCategoryVector(cat+1);

   mAllowedCategories[cat] = true;
}

// Are we allowed to print this category??
bool Debug::isCategoryAllowed(int cat)
{
   // If no entry for cat, grow the vector
   if((int)mAllowedCategories.size() < (cat+1))
      growAllowedCategoryVector(cat+1);

   // If I specified to listen to all OR
   // If it has category of ALL
   if((mAllowedCategories[vprDBG_ALL]) || (cat == vprDBG_ALL))
      return true;
   else
      return mAllowedCategories[cat];
}

void Debug::setDefaultCategoryNames()
{
   ///* XXX: Removed for insure checking
   addCategoryName(vprDBG_ALLstr,vprDBG_ALL);
   addCategoryName(vprDBG_ERRORstr,vprDBG_ERROR);
   addCategoryName(vprDBG_KERNELstr,vprDBG_KERNEL);
   addCategoryName(vprDBG_INPUT_MGRstr,vprDBG_INPUT_MGR);
   addCategoryName(vprDBG_DRAW_MGRstr,vprDBG_DRAW_MGR);
   addCategoryName(vprDBG_DISP_MGRstr,vprDBG_DISP_MGR);
   addCategoryName(vprDBG_ENV_MGRstr, vprDBG_ENV_MGR);
   addCategoryName(vprDBG_PERFORMANCEstr, vprDBG_PERFORMANCE);
   addCategoryName(vprDBG_CONFIGstr, vprDBG_CONFIG);
   //*/
}

void Debug::getAllowedCatsFromEnv()
{
   ///* XXX: For insure
   char* dbg_cats_env = getenv("VPR_DEBUG_CATEGORIES");

   if(dbg_cats_env != NULL)
   {
      std::cout << "vprDEBUG::Found VPR_DEBUG_CATEGORIES: Listing allowed categories. (If blank, then none allowed.\n" << std::flush;
      std::string dbg_cats(dbg_cats_env);

      std::map< std::string, int >::iterator i;
      for(i=mCategoryNames.begin();i != mCategoryNames.end();i++)
      {
         std::string cat_name = (*i).first;
         if (dbg_cats.find(cat_name) != std::string::npos )    // Found one
         {
            std::cout << "vprDEBUG::getAllowedCatsFromEnv: Allowing: "
                      << (*i).first.c_str() << " val:" << (*i).second
                      << std::endl << std::flush;
            addAllowedCategory((*i).second);                   // Add the category
         }
      }
   }
   else
   {
      std::cout << "vprDEBUG::VPR_DEBUG_CATEGORIES not found:\n"
                << " Setting to: vprDBG_ALL!" << std::endl << std::flush;
      addAllowedCategory(vprDBG_ALL);
   }
   //*/
   //addAllowedCategory(vprDBG_ALL);
}

void Debug::growAllowedCategoryVector(int newSize)
{
   while((int)mAllowedCategories.size() < newSize)
      mAllowedCategories.push_back(false);
}

}; // End of vpr namespace
