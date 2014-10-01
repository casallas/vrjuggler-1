/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 ****************** <VPR heading END do not edit this line> ******************/

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

#ifndef _VPR_BUFFER_OBJECT_WRITER_H
#define _VPR_BUFFER_OBJECT_WRITER_H

#include <vpr/vprConfig.h>
#include <vector>
#include <boost/static_assert.hpp>
#include <boost/concept_check.hpp>

#include <vpr/System.h>
#include <vpr/IO/ObjectWriter.h>

namespace vpr
{

/** \class BufferObjectWriter BufferObjectWriter.h vpr/IO/BufferObjectWriter.h
 *
 * Object writer for data buffers.  Writes directly to a data buffer.
 *
 * @todo Add smart buffering for type sizes.
 */
class VPR_API BufferObjectWriter : public ObjectWriter
{
public:
   /**
    * Number of bytes used to store the size of the string.
    *
    * @since 1.1.15
    */
   static const unsigned int STRING_LENGTH_SIZE;

   /** Constructor.
    * Build writer and point it at a fresh data buffer.
    * This data buffer will be owned exclusively by this object
    * and will be deleted on destruction.
    */
   BufferObjectWriter();

   /** Constructor.
    * Build writer and point it at an existing data buffer.
    * This buffer will not be owned or destroyed by the writer.
    */
   BufferObjectWriter(std::vector<vpr::Uint8>* data,
                      const unsigned int curPos = 0);

   virtual ~BufferObjectWriter();

   void setCurPos(unsigned int val)
   {
      mCurHeadPos = val;
   }

   unsigned int getCurPos()
   {
      return mCurHeadPos;
   }

   std::vector<vpr::Uint8>* getData()
   {
      return mData;
   }

   /** @name Tag and attribute handling */
   //@{
   /**
    * Starts a new section/element of name \p tagName.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void beginTag(const std::string& tagName)
   {
      boost::ignore_unused_variable_warning(tagName);
   }

   /**
    * Ends the most recently named tag.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void endTag()
   {
      /* Do nothing. */ ;
   }

   /**
    * Starts an attribute of the name \p attributeName.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void beginAttribute(const std::string& attributeName)
   {
      boost::ignore_unused_variable_warning(attributeName);
   }

   /**
    * Ends the most recently named attribute.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void endAttribute()
   {
      /* Do nothing. */ ;
   }
   //@}

   /**
    * Writes out the single byte.
    *
    * @post data = old(data)+val, \c mCurHeadPos advanced 1.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeUint8(vpr::Uint8 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeUint16(vpr::Uint16 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeUint32(vpr::Uint32 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeUint64(vpr::Uint64 val);

   /**
    * Writes out the single byte.
    *
    * @post data = old(data)+val, \c mCurHeadPos advanced 1.
    *
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeInt8(vpr::Int8 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeInt16(vpr::Int16 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeInt32(vpr::Int32 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeInt64(vpr::Int64 val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeFloat(float val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeDouble(double val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeString(std::string val);

   /**
    * @throw IOException Thrown if I/O errors occur while writing to the
    *                    underlying data source.
    */
   virtual void writeBool(bool val);

   /**
    * Writes raw data of length \p len.
    *
    * @throw vpr::IOException Thrown if the operation failed.
    */
   inline void writeRaw(vpr::Uint8* data,
                        const unsigned int len = 1);

public:
   /** If true we allocated the data buffer and should delete it. */
   bool                       mOwnDataBuffer;
   std::vector<vpr::Uint8>*   mData;
   unsigned int               mCurHeadPos;
};

inline void BufferObjectWriter::writeRaw(vpr::Uint8* data,
                                         const unsigned int len)
{
   for ( unsigned int i = 0; i < len; ++i )
   {
      mData->push_back(data[i]);
   }

   mCurHeadPos += len;
}

} // namespace vpr

#endif
