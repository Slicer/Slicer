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
  QdecFactor factorDiscreteWithValue("Gender", iType, "Female");
  std::string discreteValue = factorDiscreteWithValue.GetDiscreteValue();
  if (discreteValue.compare("Female") != 0)
    {
    std::cerr << "Failed to get discrete value\n";
    return EXIT_FAILURE;
    }
  // this will trigger an error...
  double nullValue = factorDiscreteWithValue.GetContinuousValue();
  if (nullValue != 0.0)
    {
    std::cerr << "Failed to get zero when asking for a continuous value from a discrete factor\n";
    return EXIT_FAILURE;
    }
  const int iTypeCont = QdecFactor::qdecContinuousFactorType;
  QdecFactor factorCont( "Age", iTypeCont, 33.0);
  double contValue = factorCont.GetContinuousValue();
  if (contValue != 33.0)
    {
    std::cerr << "Failed to get correct continuous value of 33: " << contValue << "\n";
    return EXIT_FAILURE;
    }

  std::string name = factorCont.GetFactorName();
  if (name.compare("Age") != 0)
    {
    std::cerr << "Factor name " << name.c_str() << " != Age\n";
    return EXIT_FAILURE;
    }
  
  if (factorCont.IsDiscrete())
    {
    std::cerr << "Continuous factor is not supposed to be discrete\n";
    return EXIT_FAILURE;
    }
  if (!factorCont.IsContinuous())
    {
    std::cerr << "Continuous factor is not supposed to be not continuous\n";
    return EXIT_FAILURE;
    }

  factorCont.SetDiscrete();
  if (!factorCont.IsDiscrete())
    {
    std::cerr << "Continuous factor is supposed to be discrete now\n";
    return EXIT_FAILURE;
    }
  if (factorCont.IsContinuous())
    {
    std::cerr << "Continuous factor is not supposed to be continuous now\n";
    return EXIT_FAILURE;
    }

  std::string factorTypeName = factorCont.GetFactorTypeName();
  if (factorTypeName.compare("discrete") != 0)
    {
    std::cerr << "Factor type name should be discrete, instead is " << factorTypeName.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  if (factorCont.HaveDotLevelsFile())
    {
    std::cerr << "Shouldn't have a dot levels file yet!\n";
    return EXIT_FAILURE;
    }
  
  factorCont.SetHaveDotLevelsFile();
  if (!factorCont.HaveDotLevelsFile())
    {
    std::cerr << "Should have a dot levels file now!\n";
    return EXIT_FAILURE;
    }

  std::string levelName = "ALevelName";
  factorCont.AddLevelName(levelName);

  std::vector<std::string> levelnames = factorCont.GetLevelNames();
  if (!factorCont.ValidLevelName(levelName.c_str()))
    {
    std::cerr << "Missing a valid level name!\n";
    return EXIT_FAILURE;
    }

  
  
  return EXIT_SUCCESS;
}
