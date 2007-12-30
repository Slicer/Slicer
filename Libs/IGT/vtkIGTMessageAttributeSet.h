/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#ifndef IGTMESSAGEATTRIBUTESET_H
#define IGTMESSAGEATTRIBUTESET_H

#include <string>
#include <map>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"

#include "vtkIGTMessageAttributeBase.h"
#include "vtkTimeStamp.h"

class vtkIGTOpenTrackerStream;

class VTK_IGT_EXPORT vtkIGTMessageAttributeSet : public vtkObject {
public:

  //BTX
  enum {
    TYPE_BOOL = 1,      // bool
    TYPE_CHAR,          // char
    TYPE_SIGNED_CHAR,   // signed char
    TYPE_UNSIGNED_CHAR, // unsigned char
    TYPE_INT,           // int
    TYPE_LONG,          // long
    TYPE_SHORT,         // short
    TYPE_UNSIGNED_INT,  // unsigned int
    TYPE_UNSIGNED_LONG, // unsigned long
    TYPE_UNSIGNED_SHORT,// unsigned short
    TYPE_DOUBLE,        // double
    TYPE_LONG_DOUBLE,   // long double
    TYPE_FLOAT,         // float
    TYPE_STRING,        // std::string
    TYPE_VECTOR_FLOAT,  // std::vector<float>
    TYPE_VTK_IMAGE_DATA // vtkImageData
  };
  //ETX

  //BTX
  typedef std::map<std::string, vtkIGTMessageAttributeBase*> AttributeMapType;
  typedef std::map<std::string, vtkIGTMessageAttributeSet*>  AttributeSetMap;
  typedef void MessageHandlingFunction(vtkIGTMessageAttributeSet*, void*);
  typedef AttributeMapType::iterator iterator;
  //ETX

public:

  static vtkIGTMessageAttributeSet *New();
  vtkTypeRevisionMacro(vtkIGTMessageAttributeSet,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkIGTMessageAttributeSet();
  ~vtkIGTMessageAttributeSet();

  //BTX
  template <typename T>
  void AddAttribute(const char* key, T* ptr);
  
  template <typename T>
  int GetAttribute(const char* key, T* ptr);
  //ETX

  // iterator interface:
  iterator begin() {
    return  AttributeMap.begin();
  }

  iterator end() {
    return AttributeMap.end();
  }

  vtkIGTOpenTrackerStream* GetOpenTrackerStream(void)
  {
    return this->OpenTrackerStream;
  }

  vtkTimeStamp* GetTimeStamp()
  {
    return this->TimeStamp;
  }
  

protected:

  void SetHandlerFunction(MessageHandlingFunction* func, void* argument);
  MessageHandlingFunction* GetHandlerFunction();
  void* GetHandlerArgument();

  void SetOpenTrackerStream(vtkIGTOpenTrackerStream* ptr)
  {
    this->OpenTrackerStream = ptr;
  }

private:

  vtkIGTOpenTrackerStream* OpenTrackerStream;
  MessageHandlingFunction* HandlerFunction;
  void*                    HandlerArgument;

  //BTX
  AttributeMapType         AttributeMap;

  friend class vtkIGTOpenTrackerStream;
  //ETX

  vtkTimeStamp *TimeStamp;


};

//#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkIGTMessageAttributeSet.txx"
//#endif 

#endif // IGTMESSAGEATTRIBUTESET_H
