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

#ifndef __qSlicerAbstractModule_h
#define __qSlicerAbstractModule_h

// Qt includes
#include <QIcon>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTGUIExport.h"

class QAction;
class qSlicerAbstractModulePrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModule : public qSlicerAbstractCoreModule
{
  Q_OBJECT

public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerAbstractModule(QObject *parent=0);
  virtual ~qSlicerAbstractModule();

  virtual QIcon icon()const;

  ///
  /// Returns a QAction if the module would like to be added in a toolbar.
  /// By default, returns no QAction.
  QAction * action();
protected:
  QScopedPointer<qSlicerAbstractModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractModule);
  Q_DISABLE_COPY(qSlicerAbstractModule);
};

#endif
