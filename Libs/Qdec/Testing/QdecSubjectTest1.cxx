/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecSubject.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecSubjectTest1(int , char * [] )
{

  std::string isId = "thisIsAnId";

  std::vector < QdecFactor* > iFactors;

  QdecSubject  qdec( isId, iFactors );


  return EXIT_SUCCESS;
}
