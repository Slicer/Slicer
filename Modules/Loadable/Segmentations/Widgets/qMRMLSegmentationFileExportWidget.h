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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qMRMLSegmentationFileExportWidget_h
#define __qMRMLSegmentationFileExportWidget_h

// MRMLWidgets includes
#include "qMRMLWidget.h"

#include "qSlicerSegmentationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qMRMLSegmentationFileExportWidgetPrivate;

class vtkMRMLNode;
class vtkMRMLSegmentationNode;
class vtkMRMLSegmentationDisplayNode;
class QItemSelection;

/// \brief Qt widget for selecting a single segment from a segmentation.
///   If multiple segments are needed, then use \sa qMRMLSegmentsTableView instead in SimpleListMode
/// \ingroup Slicer_QtModules_Segmentations_Widgets
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentationFileExportWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

    /// Key for storing selected options in application settings.
    /// If an empty key string is given, then selections are not saved or loaded in settings.
    /// Empty by default.
    Q_PROPERTY(QString settingsKey READ settingsKey WRITE setSettingsKey)

public:
  /// Constructor
  explicit qMRMLSegmentationFileExportWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLSegmentationFileExportWidget() override;

  QString settingsKey()const;
  void setSettingsKey(const QString& key);

  /// Get current segmentation node
  Q_INVOKABLE vtkMRMLSegmentationNode* segmentationNode() const;
  /// Get current segmentation node's ID
  Q_INVOKABLE QString segmentationNodeID();

signals:
  /// Emitted when conversion is done
  void exportToFilesDone();

public slots:
  void setMRMLScene(vtkMRMLScene* mrmlScene) override;

  /// Set segmentation MRML node
  void setSegmentationNode(vtkMRMLSegmentationNode* node);
  void setSegmentationNode(vtkMRMLNode* node);

  void exportToFiles();

  void showDestinationFolder();

  void updateWidgetFromSettings();
  void updateSettingsFromWidget();

  void updateWidgetFromMRML();

protected slots:

  void setFileFormat(const QString&);
  void setColorNodeID(const QString&);
  void setUseLabelsFromColorNode(bool useColorNode);

protected:
  QScopedPointer<qMRMLSegmentationFileExportWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentationFileExportWidget);
  Q_DISABLE_COPY(qMRMLSegmentationFileExportWidget);
};

#endif
