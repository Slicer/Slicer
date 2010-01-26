/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecProject.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecProjectTest1(int , char * [] )
{
  QdecProject  qdec;

  qdec.SetZipCommandFormat("cd %s; zip -r %1 %2");
  qdec.SetZipCommand("zip");
  string cmd = qdec.GetZipCommand();
  if (cmd.compare("zip") != 0)
    {
    std::cerr << "Error getting zip cmd: " << cmd << std::endl;
    return EXIT_FAILURE;
    }
  
  qdec.SetUnzipCommandFormat("unzip -d %3 %1");
  qdec.SetUnzipCommand("unzip");
  cmd = qdec.GetUnzipCommand();
  if (cmd.compare("unzip") != 0)
    {
    std::cerr << "Error getting unzip cmd: " << cmd << std::endl;
    return EXIT_FAILURE;
    }
  
  qdec.SetRmCommand("removeMe");
  cmd = qdec.GetRmCommand();
  if (cmd.compare("removeMe") != 0)
    {
    std::cerr << "Error getting rm cmd: " << cmd << std::endl;
    return EXIT_FAILURE;
    }
  
  int retval = qdec.SetSubjectsDir("/tmp/Subjects");
  if (retval == 1)
    {
    std::cerr << "Error setting subjects dir to /tmp/Subjects" << std::endl;
    return EXIT_FAILURE;
    }
  string subjectsDir = qdec.GetSubjectsDir();
  if (subjectsDir.compare("/tmp/Subjects") != 0)
    {
    std::cerr << "Error getting back subjects dir, got " << subjectsDir << " instead of '/tmp/Subjects'" << std::endl;
    return EXIT_FAILURE;
    }

  qdec.SetAverageSubject("fsaverage");
  string avgSub = qdec.GetAverageSubject();
  if (avgSub.compare("fsaverage") != 0)
    {
    std::cerr << "Error getting back average subject, got " << avgSub << " instead of 'fsaverage'" << std::endl;
    return EXIT_FAILURE;
    }
  
  retval = qdec.SetWorkingDir("/tmp");
  if (retval == 1 ||
      qdec.GetWorkingDir().compare("/tmp") != 0)
    {
    std::cerr << "Error getting working dir /tmp, instead got " << qdec.GetWorkingDir() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Default working dir = " << qdec.GetDefaultWorkingDir() << std::endl;

  std::cout << "MetadataFileName = " << qdec.GetMetadataFileName() << std::endl;
  
  /* can't test this without a subjects directory
  const char* isName = "name";
  const char* isFirstDiscreteFactor = "firstDiscrete";
  const char* isSecondDiscreteFactor = "secondDiscrete";
  const char* isFirstContinuousFactor = "firstCont";
  const char* isSecondContinuousFactor = "secondCont";
  const char* isMeasure = "measure";
  const char* isHemi = "r";
  int iSmoothnessLevel = 1.0;
  retval = qdec.CreateGlmDesign(isName, isFirstDiscreteFactor,
                        isSecondDiscreteFactor,
                        isFirstContinuousFactor,
                        isSecondContinuousFactor,
                        isMeasure,
                        isHemi,
                                iSmoothnessLevel);
  if (retval != 0)
    {
    std::cerr << "Error creating glm design." << std::endl;
    return EXIT_FAILURE;
    }
  */
  
  // qdec.DumpDataTable(stdout);

  QdecDataTable* dataTable = qdec.GetDataTable ( );
  if (dataTable == NULL)
    {
    std::cerr << "Null data table" << std::endl;
    return EXIT_FAILURE;
    }
  
  
  QdecGlmDesign* glmDesign = qdec.GetGlmDesign();
  if (glmDesign == NULL)
    {
    std::cerr << "Null design" << std::endl;
    return EXIT_FAILURE;
    }
  
  vector<string> subjectIDs = qdec.GetSubjectIDs();
  vector<string> discreteFactors = qdec.GetDiscreteFactors();
  vector<string> contFactors = qdec.GetContinousFactors();

  string hemi = qdec.GetHemi();
  std::cout << "Hemisphere = " << hemi << std::endl;
  if (hemi.compare("lh") != 0)
    {
    std::cerr << "Incorrect hemisphere, expecting lh" << std::endl;
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
