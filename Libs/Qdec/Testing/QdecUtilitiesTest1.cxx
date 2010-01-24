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
  QdecUtilities  qdec;

  bool isReadable = qdec.IsFileReadable( "fakeFile.h" );

  if( isReadable )
    {
    std::cerr << "Error in IsFileReadable() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
