/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

#ifndef __qSlicerCoreModuleFactory_h
#define __qSlicerCoreModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractQObjectFactory.h>

// Slicer includes
#include "qSlicerAbstractModule.h"
#include "qSlicerModulesCoreExport.h"

class qSlicerCoreModuleFactoryPrivate;

class Q_SLICER_MODULES_CORE_EXPORT qSlicerCoreModuleFactory :
  public ctkAbstractQObjectFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractQObjectFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCoreModuleFactory();
  ~qSlicerCoreModuleFactory() override;

  ///
  void registerItems() override;

  ///
  QString objectNameToKey(const QString& objectName) override;

  /// Extract module name given a core module \a className
  /// For example:
  ///  qSlicerCamerasModule -> cameras
  ///  qSlicerTransformsModule -> transforms
  static QString extractModuleName(const QString& className);

protected:
  QScopedPointer<qSlicerCoreModuleFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreModuleFactory);
  Q_DISABLE_COPY(qSlicerCoreModuleFactory);
};

#endif
