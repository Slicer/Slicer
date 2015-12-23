/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// SlicerLogic includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkDataIOManagerLogic.h"

int vtkDataIOManagerLogicTest1(int , char * [] )
{
  vtkNew<vtkDataIOManagerLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkDataIOManager> dataIOManager;
  logic->SetAndObserveDataIOManager(dataIOManager.GetPointer());

  vtkNew<vtkSlicerApplicationLogic> slicerApplicationLogic;
  logic->SetMRMLApplicationLogic(slicerApplicationLogic.GetPointer());
  logic->GetApplicationLogic()->Print(std::cout);

  return EXIT_SUCCESS;
}

