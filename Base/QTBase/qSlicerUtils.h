/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerUtils_h
#define __qSlicerUtils_h

#include <QString>

#include "qSlicerBaseQTBaseExport.h"

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerUtils
{
  
public:
  typedef qSlicerUtils Self; 

  /// 
  /// Return true if the string name ends with one of these executable extension:
  /// ".bat", ".com", ".sh", ".csh", ".tcsh", ".pl", ".py", ".tcl", ".m", ".exe"
  /// Note: The comparison is case insensitive
  static bool isExecutableName(const QString& name);

  ///
  /// Look for target file in build intermediate directory.
  /// On windows, the intermediate directory includes: . Debug RelWithDebInfo Release MinSizeRel
  /// And it return the first matched directory
  /// On the other plateform, this function just return the directory passed as a first argument
  static QString searchTargetInIntDir(const QString& directory, const QString& target);

  /// This function returns an empty string on all plateform expected windows
  /// where it returns ".exe"
  static QString executableExtension();

  /// This function returns ".dll. on windows, ".so" on linux and ".dylib" on Mac
  //static QString libraryExtension(); 
  
  
private:
  /// Not implemented
  qSlicerUtils(){}
  virtual ~qSlicerUtils(){}

};

#endif
