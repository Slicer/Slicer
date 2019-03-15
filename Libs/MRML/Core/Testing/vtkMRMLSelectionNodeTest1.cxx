/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLSelectionNode.h"

// ---------------------------------------------------------------------------
int TestUnit(vtkMRMLSelectionNode* node1);

// ---------------------------------------------------------------------------
int vtkMRMLSelectionNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLSelectionNode> node1;

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING( node1.GetPointer(), ActiveVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), SecondaryVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveLabelVolumeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveFiducialListID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActivePlaceNodeID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActivePlaceNodeClassName);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveROIListID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveCameraID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveTableID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveViewID);
  TEST_SET_GET_STRING( node1.GetPointer(), ActiveLayoutID);

  // annotations
  node1->AddNewPlaceNodeClassNameToList(nullptr, nullptr);
  node1->AddNewPlaceNodeClassNameToList("invalid string", nullptr);
  node1->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationFiducialNode", nullptr);
  node1->AddNewPlaceNodeClassNameToList(nullptr, ":/Icons/AnnotationROI.png");
  node1->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationROINode", ":/Icons/AnnotationROI.png");
  node1->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationFiducialNode", ":/Icons/AnnotationPoint.png");

  std::string className;
  std::cout << "Checking for className '" << className.c_str() << "' in list, got index: " << node1->PlaceNodeClassNameInList(className) << std::endl;
  className = std::string("vtkMRMLAnnotationFiducialNode");
  int index = node1->PlaceNodeClassNameInList(className);
  std::cout << "Checking for className '" << className.c_str() << "' in list, got index: " << index << std::endl;
  if (index != -1)
    {
    std::string classNamestring = node1->GetPlaceNodeClassNameByIndex(index);
    if (classNamestring.compare(className) != 0)
      {
      std::cerr << "Error! Set className '" << className.c_str()
                << "' to list at index " << index << ", but got back '"
                << classNamestring.c_str() << "'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::string resource = node1->GetPlaceNodeResourceByIndex(index);
    if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
      {
      std::cerr << "ERROR! Got resource for index " << index << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::cout << "Got resource for index " << index << ": " << resource.c_str() << std::endl;
    }
  std::string resource = node1->GetPlaceNodeResourceByClassName(className);
  if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
    {
    std::cerr << "ERROR! Got resource for className " << className << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
    node1->Print(std::cout);
    return EXIT_FAILURE;
    }

  CHECK_EXIT_SUCCESS(TestUnit(node1.GetPointer()));

  return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------
int TestUnit(vtkMRMLSelectionNode* node1 )
{
  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  node1->AddObserver(vtkMRMLSelectionNode::UnitModifiedEvent, callback.GetPointer());

  const char* quantity = "mass";
  const char* unit = "vtkMRMLUnitNodeKilogram";
  node1->SetUnitNodeID(quantity, unit);
  CHECK_STRING(node1->GetUnitNodeID(quantity), unit);

  node1->SetUnitNodeID(quantity, "");
  CHECK_NULL(node1->GetUnitNodeID(quantity));


  node1->SetUnitNodeID(quantity, nullptr);
  CHECK_NULL(node1->GetUnitNodeID(quantity));

  node1->SetUnitNodeID("", unit);
  CHECK_STRING(node1->GetUnitNodeID(""), unit);

  node1->SetUnitNodeID(nullptr, unit);
  CHECK_STRING(node1->GetUnitNodeID(nullptr), unit);

  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLSelectionNode::UnitModifiedEvent), 3);

  return EXIT_SUCCESS;
}
