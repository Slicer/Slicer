/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecFactor.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecFactorTest1(int , char * [] )
{
  const int iType = 1; // discrete
  QdecFactor qdec( "thisIsAName", iType );

  return EXIT_SUCCESS;
}
