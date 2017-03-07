/*==============================================================================

  Program: 3D Slicer

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

#ifndef __qSlicerDICOMExportDialog_h
#define __qSlicerDICOMExportDialog_h

// Qt includes
#include <QObject>

// DICOMLib includes
#include "qSlicerDICOMLibModuleWidgetsExport.h"

// MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"

class qSlicerDICOMExportDialogPrivate;
class vtkMRMLScene;
class QItemSelection;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class Q_SLICER_MODULE_DICOMLIB_WIDGETS_EXPORT qSlicerDICOMExportDialog : public QObject
{
public:
  Q_OBJECT

public:
  typedef QObject Superclass;
  qSlicerDICOMExportDialog(QObject* parent = NULL);
  virtual ~qSlicerDICOMExportDialog();

public:
  /// Show dialog
  /// \param nodeToSelect Node is selected in the tree if given
  virtual bool exec(vtkIdType itemToSelect=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);

  /// Set MRML scene
  Q_INVOKABLE void setMRMLScene(vtkMRMLScene* scene);

  /// Python compatibility function for showing dialog (calls \a exec)
  Q_INVOKABLE bool execDialog(vtkIdType itemToSelect=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    { return this->exec(itemToSelect); };

  /// Show DICOM browser and update database to show new items
  Q_INVOKABLE void showUpdatedDICOMBrowser();

protected slots:
  /// Select node that was passed with \sa exec() in subject hierarchy tree
  void selectItem();

  /// Handles change of export series or entire scene radio button selection
  void onExportSeriesRadioButtonToggled(bool);

  /// Triggers examining item when selection changes
  void onCurrentItemChanged(vtkIdType itemID);

  /// Show exportables returned by the plugins for selected node
  void examineSelectedItem();

  /// Populates DICOM tags based on selection
  void onExportableSelectedAtRow(int);

  /// Calls export series or entire scene based on radio button selection
  void onExport();

protected:
  /// Export selected node based on the selected exportable
  void exportSeries();

  /// Export entire scene as a secondary capture containing an MRB
  void exportEntireScene();

protected:
  QScopedPointer<qSlicerDICOMExportDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDICOMExportDialog);
  Q_DISABLE_COPY(qSlicerDICOMExportDialog);
};

#endif
