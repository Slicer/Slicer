/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCoreCommandOptions_h
#define __qSlicerCoreCommandOptions_h

/// qCTK includes
#include <qCTKPimpl.h>

/// qVTK includes
#include <qVTKCommandOptions.h>

#include "qSlicerBaseQTCoreExport.h"

class QSettings;
class qSlicerCoreCommandOptionsPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreCommandOptions : public qVTKCommandOptions
{
public:
  typedef qVTKCommandOptions Superclass;
  qSlicerCoreCommandOptions(QSettings* settings);
  virtual ~qSlicerCoreCommandOptions();

  /// Return True if the loading of Command Line Modules should be disabled
  bool disableCLIModule()const;

  /// Return True if slicer should display version and exit
  bool displayVersionAndExit()const;

  /// Return True if slicer should display program path and exit
  bool displayProgramPathAndExit()const;

  /// Return True if slicer should display home path and exit
  bool displayHomePathAndExit()const;

  /// Return temp directory
  QString tempDirectory()const;

protected:
  /// Initialize arguments
  virtual void initialize();

  // Disable current settings
  virtual void disableCurrentSettings();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreCommandOptions);

};

#endif
