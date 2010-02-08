/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCommandOptions_h
#define __qSlicerCommandOptions_h

/// qCTK includes
#include <qCTKPimpl.h>

/// SlicerQT includes
#include "qSlicerCoreCommandOptions.h"

#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerCommandOptionsPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerCommandOptions : public qSlicerCoreCommandOptions
{
public:
  typedef qSlicerCoreCommandOptions Superclass;
  qSlicerCommandOptions(QSettings* _settings);
  virtual ~qSlicerCommandOptions();

  bool noSplash()const;

protected:
  /// Initialize arguments
  virtual void initialize();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCommandOptions);

};

#endif
