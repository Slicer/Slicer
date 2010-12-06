/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerLoadableModuleFactory_h
#define __qSlicerLoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreExport.h"

class qSlicerLoadableModuleFactoryPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerLoadableModuleFactory :
  public ctkAbstractPluginFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractPluginFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerLoadableModuleFactory();
  virtual ~qSlicerLoadableModuleFactory();

  ///
  virtual void registerItems();

  ///
  virtual QString fileNameToKey(const QString& fileName);

  /// Extract module name given \a libraryName
  /// \sa qSlicerUtils::extractModuleNameFromLibraryName
  static QString extractModuleName(const QString& libraryName);

protected:
  QScopedPointer<qSlicerLoadableModuleFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerLoadableModuleFactory);
  Q_DISABLE_COPY(qSlicerLoadableModuleFactory);
};

#endif
