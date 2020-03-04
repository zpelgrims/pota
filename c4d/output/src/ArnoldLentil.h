#pragma once

#include "c4dtoa_api.h"
#include "c4dtoa_symbols.h"

#include "c4d.h"

// unique id obtained from http://www.plugincafe.com/forum/developer.asp 
#define LENTIL_ID {id}

class ArnoldLentil : public ArnoldObjectData
{
public:

   ///
   /// Constructor.
   ///
   static NodeData* Alloc()
   {
      return NewObjClear(ArnoldLentil);
   }

   ///
   /// C4D node initialization function.
   ///
   virtual Bool Init(GeListNode* node);

   ///
   /// Defines related Arnold node entry.
   ///
   virtual const String GetAiNodeEntryName(GeListNode* node);

   ///
   /// Event handler.
   ///
   virtual Bool Message(GeListNode* node, Int32 type, void* data);
};

static Bool RegisterArnoldLentil()
{
   return RegisterObjectPlugin(LENTIL_ID, "ArnoldLentil", 0, ArnoldLentil::Alloc, "ainode_lentil", 0, 0);
}

