/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/


#ifndef __qSlicerCLILoadableModuleFactory_h
#define __qSlicerCLILoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractLibraryFactory.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCLIExport.h"

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryItem
  : public ctkFactoryLibraryItem<qSlicerAbstractCoreModule>
{
public:
  qSlicerCLILoadableModuleFactoryItem(const QString& newTempDirectory);
protected:
  virtual qSlicerAbstractCoreModule* instanciator();
private:
  QString TempDirectory;
};

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModuleFactory :
  public ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractLibraryFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLILoadableModuleFactory();

  /// Reimplemented to scan the directory of the command line modules
  virtual void registerItems();

  /// Extract module name given \a libraryName
  /// For example:
  ///  libThresholdLib.so -> threshold
  ///  libThresholdLib.{dylib, bundle, so} -> threshold
  ///  ThresholdLib.dll -> threshold
  /// \sa qSlicerUtils::extractModuleNameFromLibraryName
  QString fileNameToKey(const QString& fileName)const;

  void setTempDirectory(const QString& newTempDirectory);

protected:
  virtual ctkAbstractFactoryItem<qSlicerAbstractCoreModule>*
    createFactoryFileBasedItem();

  virtual bool isValidFile(const QFileInfo& file)const;

private:
  QString TempDirectory;
};

#endif
