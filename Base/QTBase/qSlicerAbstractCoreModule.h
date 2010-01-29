/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractCoreModule_h
#define __qSlicerAbstractCoreModule_h

/// SlicerQT includes
#include "qSlicerAbstractModule.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTBaseExport.h"

class qSlicerAbstractCoreModulePrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerAbstractCoreModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModule Superclass;
  qSlicerAbstractCoreModule(QObject *parent=0);

protected:
  virtual void setup();

  /// 
  /// Convenient method to return slicer wiki URL
  QString slicerWikiUrl()const{ return "http://www.slicer.org/slicerWiki/index.php"; }

private:
  QCTK_DECLARE_PRIVATE(qSlicerAbstractCoreModule);
};

#endif
