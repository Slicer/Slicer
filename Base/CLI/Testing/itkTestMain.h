/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef itkTestMain_h
#define itkTestMain_h

// This file is used to create TestDriver executables
// These executables are able to register a function pointer to a string name
// in a lookup table.   By including this file, it creates a main function
// that calls RegisterTests() then looks up the function pointer for the test
// specified on the command line.
#include "itkWin32Header.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "itkMultiThreader.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIterator.h"
#include "itkSubtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkTestingComparisonImageFilter.h"
#include "itksys/SystemTools.hxx"
#include "itkIntTypes.h"
#include "itkFloatingPointExceptions.h"
#include <itkFactoryRegistration.h>

#define ITK_TEST_DIMENSION_MAX 6

typedef int ( *MainFuncPointer )(int, char *[]);
std::map<std::string, MainFuncPointer> StringToTestFunctionMap;

#define REGISTER_TEST(test)       \
  extern int test(int, char *[]); \
  StringToTestFunctionMap[#test] = test

int RegressionTestImage(const char *testImageFilename,
                        const char *baselineImageFilename,
                        int reportErrors,
                        double intensityTolerance,
                        ::itk::SizeValueType numberOfPixelsTolerance = 0,
                        unsigned int radiusTolerance = 0);

std::map<std::string, int> RegressionTestBaselines(char *);

void RegisterTests();

void PrintAvailableTests()
{
  std::cout << "Available tests:\n";
  std::map<std::string, MainFuncPointer>::iterator j = StringToTestFunctionMap.begin();
  int                                              i = 0;
  while( j != StringToTestFunctionMap.end() )
    {
    std::cout << i << ". " << j->first << "\n";
    ++i;
    ++j;
    }
}

int main(int ac, char *av[])
{
  itk::FloatingPointExceptions::Enable();

  double       intensityTolerance  = 2.0;
  unsigned int numberOfPixelsTolerance = 0;
  unsigned int radiusTolerance = 0;

  typedef std::pair<char *, char *> ComparePairType;
  std::vector<ComparePairType> compareList;

  itk::itkFactoryRegistration();

  RegisterTests();
  std::string testToRun;
  if( ac < 2 )
    {
    PrintAvailableTests();
    std::cout << "To run a test, enter the test number: ";
    int testNum = 0;
    std::cin >> testNum;
    std::map<std::string, MainFuncPointer>::iterator j = StringToTestFunctionMap.begin();
    int                                              i = 0;
    while( j != StringToTestFunctionMap.end() && i < testNum )
      {
      ++i;
      ++j;
      }

    if( j == StringToTestFunctionMap.end() )
      {
      std::cerr << testNum << " is an invalid test number\n";
      return -1;
      }
    testToRun = j->first;
    }
  else
    {
    while( ac > 0 && testToRun.empty() )
      {
      if( strcmp(av[1], "--with-threads") == 0 )
        {
        int numThreads = atoi(av[2]);
        itk::MultiThreader::SetGlobalDefaultNumberOfThreads(numThreads);
        av += 2;
        ac -= 2;
        }
      else if( strcmp(av[1], "--without-threads") == 0 )
        {
        itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);
        av += 1;
        ac -= 1;
        }
      else if( ac > 3 && strcmp(av[1], "--compare") == 0 )
        {
        compareList.emplace_back(av[2], av[3]);
        av += 3;
        ac -= 3;
        }
      else if( ac > 2 && strcmp(av[1], "--compareNumberOfPixelsTolerance") == 0 )
        {
        numberOfPixelsTolerance = atoi(av[2]);
        av += 2;
        ac -= 2;
        }
      else if( ac > 2 && strcmp(av[1], "--compareRadiusTolerance") == 0 )
        {
        radiusTolerance = atoi(av[2]);
        av += 2;
        ac -= 2;
        }
      else if( ac > 2 && strcmp(av[1], "--compareIntensityTolerance") == 0 )
        {
        intensityTolerance = atof(av[2]);
        av += 2;
        ac -= 2;
        }
      else
        {
        testToRun = av[1];
        }
      }
    }
  std::map<std::string, MainFuncPointer>::iterator j = StringToTestFunctionMap.find(testToRun);
  if( j != StringToTestFunctionMap.end() )
    {
    MainFuncPointer f = j->second;
    int             result;
    try
      {
      // Invoke the test's "main" function.
      result = ( *f )( ac - 1, av + 1 );
      // Make a list of possible baselines
      for( int i = 0; i < static_cast<int>( compareList.size() ); i++ )
        {
        char *                               baselineFilename = compareList[i].first;
        char *                               testFilename = compareList[i].second;
        std::map<std::string, int>           baselines = RegressionTestBaselines(baselineFilename);
        std::map<std::string, int>::iterator baseline = baselines.begin();
        std::string                          bestBaseline;
        int                                  bestBaselineStatus = itk::NumericTraits<int>::max();
        while( baseline != baselines.end() )
          {
          baseline->second = RegressionTestImage(testFilename,
                                                 ( baseline->first ).c_str(),
                                                 0,
                                                 intensityTolerance,
                                                 numberOfPixelsTolerance,
                                                 radiusTolerance);
          if( baseline->second < bestBaselineStatus )
            {
            bestBaseline = baseline->first;
            bestBaselineStatus = baseline->second;
            }
          if( baseline->second == 0 )
            {
            break;
            }
          ++baseline;
          }

        // if the best we can do still has errors, generate the error images
        if( bestBaselineStatus )
          {
          RegressionTestImage(testFilename,
                              bestBaseline.c_str(),
                              1,
                              intensityTolerance,
                              numberOfPixelsTolerance,
                              radiusTolerance);
          }

        // output the matching baseline
        std::cout << "<DartMeasurement name=\"BaselineImageName\" type=\"text/string\">";
        std::cout << itksys::SystemTools::GetFilenameName(bestBaseline);
        std::cout << "</DartMeasurement>" << std::endl;

        result += bestBaselineStatus;
        }
      }
    catch( const itk::ExceptionObject & e )
      {
      std::cerr << "ITK test driver caught an ITK exception:\n";
      e.Print(std::cerr);
      result = -1;
      }
    catch( const std::exception & e )
      {
      std::cerr << "ITK test driver caught an exception:\n";
      std::cerr << e.what() << "\n";
      result = -1;
      }
    catch( ... )
      {
      std::cerr << "ITK test driver caught an unknown exception!!!\n";
      result = -1;
      }
    return result;
    }
  PrintAvailableTests();
  std::cerr << "Failed: " << testToRun << ": No test registered with name " << testToRun << "\n";
  return -1;
}

// Regression Testing Code

int RegressionTestImage(const char *testImageFilename,
                        const char *baselineImageFilename,
                        int reportErrors,
                        double intensityTolerance,
                        ::itk::SizeValueType numberOfPixelsTolerance,
                        unsigned int radiusTolerance)
{
  // Use the factory mechanism to read the test and baseline files and convert
  // them to double
  typedef itk::Image<double, ITK_TEST_DIMENSION_MAX>        ImageType;
  typedef itk::Image<unsigned char, ITK_TEST_DIMENSION_MAX> OutputType;
  typedef itk::Image<unsigned char, 2>                      DiffOutputType;
  typedef itk::ImageFileReader<ImageType>                   ReaderType;

  // Read the baseline file
  ReaderType::Pointer baselineReader = ReaderType::New();
  baselineReader->SetFileName(baselineImageFilename);
  try
    {
    baselineReader->UpdateLargestPossibleRegion();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "Exception detected while reading " << baselineImageFilename << " : "  << e.GetDescription();
    return 1000;
    }

  // Read the file generated by the test
  ReaderType::Pointer testReader = ReaderType::New();
  testReader->SetFileName(testImageFilename);
  try
    {
    testReader->UpdateLargestPossibleRegion();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "Exception detected while reading " << testImageFilename << " : "  << e.GetDescription() << std::endl;
    return 1000;
    }

  // The sizes of the baseline and test image must match
  ImageType::SizeType baselineSize;
  baselineSize = baselineReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  ImageType::SizeType testSize;
  testSize = testReader->GetOutput()->GetLargestPossibleRegion().GetSize();

  if( baselineSize != testSize )
    {
    std::cerr << "The size of the Baseline image and Test image do not match!" << std::endl;
    std::cerr << "Baseline image: " << baselineImageFilename
              << " has size " << baselineSize << std::endl;
    std::cerr << "Test image:     " << testImageFilename
              << " has size " << testSize << std::endl;
    return 1;
    }

  // Now compare the two images
  typedef itk::Testing::ComparisonImageFilter<ImageType, ImageType> DiffType;
  DiffType::Pointer diff = DiffType::New();
  diff->SetValidInput( baselineReader->GetOutput() );
  diff->SetTestInput( testReader->GetOutput() );
  diff->SetDifferenceThreshold(intensityTolerance);
  diff->SetToleranceRadius(radiusTolerance);
  diff->UpdateLargestPossibleRegion();

  itk::SizeValueType status = itk::NumericTraits<itk::SizeValueType>::ZeroValue();
  status = diff->GetNumberOfPixelsWithDifferences();

  // if there are discrepancies, create an diff image
  if( ( status > numberOfPixelsTolerance ) && reportErrors )
    {
    typedef itk::RescaleIntensityImageFilter<ImageType, OutputType> RescaleType;
    typedef itk::ImageFileWriter<DiffOutputType>                    WriterType;
    typedef itk::ImageRegion<ITK_TEST_DIMENSION_MAX>                RegionType;
    OutputType::SizeType size; size.Fill(0);

    RescaleType::Pointer rescale = RescaleType::New();
    rescale->SetOutputMinimum( itk::NumericTraits<unsigned char>::NonpositiveMin() );
    rescale->SetOutputMaximum( itk::NumericTraits<unsigned char>::max() );
    rescale->SetInput( diff->GetOutput() );
    rescale->UpdateLargestPossibleRegion();
    size = rescale->GetOutput()->GetLargestPossibleRegion().GetSize();

    // Get the center slice of the image,  In 3D, the first slice
    // is often a black slice with little debugging information.
    OutputType::IndexType index; index.Fill(0);
    for( unsigned int i = 2; i < ITK_TEST_DIMENSION_MAX; i++ )
      {
      index[i] = size[i] / 2; // NOTE: Integer Divide used to get approximately
                              // the center slice
      size[i] = 0;
      }

    RegionType region;
    region.SetIndex(index);

    region.SetSize(size);

    typedef itk::ExtractImageFilter<OutputType, DiffOutputType> ExtractType;
    ExtractType::Pointer extract = ExtractType::New();
    extract->SetDirectionCollapseToGuess(); // ITKv3 compatible, but not recommended
    extract->SetInput( rescale->GetOutput() );
    extract->SetExtractionRegion(region);

    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( extract->GetOutput() );

    std::cout << "<DartMeasurement name=\"ImageError\" type=\"numeric/double\">";
    std::cout << status;
    std::cout <<  "</DartMeasurement>" << std::endl;

    std::ostringstream diffName;
    diffName << testImageFilename << ".diff.png";
    try
      {
      rescale->SetInput( diff->GetOutput() );
      rescale->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during rescale of " << diffName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during rescale of " << diffName.str() << std::endl;
      }
    writer->SetFileName( diffName.str().c_str() );
    try
      {
      writer->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during write of " << diffName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during write of " << diffName.str() << std::endl;
      }

    std::cout << "<DartMeasurementFile name=\"DifferenceImage\" type=\"image/png\">";
    std::cout << diffName.str();
    std::cout << "</DartMeasurementFile>" << std::endl;

    std::ostringstream baseName;
    baseName << testImageFilename << ".base.png";
    try
      {
      rescale->SetInput( baselineReader->GetOutput() );
      rescale->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during rescale of " << baseName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during rescale of " << baseName.str() << std::endl;
      }
    try
      {
      writer->SetFileName( baseName.str().c_str() );
      writer->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during write of " << baseName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during write of " << baseName.str() << std::endl;
      }

    std::cout << "<DartMeasurementFile name=\"BaselineImage\" type=\"image/png\">";
    std::cout << baseName.str();
    std::cout << "</DartMeasurementFile>" << std::endl;

    std::ostringstream testName;
    testName << testImageFilename << ".test.png";
    try
      {
      rescale->SetInput( testReader->GetOutput() );
      rescale->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during rescale of " << testName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during rescale of " << testName.str() << std::endl;
      }
    try
      {
      writer->SetFileName( testName.str().c_str() );
      writer->Update();
      }
    catch( const std::exception & e )
      {
      std::cerr << "Error during write of " << testName.str() << std::endl;
      std::cerr << e.what() << "\n";
      }
    catch( ... )
      {
      std::cerr << "Error during write of " << testName.str() << std::endl;
      }

    std::cout << "<DartMeasurementFile name=\"TestImage\" type=\"image/png\">";
    std::cout << testName.str();
    std::cout << "</DartMeasurementFile>" << std::endl;
    }
  return ( status > numberOfPixelsTolerance ) ? 1 : 0;
}

//
// Generate all of the possible baselines
// The possible baselines are generated fromn the baselineFilename using the
// following algorithm:
// 1) strip the suffix
// 2) append a digit .x
// 3) append the original suffix.
// It the file exists, increment x and continue
//
std::map<std::string, int> RegressionTestBaselines(char *baselineFilename)
{
  std::map<std::string, int> baselines;
  baselines[std::string(baselineFilename)] = 0;

  std::string originalBaseline(baselineFilename);

  int                    x = 0;
  std::string::size_type suffixPos = originalBaseline.rfind(".");
  std::string            suffix;
  if( suffixPos != std::string::npos )
    {
    suffix = originalBaseline.substr( suffixPos, originalBaseline.length() );
    originalBaseline.erase( suffixPos, originalBaseline.length() );
    }
  while( ++x )
    {
    std::ostringstream filename;
    filename << originalBaseline << "." << x << suffix;
    std::ifstream filestream( filename.str().c_str() );
    if( !filestream )
      {
      break;
      }
    baselines[filename.str()] = 0;
    filestream.close();
    }

  return baselines;
}

#endif
