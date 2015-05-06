
// MRML includes
#include <vtkMRMLLayoutNode.h>

namespace
{

// --------------------------------------------------------------------------
bool checkViewArrangement(int line, qMRMLLayoutManager* layoutManager,
                          vtkMRMLLayoutNode * layoutNode, int expectedViewArrangement)
{
  // Ignore deprecated arrangments
  if (expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutSingleLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayout3DPlusLightboxView)
    {
    return true;
    }

  if (layoutManager->layout() != expectedViewArrangement ||
      layoutNode->GetViewArrangement() != expectedViewArrangement)
    {
    std::cerr << "Line " << line << " - Add scene failed:\n"
              << " expected ViewArrangement: " << expectedViewArrangement << "\n"
              << " current ViewArrangement: " << layoutNode->GetViewArrangement() << "\n"
              << " current layout: " << layoutManager->layout() << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace
