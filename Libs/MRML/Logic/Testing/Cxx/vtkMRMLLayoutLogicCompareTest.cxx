// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// STD includes

bool TestSetSlicerLayoutCompareGridView();
bool TestSetSlicerLayoutCompareGridViewEvents();

//----------------------------------------------------------------------------
int vtkMRMLLayoutLogicCompareTest(int , char * [] )
{
  bool res = true;
  res = TestSetSlicerLayoutCompareGridView() && res;
  res = TestSetSlicerLayoutCompareGridViewEvents() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestSetSlicerLayoutCompareGridView()
{
  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

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
    return false;
    }

  layoutNode->SetNumberOfCompareViewRows(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 4)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return false;
    }

  layoutNode->SetNumberOfCompareViewColumns(2);
  if (layoutLogic->GetViewNodes()->GetNumberOfItems() != 6)
    {
    std::cout << __LINE__ << ": SetNumberOfCompareViewRows(Grid) failed. "
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << " views."
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestSetSlicerLayoutCompareGridViewEvents()
{
  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkMRMLLayoutLogic> layoutLogic;
  layoutLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> spy;
  layoutNode->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutCompareGridView);
  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewRows(2);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();


  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewColumns(2);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  if (spy->GetTotalNumberOfEvents() != 1 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 1)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();

  // Fires 2 modified events:
  //  once when changing the compareviewrows,
  //  once when updating the layout description by the logic
  // Ideally, it should be 1 event.
  layoutNode->SetNumberOfCompareViewRows(3);
  if (spy->GetTotalNumberOfEvents() != 2 ||
      spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != 2)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}
