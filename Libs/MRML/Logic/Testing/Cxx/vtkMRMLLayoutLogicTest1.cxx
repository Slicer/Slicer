// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLLayoutLogicTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  vtkNew<vtkMRMLLayoutLogic> layoutLogic;
  layoutLogic->SetMRMLScene(scene.GetPointer());
  vtkMRMLLayoutNode* layoutNode = layoutLogic->GetLayoutNode();
  if (!layoutNode)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutNode::SetMRMLScene failed"
              << ", no layout node:" << layoutNode << std::endl;
    return EXIT_FAILURE;
    }
  layoutNode->SetViewArrangement(
    vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  vtkCollection* views = layoutLogic->GetViewNodes();
  if (views->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << layoutLogic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
    views->GetItemAsObject(0));
  vtkMRMLSliceNode* redNode = vtkMRMLSliceNode::SafeDownCast(
    views->GetItemAsObject(1));
  vtkMRMLSliceNode* yellowNode = vtkMRMLSliceNode::SafeDownCast(
    views->GetItemAsObject(2));
  vtkMRMLSliceNode* greenNode = vtkMRMLSliceNode::SafeDownCast(
    views->GetItemAsObject(3));

  if (!viewNode || !redNode || !yellowNode || !greenNode)
    {
    std::cerr << __LINE__ << " Wrong nodes returned:"
              << viewNode << " " << redNode << " "
              << yellowNode << " " << greenNode << std::endl;
    return EXIT_FAILURE;
    }

  layoutLogic->Print(std::cout);

  return EXIT_SUCCESS;
}

