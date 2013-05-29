/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLSelectionNode.h"


#include "vtkMRMLCoreTestingMacros.h"

// ---------------------------------------------------------------------------
bool TestUnit(vtkMRMLSelectionNode* node1);

// ---------------------------------------------------------------------------
int vtkMRMLSelectionNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLSelectionNode > node1 = vtkSmartPointer< vtkMRMLSelectionNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLSelectionNode, node1);

  TEST_SET_GET_STRING(node1, ActiveVolumeID);
  TEST_SET_GET_STRING(node1, SecondaryVolumeID);
  TEST_SET_GET_STRING(node1, ActiveLabelVolumeID);
  TEST_SET_GET_STRING(node1, ActiveFiducialListID);
  TEST_SET_GET_STRING(node1, ActiveAnnotationID);
  TEST_SET_GET_STRING(node1, ActiveROIListID);
  TEST_SET_GET_STRING(node1, ActiveCameraID);
  TEST_SET_GET_STRING(node1, ActiveViewID);
  TEST_SET_GET_STRING(node1, ActiveLayoutID);


  node1->AddNewAnnotationIDToList(NULL, NULL);
  node1->AddNewAnnotationIDToList("invalid string", NULL);
  node1->AddNewAnnotationIDToList("vtkMRMLAnnotationFiducialNode", NULL);
  node1->AddNewAnnotationIDToList(NULL, ":/Icons/AnnotationROI.png");
  node1->AddNewAnnotationIDToList("vtkMRMLAnnotationROINode", ":/Icons/AnnotationROI.png");
  node1->AddNewAnnotationIDToList("vtkMRMLAnnotationFiducialNode", ":/Icons/AnnotationPoint.png");

  std::string id;
  std::cout << "Checking for id '" << id.c_str() << "' in list, got index: " << node1->AnnotationIDInList(id) << std::endl;
  id = std::string("vtkMRMLAnnotationFiducialNode");
  int index = node1->AnnotationIDInList(id);
  std::cout << "Checking for id '" << id.c_str() << "' in list, got index: " << index << std::endl;
  if (index != -1)
    {
    std::string idstring = node1->GetAnnotationIDByIndex(index);
    if (idstring.compare(id) != 0)
      {
      std::cerr << "Error! Set id '" << id.c_str() << "' to list at index " << index << ", but got back '" << idstring.c_str() << "'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::string resource = node1->GetAnnotationResourceByIndex(index);
    if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
      {
      std::cerr << "ERROR! Got resource for index " << index << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
      node1->Print(std::cout);
      return EXIT_FAILURE;
      }
    std::cout << "Got resource for index " << index << ": " << resource.c_str() << std::endl;
    }
  std::string resource = node1->GetAnnotationResourceByID(id);
  if (resource.compare(":/Icons/AnnotationPoint.png") != 0)
    {
    std::cerr << "ERROR! Got resource for id " << id << ": '" << resource.c_str() << "', but expected ':/Icons/AnnotationPoint.png'" << std::endl;
    node1->Print(std::cout);
    return EXIT_FAILURE;
    }

  if (TestUnit(node1) != EXIT_SUCCESS)
    {
    return EXIT_FAILURE;
    }

  node1->Print(std::cout);
  return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------
bool TestUnit(vtkMRMLSelectionNode* node1)
{
  vtkSmartPointer<vtkMRMLNodeCallback> callback =
    vtkSmartPointer<vtkMRMLNodeCallback>::New();
  node1->AddObserver(vtkMRMLSelectionNode::UnitModifiedEvent, callback);

  const char* quantity = "mass";
  const char* unit = "vtkMRMLUnitNodeKilogram";
  node1->SetUnitNodeID(quantity, unit);
  if (strcmp(node1->GetUnitNodeID(quantity), unit) != 0)
    {
    std::cerr<<"Expecting: "<<unit<<" got "
      <<node1->GetUnitNodeID(quantity)<<std::endl;
    return EXIT_FAILURE;
    }

  node1->SetUnitNodeID(quantity, "");
  if (node1->GetUnitNodeID(quantity) != 0)
    {
    std::cerr<<"Expecting: "<<0<<" got "
      <<node1->GetUnitNodeID(quantity)<<std::endl;
    return EXIT_FAILURE;
    }

  node1->SetUnitNodeID(quantity, 0);
  if (node1->GetUnitNodeID(quantity) != 0)
    {
    std::cerr<<"Expecting: "<<0<<" got "
      <<node1->GetUnitNodeID(quantity)<<std::endl;
    return EXIT_FAILURE;
    }

  node1->SetUnitNodeID("", unit);
  if (strcmp(node1->GetUnitNodeID(""), unit) != 0)
    {
    std::cerr<<"Expecting: "<<unit<<" got "
      <<node1->GetUnitNodeID("")<<std::endl;
    return EXIT_FAILURE;
    }

  node1->SetUnitNodeID(0, unit);
  if (strcmp(node1->GetUnitNodeID(0), unit) != 0)
    {
    std::cerr<<"Expecting: "<<unit<<" got "
      <<node1->GetUnitNodeID(0)<<std::endl;
    return EXIT_FAILURE;
    }

  if (callback->GetNumberOfEvents(vtkMRMLSelectionNode::UnitModifiedEvent) != 3)
    {
    std::cerr<<"Expecting: "<<3<<" callbacks got "
      <<callback->GetNumberOfEvents(vtkMRMLSelectionNode::UnitModifiedEvent)
      <<std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
