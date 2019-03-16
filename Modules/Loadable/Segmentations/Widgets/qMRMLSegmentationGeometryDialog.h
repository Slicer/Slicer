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
  and CANARIE.

==============================================================================*/

#ifndef __qMRMLSegmentationGeometryDialog_h
#define __qMRMLSegmentationGeometryDialog_h

// Qt includes
#include <QObject>

// Segmentations includes
#include "qSlicerSegmentationsModuleWidgetsExport.h"

#include "qMRMLSegmentationGeometryWidget.h"

class vtkMRMLSegmentationNode;
class qMRMLSegmentationGeometryDialogPrivate;

/// \brief Qt dialog for changing segmentation labelmap geometry
/// \ingroup Slicer_QtModules_Segmentations_Widgets
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentationGeometryDialog : public QObject
{
public:
  Q_OBJECT

  /// Controls for editing (source geometry selector and spacing options box) only appear if editing is enabled
  /// Off by default
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  /// If turned on, the existing labelmaps in the segmentation are resampled in addition to setting reference
  /// image geometry conversion parameter
  /// Otherwise only the reference image geometry parameter is set
  /// Off by default
  Q_PROPERTY(bool resampleLabelmaps READ resampleLabelmaps WRITE setResampleLabelmaps)

public:
  typedef QObject Superclass;
  qMRMLSegmentationGeometryDialog(vtkMRMLSegmentationNode* segmentationNode, QObject* parent=nullptr);
  ~qMRMLSegmentationGeometryDialog() override;

public:
  /// Show dialog
  /// \param nodeToSelect Node is selected in the tree if given
  /// \return Success flag (if dialog result is not Accepted then false)
  virtual bool exec();

  /// Python compatibility function for showing dialog (calls \a exec)
  Q_INVOKABLE bool execDialog() { return this->exec(); };

  bool editEnabled()const;
  bool resampleLabelmaps()const;

public slots:
  void setEditEnabled(bool aEditEnabled);
  void setResampleLabelmaps(bool aResampleLabelmaps);

protected:
  QScopedPointer<qMRMLSegmentationGeometryDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentationGeometryDialog);
  Q_DISABLE_COPY(qMRMLSegmentationGeometryDialog);
};

#endif
