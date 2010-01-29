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
  
  
private:
  /// Not implemented
  qSlicerUtils(){}
  virtual ~qSlicerUtils(){}

};

#endif
