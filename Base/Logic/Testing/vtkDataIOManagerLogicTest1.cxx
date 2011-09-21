/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkDataIOManagerLogic.h"


#include "TestingMacros.h"

int vtkDataIOManagerLogicTest1(int , char * [] )
{
  vtkSmartPointer< vtkDataIOManagerLogic > node1 = vtkSmartPointer< vtkDataIOManagerLogic >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  vtkSmartPointer< vtkDataIOManagerLogic > node2 = vtkSmartPointer< vtkDataIOManagerLogic >::New();


  vtkSmartPointer<vtkDataIOManager> dataIOManager = vtkSmartPointer<vtkDataIOManager>::New();
  node1->SetAndObserveDataIOManager(dataIOManager);

// Common to all logic's
  vtkSmartPointer<vtkSlicerApplicationLogic> slicerApplictionLogic =
    vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  node1->SetApplicationLogic(slicerApplictionLogic);
  node1->GetApplicationLogic()->Print(std::cout);
  TEST_SET_GET_STRING(node1, ModuleName);
  TEST_SET_GET_STRING(node1, ModuleLocation);
  // ERROR because SLICER_HOME is only defined in qSlicerCoreApplication()
  std::cout << "ModuleShareDirectory: "
            << (node1->GetModuleShareDirectory() ?
                node1->GetModuleShareDirectory() : "ERROR") << std::endl;
  std::cout << "ModuleLibDirectory: "
            << (node1->GetModuleLibDirectory() ?
                node1->GetModuleLibDirectory() : "ERROR") << std::endl;
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  return EXIT_SUCCESS;
}

