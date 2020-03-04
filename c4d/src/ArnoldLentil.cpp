#include "ArnoldLentil.h"

#include "ainode_lentil.h"

Bool ArnoldLentil::Init(GeListNode* node)
{
   return ArnoldObjectData::Init(node);
}

const String ArnoldLentil::GetAiNodeEntryName(GeListNode* node)
{
   return String("lentil");
}

Bool ArnoldLentil::Message(GeListNode* node, Int32 type, void* data)
{
   return ArnoldObjectData::Message(node, type, data);
}

