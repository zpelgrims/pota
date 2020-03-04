#include "ArnoldLentilTranslator.h"

#include "ainode_lentil.h"

ArnoldLentilTranslator::ArnoldLentilTranslator(BaseList2D* node, RenderContext* context) : AbstractTranslator(LENTIL_TRANSLATOR, node, context)
{
}

char* ArnoldLentilTranslator::GetAiNodeEntryName()
{
   return "lentil";
}

void ArnoldLentilTranslator::InitSteps(int nsteps)
{
   // init all node array parameters
   AbstractTranslator::InitSteps(nsteps);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;
}

void ArnoldLentilTranslator::Export(int step)
{
   // exports all node parameters
   AbstractTranslator::Export(step);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;

   // first motion step
   if (step == 0)
   {
   }
}

