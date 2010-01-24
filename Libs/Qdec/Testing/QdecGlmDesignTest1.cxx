/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecGlmDesign.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecGlmDesignTest1(int , char * [] )
{
  QdecDataTable* iDataTable = new QdecDataTable;

  QdecGlmDesign  qdec(iDataTable);

  std::cout << "Name = " << qdec.GetName() << std::endl;

  delete iDataTable;

  return EXIT_SUCCESS;
}
