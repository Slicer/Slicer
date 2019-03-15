/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLInteractionNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLInteractionNodeTest1(int , char * [] )
{
  vtkNew< vtkMRMLInteractionNode > node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_INT( node1, CurrentInteractionMode, vtkMRMLInteractionNode::Place);
  TEST_SET_GET_INT( node1, CurrentInteractionMode, vtkMRMLInteractionNode::ViewTransform);
  // test re-setting with same value
  TEST_SET_GET_INT( node1, CurrentInteractionMode, vtkMRMLInteractionNode::ViewTransform);

  TEST_SET_GET_INT( node1, LastInteractionMode, vtkMRMLInteractionNode::Place);
  TEST_SET_GET_INT( node1, LastInteractionMode, vtkMRMLInteractionNode::ViewTransform);
  // test re-setting with same value
  TEST_SET_GET_INT( node1, LastInteractionMode, vtkMRMLInteractionNode::ViewTransform);

  TEST_SET_GET_INT_RANGE( node1, PlaceModePersistence, 0, 1);
  TEST_SET_GET_INT_RANGE( node1, TransformModePersistence, 0, 1);

  node1->NormalizeAllMouseModes();

  const char *modeStr = node1->GetInteractionModeAsString();
  std::cout << "Interaction mode = " << (modeStr ? modeStr : "null") << std::endl;
  for (int m = 0; m < 9; m++)
    {
    modeStr = node1->GetInteractionModeAsString(m);
    std::cout << "Interaction mode " << m << " = " << (modeStr ? modeStr : "null") << std::endl;
    }
  int mode = node1->GetInteractionModeByString(nullptr);
  std::cout << "For null string, interaction mode = " << mode << std::endl;
  mode = node1->GetInteractionModeByString("invalid");
  std::cout << "For 'invalid' string, interaction mode = " << mode << std::endl;

  node1->SwitchToPersistentPlaceMode();
  if (!node1->GetPlaceModePersistence())
    {
    std::cerr << "Error in SwitchToPersistentPlaceMode, got persistence = " << node1->GetPlaceModePersistence() << std::endl;
    return EXIT_FAILURE;
    }
  node1->SwitchToSinglePlaceMode();
  if (node1->GetPlaceModePersistence())
    {
    std::cerr << "Error in SwitchToSinglePlaceMode, got persistence = " << node1->GetPlaceModePersistence() << std::endl;
    return EXIT_FAILURE;
    }
  node1->SwitchToViewTransformMode();
  if (!node1->GetTransformModePersistence())
    {
    std::cerr << "Error in SwitchToViewTransformMode, got persistence = " << node1->GetTransformModePersistence() << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
