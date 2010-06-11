/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKSymbolResolver_h
#define __qCTKSymbolResolver_h

// Qt includes
#include <QLibrary>

// CTK includes
#include <ctkPimpl.h>

#include "qCTKWidgetsExport.h"

class qCTKSymbolResolverPrivate;

class QCTK_WIDGETS_EXPORT qCTKSymbolResolver
{
public:
  qCTKSymbolResolver();
  virtual ~qCTKSymbolResolver();

  /// Returns a text string with the description of the last error that occurred.
  /// Currently, errorString will only be set if load(), unload() or resolve()
  /// for some reason fails.
  QString errorString() const;

  /// This property holds the file name of the library or executable
  QString fileName() const;

  /// Returns true if the library is loaded; otherwise returns false.
  bool isLoaded() const;

  /// Loads the library or executable and returns true if was loaded successfully;
  /// otherwise returns false.
  /// Since resolve() always calls this function before resolving any symbols it is
  /// not necessary to call it explicitly. In some situations you might want the
  /// library or executable loaded in advance, in which case you would use this function.
  bool load();

  /// This property holds give the load() function some hints on how it should behave.
  /// Note that the loading hint will apply only if QLibrary is used
  //QLibrary::LoadHints loadHints() const;

  /// Returns the address of the exported symbol symbol.
  /// The library or executable is loaded if necessary. The function returns 0 if the symbol
  /// could not be resolved or if the library (or executable) could not be loaded.
  void * resolve(const char * symbol);

  /// 
  void setFileName(const QString & fileName);

  /// Sets the fileName property and major version number to fileName and versionNumber respectively.
  /// The versionNumber is ignored on Windows and also on linux if filename is an executable.
  void setFileNameAndVersion(const QString & fileName, int versionNumber);

  /// Sets the fileName  property and full version number to fileName and version respectively.
  /// The version parameter is ignored on Windows and also on linux if filename is an executable.
  void setFileNameAndVersion(const QString & fileName, const QString & version);

  /// 
  //void setLoadHints(QLibrary::LoadHints hints);

  /// Unloads the library or executable and returns true if the library or executable
  /// could be unloaded; otherwise returns false.
  /// This happens automatically on application termination,
  /// so you shouldn't normally need to call this function.
  /// If other instances of QLibrary are using the same library, the call will fail,
  /// and unloading will only happen when every instance has called unload().
  /// Note that on Mac OS X 10.3 (Panther), dynamic libraries cannot be unloaded.
  bool unload();

private:
  CTK_DECLARE_PRIVATE(qCTKSymbolResolver);

};

#endif
