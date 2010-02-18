/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecGlmFit.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecGlmFitTest1(int , char * [] )
{
  QdecGlmFit  qdec;

  //QdecGlmDesign* iGlmDesign = NULL;
  
  QdecGlmFitResults* results = qdec.GetResults();

  if (results == NULL)
    {
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
