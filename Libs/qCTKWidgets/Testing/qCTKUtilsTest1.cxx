/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// qCTK includes
#include "qCTKUtils.h"

// QT includes
#include <QStringList>

// STL includes
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

int qCTKUtilsTest1(int argc, char * argv [] )
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);  

  // Test qListToSTLVector(const QStringList& list, std::vector<char*>& vector)
  
  QStringList inputStringList;
  inputStringList << "Testing";
  inputStringList << " is ";
  inputStringList << "awesome !";

  std::vector<char*> outputCharVector;
  
  qCTKUtils::qListToSTLVector(inputStringList, outputCharVector);

  if (outputCharVector.size() != 3)
    {
    std::cerr << "Error in qListToSTLVector(const QStringList&, std::vector<char*>&)" << std::endl
              << "outputCharVector should contains 3 elements." << std::endl;
    return EXIT_FAILURE;
    }
  
  if ((strcmp(outputCharVector[0], "Testing") != 0) ||
      (strcmp(outputCharVector[1], " is ") != 0) ||
      (strcmp(outputCharVector[2], "awesome !") != 0))
    {
    std::cerr << "Error in qListToSTLVector(const QStringList&, std::vector<char*>&)" << std::endl
              << "Content of outputCharVector is incorrect" << std::endl
              << "inputStringList[0] => [" << qPrintable(inputStringList[0]) << "]" << std::endl
              << "inputStringList[1] => [" << qPrintable(inputStringList[1]) << "]" << std::endl
              << "inputStringList[2] => [" << qPrintable(inputStringList[2]) << "]" << std::endl
              << "outputCharVector[0] => [" << outputCharVector[0] << "]" << std::endl
              << "outputCharVector[1] => [" << outputCharVector[1] << "]" << std::endl
              << "outputCharVector[2] => [" << outputCharVector[2] << "]" << std::endl;
    return EXIT_FAILURE;
    }

  delete [] outputCharVector[0];
  delete [] outputCharVector[1];
  delete [] outputCharVector[2]; 

  
  
  // Test qListToSTLVector(const QStringList& list, std::vector<std::string>& vector)

  std::vector<std::string> outputStringVector;

  qCTKUtils::qListToSTLVector(inputStringList, outputStringVector);

  if (outputStringVector.size() != 3)
    {
    std::cerr << "Error in qListToSTLVector(const QStringList&, std::vector<std::string>&)" << std::endl
              << "outputStringVector should contains 3 elements." << std::endl;
    return EXIT_FAILURE;
    }  

  if ((outputStringVector[0].compare("Testing") != 0) ||
      (outputStringVector[1].compare(" is ") != 0) ||
      (outputStringVector[2].compare("awesome !") != 0))
    {
    std::cerr << "Error in qListToSTLVector(const QStringList&, std::vector<std::string>&)" << std::endl
              << "Content of outputStringVector is incorrect" << std::endl
              << "inputStringList[0] => [" << qPrintable(inputStringList[0]) << "]" << std::endl
              << "inputStringList[1] => [" << qPrintable(inputStringList[1]) << "]" << std::endl
              << "inputStringList[2] => [" << qPrintable(inputStringList[2]) << "]" << std::endl
              << "outputStringVector[0] => [" << outputStringVector[0] << "]" << std::endl
              << "outputStringVector[1] => [" << outputStringVector[1] << "]" << std::endl
              << "outputStringVector[2] => [" << outputStringVector[2] << "]" << std::endl;
    return EXIT_FAILURE;
    }


  // Test stlVectorToQList(const std::vector<std::string>& vector, QStringList& list)
  
  std::vector<std::string> inputStringVector;
  inputStringVector.push_back("Testing");
  inputStringVector.push_back(" is ");
  inputStringVector.push_back("awesome !");

  QStringList ouputStringList;

  qCTKUtils::stlVectorToQList(inputStringVector, ouputStringList);

  if (ouputStringList.size() != 3)
    {
    std::cerr << "Error in stlVectorToQList(const std::vector<std::string>&, QStringList&)" << std::endl
              << "ouputStringList should contains 3 elements." << std::endl;
    return EXIT_FAILURE;
    }
    
  if ((ouputStringList[0] != QLatin1String("Testing")) ||
      (ouputStringList[1] != QLatin1String(" is ")) ||
      (ouputStringList[2] != QLatin1String("awesome !")))
    {
    std::cerr << "Error in stlVectorToQList(const std::vector<std::string>&, QStringList&)" << std::endl
              << "Content of ouputStringList is incorrect" << std::endl
              << "inputStringVector[0] => [" << inputStringVector[0] << "]" << std::endl
              << "inputStringVector[1] => [" << inputStringVector[1] << "]" << std::endl
              << "inputStringVector[2] => [" << inputStringVector[2] << "]" << std::endl
              << "ouputStringList[0] => [" << qPrintable(ouputStringList[0]) << "]" << std::endl
              << "ouputStringList[1] => [" << qPrintable(ouputStringList[1]) << "]" << std::endl
              << "ouputStringList[2] => [" << qPrintable(ouputStringList[2]) << "]" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
