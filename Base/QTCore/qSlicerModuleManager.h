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

#ifndef __qSlicerModuleManager_h
#define __qSlicerModuleManager_h

// Qt includes
#include <QStringList>
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModuleFactoryManager;

class qSlicerModuleManagerPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerModuleManager : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerModuleManager(QObject* newParent = nullptr);
  ~qSlicerModuleManager() override;

  /// Print internal state using qDebug()
  virtual void printAdditionalInfo();

  /// Return a pointer to the current module factory manager
  Q_INVOKABLE qSlicerModuleFactoryManager * factoryManager()const;

  /// Return the list of all the loaded modules
  Q_INVOKABLE QStringList modulesNames()const;

  /// Return the loaded module identified by \a name
  Q_INVOKABLE qSlicerAbstractCoreModule* module(const QString& name)const;

signals:
  void moduleLoaded(const QString& module);
  void moduleAboutToBeUnloaded(const QString& module);

protected:
  QScopedPointer<qSlicerModuleManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleManager);
  Q_DISABLE_COPY(qSlicerModuleManager);
};

#endif
