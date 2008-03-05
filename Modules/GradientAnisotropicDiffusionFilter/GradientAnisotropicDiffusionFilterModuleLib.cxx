#include "GradientAnisotropicDiffusionFilterModuleLib.h"

#include "vtkGradientAnisotropicDiffusionFilterLogic.h"
#include "vtkGradientAnisotropicDiffusionFilterGUI.h"

static char GradientAnisotropicDiffusionFilterDescription[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<loadable>\n"
"  <name>Gradient Anisotropic Module</name>\n"
"  <shortname>GradientAnisotropicFilter</shortname>\n"
"  <guiname>GradientAnisotropicFilter</guiname>\n"
"  <tclinitname>Gradientanisotropicdiffusionfilter_Init</tclinitname>\n"
"  <message>Initializing Gradient Anisotropic Module...</message>\n"
"</loadable>";

char*
GetLoadableModuleDescription()
{
  return GradientAnisotropicDiffusionFilterDescription;
}

void*
GetLoadableModuleGUI()
{
  return vtkGradientAnisotropicDiffusionFilterGUI::New ( );
}


void*
GetLoadableModuleLogic()
{
  return vtkGradientAnisotropicDiffusionFilterLogic::New ( );
}
