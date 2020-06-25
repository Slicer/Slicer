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

int vtkMRMLLayoutLogicTest2(int , char * [] )
{
  vtkNew<vtkMRMLLayoutLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkMRMLScene> scene;

  // populate scene with nodes
  vtkNew<vtkMRMLViewNode> viewNode;
  viewNode->SetLayoutName("1");
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkMRMLSliceNode> redSliceNode;
  redSliceNode->SetLayoutName("Red");
  scene->AddNode(redSliceNode.GetPointer());

  vtkNew<vtkMRMLSliceNode> yellowSliceNode;
  yellowSliceNode->SetLayoutName("Yellow");
  scene->AddNode(yellowSliceNode.GetPointer());

  vtkNew<vtkMRMLSliceNode> greenSliceNode;
  greenSliceNode->SetLayoutName("Green");
  scene->AddNode(greenSliceNode.GetPointer());

  // populate scene with layout
  vtkNew<vtkMRMLLayoutNode> layout;
  scene->AddNode(layout.GetPointer());

  logic->SetMRMLScene(scene.GetPointer());
  logic->SetMRMLScene(nullptr);
  logic->SetMRMLScene(scene.GetPointer());

  // ConventionalView
  layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != viewNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(1) != redSliceNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(2) != greenSliceNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(3) != yellowSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong nodes returned: "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(0))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(1))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(2))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(3))->GetID() << " "
              << std::endl << "  instead of :"
              << viewNode->GetID() << " " << redSliceNode->GetID() << " "
              << greenSliceNode->GetID() << " " << yellowSliceNode->GetID() << " "
              << "Pointers: "
              << logic->GetViewNodes()->GetItemAsObject(0) << " "
              << logic->GetViewNodes()->GetItemAsObject(1) << " "
              << logic->GetViewNodes()->GetItemAsObject(2) << " "
              << logic->GetViewNodes()->GetItemAsObject(3) << " "
              << std::endl << "  instead of :"
              << viewNode.GetPointer() << " " << redSliceNode.GetPointer() << " "
              << yellowSliceNode.GetPointer() << " " << greenSliceNode.GetPointer() << " "
              << std::endl;
    return EXIT_FAILURE;
    }

  // 3D layout
  layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != viewNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }

  // RED layout
  layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != redSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  // YELLOW layout
  layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ <<  " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != yellowSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  // GREEN layout
  layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (logic->GetViewNodes()->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " Wrong number of views returned:"
              << logic->GetViewNodes()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
    }
  if (logic->GetViewNodes()->GetItemAsObject(0) != greenSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong node returned:"
              << logic->GetViewNodes()->GetItemAsObject(0) << std::endl;
    return EXIT_FAILURE;
    }
  logic->Print(std::cout);

  return EXIT_SUCCESS;
}

