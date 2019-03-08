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

#ifndef __qSlicerColorsModule_h
#define __qSlicerColorsModule_h

/// SlicerQt includes
#include "qSlicerLoadableModule.h"

/// Colors includes
#include "qSlicerColorsModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerColorsModulePrivate;

class Q_SLICER_QTMODULES_COLORS_EXPORT qSlicerColorsModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:
  typedef qSlicerLoadableModule Superclass;
  qSlicerColorsModule(QObject *parent=0);
  virtual ~qSlicerColorsModule();

  virtual QStringList categories()const;
  virtual QIcon icon()const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;
  virtual bool isHidden()const;

  /// Specify editable node types
  virtual QStringList associatedNodeTypes()const;

  virtual void setMRMLScene(vtkMRMLScene* newMRMLScene);

protected:
  /// Reimplemented to initialize the color logic
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerColorsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerColorsModule);
  Q_DISABLE_COPY(qSlicerColorsModule);
};

#endif
