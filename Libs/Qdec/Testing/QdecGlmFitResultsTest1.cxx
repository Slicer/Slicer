/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecGlmFitResults.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecGlmFitResultsTest1(int , char * [] )
{
  QdecDataTable * iDataTable = new QdecDataTable;

  QdecGlmDesign iGlmDesign( iDataTable );

  vector< string > iContrastSigFiles;

  std::string iConcatContrastSigFile;
  std::string ifnResidualErrorStdDevFile;
  std::string ifnRegressionCoefficientsFile;
  std::string ifnFsgdFile;


  QdecGlmFitResults  qdec( &iGlmDesign, iContrastSigFiles, iConcatContrastSigFile,
   ifnResidualErrorStdDevFile, ifnRegressionCoefficientsFile, ifnFsgdFile );


  delete iDataTable;

  return EXIT_SUCCESS;
}
