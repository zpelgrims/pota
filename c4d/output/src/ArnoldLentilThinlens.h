#pragma once

#include "c4dtoa_api.h"
#include "c4dtoa_symbols.h"

#include "c4d.h"

// unique id obtained from http://www.plugincafe.com/forum/developer.asp 
#define LENTIL_THINLENS_ID {id}

class ArnoldLentilThinlens : public ArnoldObjectData
{
public:

   ///
   /// Constructor.
   ///
   static NodeData* Alloc()
   {
      return NewObjClear(ArnoldLentilThinlens);
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

static Bool RegisterArnoldLentilThinlens()
{
   return RegisterObjectPlugin(LENTIL_THINLENS_ID, "ArnoldLentilThinlens", 0, ArnoldLentilThinlens::Alloc, "ainode_lentil_thinlens", 0, 0);
}

