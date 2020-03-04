#include "ArnoldLentilThinlensTranslator.h"

#include "ainode_lentil_thinlens.h"

ArnoldLentilThinlensTranslator::ArnoldLentilThinlensTranslator(BaseList2D* node, RenderContext* context) : AbstractTranslator(LENTIL_THINLENS_TRANSLATOR, node, context)
{
}

char* ArnoldLentilThinlensTranslator::GetAiNodeEntryName()
{
   return "lentil_thinlens";
}

void ArnoldLentilThinlensTranslator::InitSteps(int nsteps)
{
   // init all node array parameters
   AbstractTranslator::InitSteps(nsteps);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;
}

void ArnoldLentilThinlensTranslator::Export(int step)
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

