/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "QdecContrast.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

 
int QdecContrastTest1(int , char * [] )
{
  vector< double > iaVector;
  iaVector.push_back( 1.6 );
  iaVector.push_back( 2.6 );
  iaVector.push_back( 3.6 );
  iaVector.push_back( 4.6 );

  std::string isName = "nameOfNoName";
  std::string isQuestion = "DoWeExist?";

  QdecContrast  qdec( iaVector, isName, isQuestion );

  std::cout << "Name = " << qdec.GetName() << std::endl;
  std::cout << "Question = " << qdec.GetQuestion() << std::endl;
  std::cout << "Contrast = " << qdec.GetContrastStr() << std::endl;
  std::cout << "DotMatFileName = " << qdec.GetDotMatFileName() << std::endl;

  return EXIT_SUCCESS;
}
