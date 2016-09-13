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

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerSegmentationsModuleExport.h"

class qSlicerSegmentationsModulePrivate;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);
  QVTK_OBJECT

public:
  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerSegmentationsModule(QObject *parent=0);
  virtual ~qSlicerSegmentationsModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  virtual QString helpText()const;

  /// Return acknowledgments
  virtual QString acknowledgementText()const;

  /// Return the authors of the module
  virtual QStringList  contributors()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  /// Return the categories for the module
  virtual QStringList categories()const;

  /// Define associated node types
  virtual QStringList associatedNodeTypes()const;

public slots:
  /// Set up MRML scene events
  virtual void setMRMLScene(vtkMRMLScene* scene);

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

protected slots:
  /// Called when a node is added to the scene. Makes connections to enable
  /// subject hierarchy node creation for each segment to allow per-segment actions in SH.
  void onNodeAdded(vtkObject* scene, vtkObject* nodeObject);

  /// Called when a node is removed from the scene so that the segment subject hierarchy
  /// nodes are removed too
  void onNodeRemoved(vtkObject* scene, vtkObject* nodeObject);

  /// Called when subject hierarchy node is modified. Renames segment if subject hierarchy node has been renamed
  void onSubjectHierarchyNodeModified(vtkObject*);

protected:
  QScopedPointer<qSlicerSegmentationsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationsModule);
  Q_DISABLE_COPY(qSlicerSegmentationsModule);

};

#endif
