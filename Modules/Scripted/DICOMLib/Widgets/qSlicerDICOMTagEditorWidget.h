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

#ifndef __qSlicerDICOMTagEditorWidget_h
#define __qSlicerDICOMTagEditorWidget_h

// Qt includes
#include "QWidget"

// DICOMLib includes
#include "qSlicerDICOMLibModuleWidgetsExport.h"

class qSlicerDICOMExportable;
class qSlicerDICOMTagEditorWidgetPrivate;
class vtkMRMLScene;

/// Widget for showing and editing pseudo-tags for series to export. Pseudo-tag is a
/// pair of strings (name, value) that DICOM plugins use to set values of real DICOM tags in
/// the exported data set.
/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_DICOMLIB_WIDGETS_EXPORT qSlicerDICOMTagEditorWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerDICOMTagEditorWidget(QWidget *parent=nullptr);
  ~qSlicerDICOMTagEditorWidget() override;

public:
  /// Set exportables to show the series pseudo-tags to edit.
  /// Creates and populates tables.
  /// \param exportables List of exportable objects
  /// \return Empty string if input is valid, error message if
  ///   invalid (e.g. when the nodes referenced by exportables are
  /// in different study)
  QString setExportables(QList<qSlicerDICOMExportable*> exportables);

  /// Get exportable list
  QList<qSlicerDICOMExportable*> exportables()const;

  /// Clear exportables and tables
  void clear();

  /// Write edited tags into the subject hierarchy item attributes
  void commitChangesToItems();

signals:
  /// Signal emitted each time a tag was edited by the user
  void tagEdited();

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Sets new tag value in corresponding exportable
  void tagsTableCellChanged(int row, int column);

protected:
  QScopedPointer<qSlicerDICOMTagEditorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDICOMTagEditorWidget);
  Q_DISABLE_COPY(qSlicerDICOMTagEditorWidget);
};

#endif
