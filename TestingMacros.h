/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __TestingMacros_h
#define __TestingMacros_h

#include "vtkDebugLeaks.h"
#include "vtkSmartPointer.h"

#define DEBUG_LEAKS_ENABLE_EXIT_ERROR() \
  vtkDebugLeaks::SetExitError(true);

#define EXERCISE_BASIC_OBJECT_METHODS( object ) \
    { \
    if ( object == NULL ) \
      { \
      std::cerr << "EXERCISE_BASIC_OBJECT_METHODS( with NULL object )" << std::endl;  \
      return EXIT_FAILURE;  \
      } \
    object->Print( std::cout );  \
    std::cout << "Name of Class = " << object->GetClassName() << std::endl; \
    std::cout << "Name of Superclass = " << object->Superclass::GetClassName() << std::endl; \
    }

#define TRY_EXPECT_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    std::cerr << "Failed to catch expected exception" << std::endl;  \
    return EXIT_FAILURE;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cout << "Caught expected exception" << std::endl;  \
    std::cout << excp << std::endl; \
    }  


#define TRY_EXPECT_NO_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cerr << excp << std::endl; \
    return EXIT_FAILURE;  \
    }  


#define TEST_ITK_SET_GET( variable, command ) \
  if( variable.GetPointer() != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable.GetPointer() << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }


#define TEST_ITK_SET_GET_VALUE( variable, command ) \
  if( variable != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_BOOLEAN( object, variable ) \
  object->Set##variable( false ); \
  object->Set##variable( true ); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->Set##variable( false ); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##On(); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in On/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##Off(); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Off/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_STRING( object, variable ) \
  { \
  const char * originalStringPointer = object->Get##variable(); \
  std::string originalString; \
  if( originalStringPointer != NULL ) \
    { \
    originalString = originalStringPointer; \
    } \
  std::string string1 = "testingIsGood"; \
  object->Set##variable( string1.c_str() ); \
  if( object->Get##variable() != string1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  std::string string2 = "moreTestingIsBetter"; \
  object->Set##variable( string2.c_str() ); \
  if( object->Get##variable() != string2 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  if( originalStringPointer != NULL ) \
    { \
    object->Set##variable( originalString.c_str() );  \
    } \
  else \
    { \
    object->Set##variable( NULL ); \
    } \
  }


#endif
