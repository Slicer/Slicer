/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSegmentationsModule_h
#define __qSlicerSegmentationsModule_h

// CTK includes
#include "ctkVTKObject.h"

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerSegmentationsModuleExport.h"

class qSlicerSegmentationsModulePrivate;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
  QVTK_OBJECT

public:
  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerSegmentationsModule(QObject *parent=nullptr);
  ~qSlicerSegmentationsModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  QString helpText()const override;

  /// Return acknowledgments
  QString acknowledgementText()const override;

  /// Return the authors of the module
  QStringList  contributors()const override;

  /// Return module dependencies
  QStringList dependencies()const override;

  /// Return a custom icon for the module
  QIcon icon()const override;

  /// Return the categories for the module
  QStringList categories()const override;

  /// Define associated node types
  QStringList associatedNodeTypes()const override;

public slots:
  /// Set up MRML scene events
  void setMRMLScene(vtkMRMLScene* scene) override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

protected slots:
  /// Called when a node is added to the scene. Makes connections to enable
  /// subject hierarchy node creation for each segment to allow per-segment actions in SH.
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

protected:
  QScopedPointer<qSlicerSegmentationsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationsModule);
  Q_DISABLE_COPY(qSlicerSegmentationsModule);

};

#endif
