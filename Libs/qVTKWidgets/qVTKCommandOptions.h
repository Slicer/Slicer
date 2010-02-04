/*=========================================================================
  
  Program:   ParaView
  Module:    $RCSfile: vtkCommandOptions.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qVTKCommandOptions_h
#define __qVTKCommandOptions_h

/// qCTK includes
#include <qCTKPimpl.h>

#include "qVTKWidgetsExport.h"

class QSettings;
class qVTKCommandOptionsPrivate;

class QVTK_WIDGETS_EXPORT qVTKCommandOptions
{
public:
  /// Constructors
  explicit qVTKCommandOptions(QSettings* settings);
  virtual ~qVTKCommandOptions();

  enum
  {
    ALL = 0,
    QSETTINGS_ONLY = 0x1
  };

  ///
  /// Parse the arguments
  bool parse(int argc, const char* const argv[]);

  ///
  /// If parse return False, this function will return the unparsed arguments
  void remainingArguments(int* argc, char** argv[]);

  ///
  /// Get the index of the last parsed argument.
  int indexOfLastParsedArgument();
  
  ///
  /// Was help selected?
  bool helpSelected()const;
  QString help(); 

  ///
  /// Should user settings be considered ?
  bool disableSettings()const;

  ///
  /// Set/Get the type of the process for this set of options.
  /// GUI application, Batch, Daemon, etc ...
  int processType()const;
  void setProcessType(int p);

  ///
  /// In case of unknown argument, return its name.
  QString unknownArgument()const;

  ///
  /// Get the error message if Parse returned 0.
  QString errorMessage()const;

protected:
  
  /// Add a command line argument.
  /// For each argument added there is a long version --long and a short version -l,
  /// a help string, and a variable that is set to the value of the option.
  /// The types can be int, char*, or boolean (set to 1 if argument is present).
  /// Also deprecated arguments can be added with only a help string.  The help
  /// string should say that the argument is deprecated and suggest the
  /// alternative argument to use.
  void addBooleanArgument(const char* longarg, const char* shortarg,
                          bool* var, const char* arghelp, bool defaultValue = false,
                          int type=ALL);
  void addDeprecatedArgument(const char* longarg, const char* shortarg,
                             const char* arghelp, int type=ALL);
  void addArgument(const char* longarg, const char* shortarg,
                   int* var, const char* arghelp, int defaultValue = 0, int type=ALL);
  void addArgument(const char* longarg, const char* shortarg,
                   QString* var, const char* arghelp, const QString& defaultValue, int type=ALL);


  /// Initialize arguments.
  virtual void initialize();

  /// Disable current settings
  virtual void disableCurrentSettings() = 0;
  
  ///
  /// After parsing, process extra option dependencies.
  virtual bool postProcess(int argc, const char* const* argv);

  /// This method is called when wrong argument is found. If it returns False, then
  /// the parsing will fail.
  virtual bool wrongArgument(const char* argument);

  /// This method is called when a deprecated argument is found. If it returns False, then
  /// the parsing will fail.
  virtual bool deprecatedArgument(const char* argument);
  
private:
  QCTK_DECLARE_PRIVATE(qVTKCommandOptions);
}; 

#endif
