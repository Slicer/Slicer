/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#ifndef __qSlicerLayerDMModule_h
#define __qSlicerLayerDMModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerLayerDMModuleExport.h"

class qSlicerLayerDMModulePrivate;

class Q_SLICER_QTMODULES_LAYERDM_EXPORT qSlicerLayerDMModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:
  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerLayerDMModule(QObject* parent = nullptr);
  ~qSlicerLayerDMModule() override;

  qSlicerGetTitleMacro(tr("LayerDM"));

  QString helpText() const override;
  QString acknowledgementText() const override;
  QStringList contributors() const override;

  QIcon icon() const override;

  QStringList categories() const override;
  QStringList dependencies() const override;

  bool isHidden() const override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

private:
  Q_DECLARE_PRIVATE(qSlicerLayerDMModule);
  Q_DISABLE_COPY(qSlicerLayerDMModule);
};

#endif
