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
  QdecFactor* qf =
    new QdecFactor( "Age",
                    QdecFactor::qdecContinuousFactorType,
                    33.0);
  iFactors.push_back( qf );
  QdecFactor* qfGender =
    new QdecFactor( "Gender",
                    QdecFactor::qdecDiscreteFactorType,
                    "Female");
  iFactors.push_back( qfGender );

  QdecSubject  qdec( isId, iFactors );


  std::string id = qdec.GetId();

  if (id.compare(isId) != 0)
    {
    std::cerr << "Got incorrect id, " << id.c_str() << " != " << isId.c_str() << "\n";
    return EXIT_FAILURE;
    }

  std::string gender = qdec.GetDiscreteFactor("Gender");
  if (gender.compare("Female") != 0)
    {
    std::cerr << "Got incorrect gender, " << gender.c_str() << " != Female\n";
    return EXIT_FAILURE;
    }

  double age = qdec.GetContinuousFactor("Age");
  if (age != 33.0)
    {
    std::cerr << "Got incorrect age, " << age << " != 33\n";
    return EXIT_FAILURE;
    }

  std::vector < QdecFactor* > contFactors = qdec.GetContinuousFactors();
  std::vector < QdecFactor* > factors = qdec.GetFactors();
  
  return EXIT_SUCCESS;
}
