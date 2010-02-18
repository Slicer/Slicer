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
  QdecGlmFit  iGlmFit;

//  QdecDataTable* iDataTable = new QdecDataTable;

//  QdecGlmDesign iGlmDesign(iDataTable);

//  iGlmFit.CreateResultsFromCachedData(&iGlmDesign);
  
  QdecGlmFitResults* results = iGlmFit.GetResults();

  // can't test this without files on disk, so results should be null
  if (results != NULL)
    {
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
