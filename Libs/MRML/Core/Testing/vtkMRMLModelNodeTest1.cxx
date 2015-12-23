/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkDataSetAttributes.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>

//---------------------------------------------------------------------------
int ExerciseBasicMethods();
int TestActiveScalars();

//---------------------------------------------------------------------------
int vtkMRMLModelNodeTest1(int , char * [] )
{
  CHECK_EXIT_SUCCESS(ExerciseBasicMethods());
  CHECK_EXIT_SUCCESS(TestActiveScalars());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int ExerciseBasicMethods()
{
  vtkNew<vtkMRMLModelNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestActiveScalars()
{
  vtkNew<vtkMRMLModelNode> node1;

  vtkNew<vtkSphereSource> source;
  node1->SetPolyDataConnection(source->GetOutputPort());

  vtkNew<vtkIntArray> testingArray;
  testingArray->SetName("testingArray");
  node1->AddPointScalars(testingArray.GetPointer());

  vtkNew<vtkIntArray> testingArray2;
  testingArray2->SetName("testingArray2");
  node1->AddCellScalars(testingArray2.GetPointer());

  int attribute = vtkDataSetAttributes::SCALARS;
  node1->SetActivePointScalars("testingArray", attribute);
  node1->SetActiveCellScalars("testingArray2", attribute);

  const char *name = node1->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  std::cout << "Active point scalars name = " << (name  == NULL ? "null" : name) << std::endl;
  name = node1->GetActiveCellScalarName(vtkDataSetAttributes::SCALARS);
  std::cout << "Active cell scalars name = " << (name == NULL ? "null" : name) << std::endl;
  node1->RemoveScalars("testingArray");

  return EXIT_SUCCESS;
}
