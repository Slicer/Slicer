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
  qSlicerCoreCommandOptions(QSettings* _settings);
  virtual ~qSlicerCoreCommandOptions();

  /// Return True if the loading of Command Line Modules should be disabled
  bool disableCLIModule()const;

  /// Return True if the loading of Loadable Modules should be disabled
  bool disableLoadableModule()const;

  /// Return True if slicer should display version and exit
  bool displayVersionAndExit()const;

  /// Return True if slicer should display program path and exit
  bool displayProgramPathAndExit()const;

  /// Return True if slicer should display home path and exit
  bool displayHomePathAndExit()const;

  /// Return temp directory
  /// Try to read the value from the setting, if no value is found it defaults to
  /// the value returned by QDir::tempPath()
  /// See http://qt.nokia.com/doc/4.6/qdir.html#tempPath
  QString tempDirectory()const;

protected:
  /// Initialize arguments
  virtual void initialize();

  // Disable current settings
  virtual void disableCurrentSettings();

  /// This method is called when wrong argument is found. If it returns False, then
  /// the parsing will fail.
  virtual bool wrongArgument(const char* argument);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreCommandOptions);

};

#endif
