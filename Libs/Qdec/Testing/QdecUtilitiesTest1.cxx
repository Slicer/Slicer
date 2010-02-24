/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecUtilities.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecUtilitiesTest1(int , char * [] )
{

  bool isReadable = QdecUtilities::IsFileReadable( "fakeFile.h" );

  if( isReadable )
    {
    std::cerr << "Error in IsFileReadable() " << std::endl;
    return EXIT_FAILURE;
    }

  bool fakeFileIsReadable = true;
  try
    {
    QdecUtilities::AssertFileIsReadable("fakeFile.h");
    }
  catch (...)
    {
    std::cout << "Success in catching that a fake file is not readable.\n";
    fakeFileIsReadable = false;
    }
  if (fakeFileIsReadable)
    {
    std::cerr << "Error in AssertFileIsReadable\n";
    return EXIT_FAILURE;
    }

  const char *fname = "/tmp/testThis.h";
  char pathName[128];

  const char *returnString = QdecUtilities::FileNamePath(fname, pathName);

  std::cout << "Calling FileNamePath with fname = " << fname << ", pathName = " << pathName << " and return string = " << returnString << std::endl;

  const char *fname2 = "testThis.h";
  const char *returnString2 = QdecUtilities::FileNamePath(fname2, pathName);
  std::cout << "Calling FileNamePath with fname = " << fname2 << ", pathName = " << pathName << " and return string = " << returnString2 << std::endl;

  return EXIT_SUCCESS;
}
