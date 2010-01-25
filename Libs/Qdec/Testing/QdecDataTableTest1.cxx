/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecDataTable.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecDataTableTest1(int argc, char * argv[] )
{
  QdecDataTable  dataTable;

  std::string fileName;
  if (argc == 2)
    {
    fileName = argv[1];
    std::cout << "Using filename " << fileName.c_str() << std::endl;
    }
  else
    {
    fileName = "qdec.table.dat";
    }
  char dirName[128];
  int retval = dataTable.Load(fileName.c_str(), dirName);

  if (retval == -1)
    {
    std::cerr << "Error loading table file " << fileName  << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "dirName = '" << dirName << "'\n";
    }

  std::cout << "File name = " << (dataTable.GetFileName()).c_str() << "\n";

  std::vector<std::string> subjectIDs = dataTable.GetSubjectIDs();

  std::vector< QdecSubject* > subjects =  dataTable.GetSubjects();

  QdecFactor *factor = dataTable.GetFactor("Age");
  if (factor == NULL)
    {
    std::cerr << "Null factor Age\n";
    return EXIT_FAILURE;
    }
  std::vector<double> meanAndDev = dataTable.GetMeanAndStdDev("Age");
  std::cout << "Mean = " << meanAndDev[0] << ", standard deviation = " << meanAndDev[1] << std::endl;
                                                  

  std::vector< std::string > dFactors = dataTable.GetDiscreteFactors();
  std::vector< std::string > cFactors = dataTable.GetContinuousFactors();

  std::cout << "Number of classes = " << dataTable.GetNumberOfClasses() << std::endl;
  std::cout << "Number of regressors = " << dataTable.GetNumberOfRegressors() << std::endl;


  dataTable.Dump(stdout);
  
  return EXIT_SUCCESS;
}
