/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLILoadableModule_h
#define __qSlicerCLILoadableModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerCLIModule.h"

#include "qSlicerBaseQTCLIExport.h"

class qSlicerCLILoadableModulePrivate;
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModule : public qSlicerCLIModule
{
public:

  /// Convenient typedefs
  typedef qSlicerCLIModule Superclass;
  typedef int (*ModuleEntryPointType)(int argc, char* argv[]);
  
  qSlicerCLILoadableModule(QWidget *parent=0);
  virtual ~qSlicerCLILoadableModule(){}

  ///
  /// Set module entry point
  void setEntryPoint(ModuleEntryPointType _entryPoint);

  ///
  /// Get entry point as string
  virtual QString entryPoint()const;

  ///
  /// Get module type
  virtual QString moduleType()const;

protected:
  /// 
  virtual void setup();


private:
  CTK_DECLARE_PRIVATE(qSlicerCLILoadableModule);
};

#endif
