/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLModelNode.h"

#include <vtkPolyData.h>



#include "vtkMRMLCoreTestingMacros.h"


int vtkMRMLModelNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelNode > node1 = vtkSmartPointer< vtkMRMLModelNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

//  EXERCISE_BASIC_MRML_METHODS(vtkMRMLModelNode, node1);
  
//  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(node1);

//  EXERCISE_BASIC_STORABLE_MRML_METHODS(node1);
  
  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLModelNode, node1);


  vtkSmartPointer <vtkIntArray> darray = vtkSmartPointer <vtkIntArray>::New();
  darray->SetName("testingArray");
  node1->AddPointScalars(darray);
  darray->SetName("testingArray2");
  node1->AddCellScalars(darray);
  node1->SetActiveScalars("testingArray", "Scalars");
  int attribute =  0; // vtkDataSetAttributes::SCALARS;
  node1->SetActivePointScalars("testingArray", attribute);
  node1->SetActiveCellScalars("testingArray2", attribute);
  const char *name = node1->GetActivePointScalarName("scalars");
  std::cout << "Active point scalars name = " << (name  == NULL ? "null" : name) << std::endl;
  name = node1->GetActiveCellScalarName("scalars");
  std::cout << "Active cell scalars name = " << (name == NULL ? "null" : name) << std::endl;
  node1->RemoveScalars("testingArray");

  std::cout << "Model node default write ext = " << node1->GetDefaultWriteFileExtension() << std::endl;

  return EXIT_SUCCESS;
}
