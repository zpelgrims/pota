#include "ArnoldLentilThinlens.h"

#include "ainode_lentil_thinlens.h"

Bool ArnoldLentilThinlens::Init(GeListNode* node)
{
   return ArnoldObjectData::Init(node);
}

const String ArnoldLentilThinlens::GetAiNodeEntryName(GeListNode* node)
{
   return String("lentil_thinlens");
}

Bool ArnoldLentilThinlens::Message(GeListNode* node, Int32 type, void* data)
{
   return ArnoldObjectData::Message(node, type, data);
}

