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

#ifndef __qSlicerViewControllersModule_h
#define __qSlicerViewControllersModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerViewControllersModuleExport.h"

class QSettings;

class qSlicerViewControllersModulePrivate;
class vtkMRMLAbstractViewNode;
class vtkMRMLPlotViewNode;
class vtkMRMLSliceNode;
class vtkMRMLViewNode;

class Q_SLICER_QTMODULES_VIEWCONTROLLERS_EXPORT qSlicerViewControllersModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerViewControllersModule(QObject *parent=nullptr);
  ~qSlicerViewControllersModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  QStringList categories()const override;
  QIcon icon()const override;
  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  /// Read default slice view settings from application settings (.ini file)
  /// into defaultViewNode.
  static void readDefaultSliceViewSettings(vtkMRMLSliceNode* defaultViewNode);

  /// Read default 3D view settings from application settings (.ini file)
  /// into defaultViewNode.
  static void writeDefaultSliceViewSettings(vtkMRMLSliceNode* defaultViewNode);

  /// Write default slice view settings to application settings (.ini file)
  /// from defaultViewNode.
  static void readDefaultThreeDViewSettings(vtkMRMLViewNode* defaultViewNode);

  /// Write default 3D  view settings to application settings (.ini file)
  /// from defaultViewNode.
  static void writeDefaultThreeDViewSettings(vtkMRMLViewNode* defaultViewNode);

  /// Read default plot view settings from application settings (.ini file)
  /// into defaultViewNode.
  void readDefaultPlotViewSettings(vtkMRMLPlotViewNode *defaultViewNode);

  /// Write default plot view settings to application settings (.ini file)
  /// from defaultViewNode.
  void writeDefaultPlotViewSettings(vtkMRMLPlotViewNode *defaultViewNode);

  /// Set MRML scene for the module. Updates the default view settings based on
  /// the application settings.
  void setMRMLScene(vtkMRMLScene* scene) override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  /// Helper functions to read/write common view settings
  static void readCommonViewSettings(vtkMRMLAbstractViewNode* defaultViewNode, QSettings& settings);
  static void writeCommonViewSettings(vtkMRMLAbstractViewNode* defaultViewNode, QSettings& settings);

protected:
  QScopedPointer<qSlicerViewControllersModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerViewControllersModule);
  Q_DISABLE_COPY(qSlicerViewControllersModule);
};

#endif
