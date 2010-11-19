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

#ifndef __qSlicerCLIExecutableModuleFactory_h
#define __qSlicerCLIExecutableModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTCLIExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryItem : public ctkAbstractFactoryItem<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractFactoryItem<qSlicerAbstractCoreModule> Superclass;
  explicit qSlicerCLIExecutableModuleFactoryItem(const QString& itemPath);
  virtual ~qSlicerCLIExecutableModuleFactoryItem(){}

  ///
  virtual bool load();

  ///
  /// Return path associated with the executable module
  QString path()const;

protected:
  virtual qSlicerAbstractCoreModule* instanciator();

private:
  QString          Path;
};


//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIExecutableModuleFactory :
  public ctkAbstractPluginFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractPluginFactory<qSlicerAbstractCoreModule> Superclass;
  
  qSlicerCLIExecutableModuleFactory();
  virtual ~qSlicerCLIExecutableModuleFactory();

  ///
  virtual void registerItems();

  ///
  ///   virtual QString fileNameToKey(const QString& fileName);

  /// Extract module name given \a executableName
  /// For example: 
  ///  Threshold.exe -> threshold
  ///  Threshold -> threshold
  static QString extractModuleName(const QString& executableName);

protected:
  virtual ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* createFactoryPluginItem(
    const QFileInfo& plugin);

  QScopedPointer<qSlicerCLIExecutableModuleFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIExecutableModuleFactory);
  Q_DISABLE_COPY(qSlicerCLIExecutableModuleFactory);
};

#endif
