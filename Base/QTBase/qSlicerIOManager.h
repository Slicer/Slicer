/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerIOManager_h
#define __qSlicerIOManager_h 

#include "qSlicerBaseQTBaseWin32Header.h"
#include <QString>

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerIOManager
{

public:
   
  // Description:
  static void loadScene(const QString& fileName);
  
  // Description:
  static void importScene(const QString& fileName);
  
   // Description:
  static void addData(const QString& fileName);
  
  // Description:
  static void addVolume(const QString& fileName);
  
  // Description:
  static void addTransform(const QStringList& fileName);
  
private:
  qSlicerIOManager(){} // Not implemented
  virtual ~qSlicerIOManager(){} // Not implemented
};

#endif
