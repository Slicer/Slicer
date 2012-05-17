// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// STD includes

bool TestSetSlicerLayoutCompareGridView();

//----------------------------------------------------------------------------
int vtkMRMLLayoutLogicCompareTest(int , char * [] )
{
  bool res = true;
  res = TestSetSlicerLayoutCompareGridView() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestSetSlicerLayoutCompareGridView()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLayoutLogic> layoutLogic;
  layoutLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutCompareGridView);
  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareGridView)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed." << std::endl;
    return false;
    }

  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 3)
    {
    std::cout << __LINE__ << ": SetViewArrangement(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetNumberOfCompareViewRows(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 4)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetNumberOfCompareViewColumns(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 6)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return EXIT_FAILURE;
    }
}
