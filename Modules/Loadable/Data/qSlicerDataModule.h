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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerDataModule_h
#define __qSlicerDataModule_h

/// SlicerQt includes
#include "qSlicerLoadableModule.h"

/// Data includes
#include "qSlicerDataModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerDataModulePrivate;

class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerDataModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);
public:
  typedef qSlicerLoadableModule Superclass;
  qSlicerDataModule(QObject *parent=0);
  virtual ~qSlicerDataModule();

  virtual void setup();

  virtual QIcon icon()const;
  virtual QStringList categories()const;
  virtual QStringList dependencies()const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;
protected:

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerDataModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDataModule);
  Q_DISABLE_COPY(qSlicerDataModule);
};

#endif
