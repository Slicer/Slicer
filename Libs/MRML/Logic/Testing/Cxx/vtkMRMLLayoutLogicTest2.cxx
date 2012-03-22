// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLLayoutLogicTest2(int , char * [] )
{
  vtkSmartPointer< vtkMRMLLayoutLogic > logic = vtkSmartPointer<vtkMRMLLayoutLogic>::New();
  EXERCISE_BASIC_OBJECT_METHODS( logic );

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  // populate scene with nodes
  vtkSmartPointer<vtkMRMLViewNode> viewNode = vtkSmartPointer<vtkMRMLViewNode>::New();
  scene->AddNode(viewNode);
  vtkSmartPointer<vtkMRMLSliceNode> redSliceNode = vtkSmartPointer<vtkMRMLSliceNode>::New();
  redSliceNode->SetSingletonTag("Red");
  scene->AddNode(redSliceNode);
  vtkSmartPointer<vtkMRMLSliceNode> yellowSliceNode = vtkSmartPointer<vtkMRMLSliceNode>::New();
  yellowSliceNode->SetSingletonTag("Yellow");
  scene->AddNode(yellowSliceNode);
  vtkSmartPointer<vtkMRMLSliceNode> greenSliceNode = vtkSmartPointer<vtkMRMLSliceNode>::New();
  greenSliceNode->SetSingletonTag("Green");
  scene->AddNode(greenSliceNode);
  // populate scene with layout
  vtkSmartPointer<vtkMRMLLayoutNode> layout = vtkSmartPointer<vtkMRMLLayoutNode>::New();
  scene->AddNode(layout);

  logic->SetMRMLScene(scene);
  logic->SetMRMLScene(0);
  logic->SetMRMLScene(scene);

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
      logic->GetViewNodes()->GetItemAsObject(2) != yellowSliceNode.GetPointer() ||
      logic->GetViewNodes()->GetItemAsObject(3) != greenSliceNode.GetPointer())
    {
    std::cerr << __LINE__ << " Wrong nodes returned:"
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(0))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(1))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(2))->GetID() << " "
              << vtkMRMLNode::SafeDownCast(logic->GetViewNodes()->GetItemAsObject(3))->GetID()
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

