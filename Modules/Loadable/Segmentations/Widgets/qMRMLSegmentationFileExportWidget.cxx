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

// Segmentations includes
#include "qMRMLSegmentationFileExportWidget.h"

#include "ui_qMRMLSegmentationFileExportWidget.h"

#include "vtkSlicerSegmentationsModuleLogic.h"

#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkStringArray.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>

// CTK includes
#include <ctkMessageBox.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include <vtkMRMLSliceLogic.h>
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>
#include <vtkSlicerApplicationLogic.h>

//-----------------------------------------------------------------------------
class qMRMLSegmentationFileExportWidgetPrivate: public Ui_qMRMLSegmentationFileExportWidget
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationFileExportWidget);

protected:
  qMRMLSegmentationFileExportWidget* const q_ptr;
public:
  qMRMLSegmentationFileExportWidgetPrivate(qMRMLSegmentationFileExportWidget& object);
  void init();

public:
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;
  QString SettingsKey;
};

//-----------------------------------------------------------------------------
qMRMLSegmentationFileExportWidgetPrivate::qMRMLSegmentationFileExportWidgetPrivate(qMRMLSegmentationFileExportWidget& object)
  : q_ptr(&object)
{
  this->SegmentationNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidgetPrivate::init()
{
  Q_Q(qMRMLSegmentationFileExportWidget);
  this->setupUi(q);
  q->setEnabled(false);
  QObject::connect(this->ExportToFilesButton, SIGNAL(clicked()),
    q, SLOT(exportToFiles()));
  QObject::connect(this->ShowDestinationFolderButton, SIGNAL(clicked()),
    q, SLOT(showDestinationFolder()));
  QObject::connect(this->FileFormatComboBox, SIGNAL(currentIndexChanged(const QString&)),
    q, SLOT(setFileFormat(const QString&)));
  QObject::connect(this->ColorTableNodeSelector, SIGNAL(currentNodeIDChanged(const QString&)),
    q, SLOT(setColorNodeID(const QString&)));
  QObject::connect(this->UseColorTableValuesCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setUseLabelsFromColorNode(bool)));
}

//-----------------------------------------------------------------------------
// qMRMLSegmentationFileExportWidget methods

//-----------------------------------------------------------------------------
qMRMLSegmentationFileExportWidget::qMRMLSegmentationFileExportWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentationFileExportWidgetPrivate(*this))
{
  Q_D(qMRMLSegmentationFileExportWidget);
  d->init();
  this->updateWidgetFromSettings();
}

//-----------------------------------------------------------------------------
qMRMLSegmentationFileExportWidget::~qMRMLSegmentationFileExportWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setMRMLScene(vtkMRMLScene* scene)
{
  qMRMLWidget::setMRMLScene(scene);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* qMRMLSegmentationFileExportWidget::segmentationNode() const
{
  Q_D(const qMRMLSegmentationFileExportWidget);
  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationFileExportWidget::segmentationNodeID()
{
  Q_D(qMRMLSegmentationFileExportWidget);
  return (d->SegmentationNode.GetPointer() ? d->SegmentationNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setSegmentationNode(vtkMRMLSegmentationNode* node)
{
  Q_D(qMRMLSegmentationFileExportWidget);

  qvtkReconnect(d->SegmentationNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->SegmentationNode, node, vtkMRMLSegmentationNode::ReferenceImageGeometryChangedEvent, this,
    SLOT(onSegmentationReferenceImageGeometryChanged()));

  bool nodeChanged = d->SegmentationNode != node;

  d->SegmentationNode = node;
  this->setEnabled(d->SegmentationNode.GetPointer() != nullptr);
  this->updateWidgetFromMRML();

  // Get reference volume node
  if (nodeChanged)
    {
    this->onSegmentationReferenceImageGeometryChanged();
    }

}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setSegmentationNode(vtkMRMLNode* node)
{
  this->setSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSegmentationFileExportWidget);
  vtkMRMLColorTableNode* exportColorTableNode = nullptr;
  if (d->SegmentationNode)
    {
    exportColorTableNode = d->SegmentationNode->GetLabelmapConversionColorTableNode();
    }
  d->ColorTableNodeSelector->setCurrentNode(exportColorTableNode);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::updateWidgetFromSettings()
{
  Q_D(qMRMLSegmentationFileExportWidget);

  if (d->SettingsKey.isEmpty())
    {
    return;
    }

  QSettings settings;

  QString fileFormat = settings.value(d->SettingsKey + "/FileFormat", "STL").toString();
  d->FileFormatComboBox->setCurrentIndex(d->FileFormatComboBox->findText(fileFormat));
  this->setFileFormat(fileFormat);

  QString path = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePath(settings.value(d->SettingsKey + "/DestinationFolder", ".").toString());
  d->DestinationFolderButton->setDirectory(path);
  d->VisibleSegmentsOnlyCheckBox->setChecked(settings.value(d->SettingsKey + "/VisibleSegmentsOnly", false).toBool());

  d->MergeIntoSingleSTLFileCheckBox->setChecked(settings.value(d->SettingsKey + "/MergeIntoSingleFile", false).toBool());
  d->MergeIntoSingleOBJFileCheckBox->setChecked(settings.value(d->SettingsKey + "/MergeIntoSingleFile", false).toBool());

  d->SizeScaleSpinBox->setValue(settings.value(d->SettingsKey + "/SizeScale", 1.0).toDouble());
  d->ShowDestinationFolderOnExportCompleteCheckBox->setChecked(settings.value(d->SettingsKey + "/ShowDestinationFolderOnExportComplete", true).toBool());

  QString coordinateSystem = settings.value(d->SettingsKey + "/CoordinateSystem", "LPS").toString();
  d->CoordinateSystemComboBox->setCurrentIndex(d->CoordinateSystemComboBox->findText(coordinateSystem));
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::updateSettingsFromWidget()
{
  Q_D(qMRMLSegmentationFileExportWidget);

  if (d->SettingsKey.isEmpty())
    {
    return;
    }

  QSettings settings;

  settings.setValue(d->SettingsKey + "/FileFormat", d->FileFormatComboBox->currentText());
  QString path = qSlicerCoreApplication::application()->toSlicerHomeRelativePath(d->DestinationFolderButton->directory());
  settings.setValue(d->SettingsKey + "/DestinationFolder", path);
  settings.setValue(d->SettingsKey + "/VisibleSegmentsOnly", d->VisibleSegmentsOnlyCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/MergeIntoSingleFile", d->MergeIntoSingleSTLFileCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/SizeScale", d->SizeScaleSpinBox->value());
  settings.setValue(d->SettingsKey + "/ShowDestinationFolderOnExportComplete", d->ShowDestinationFolderOnExportCompleteCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/CoordinateSystem", d->CoordinateSystemComboBox->currentText());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::onSegmentationReferenceImageGeometryChanged()
{
  Q_D(qMRMLSegmentationFileExportWidget);
  if (!d->SegmentationNode)
    {
    return;
    }

  vtkMRMLNode* referenceVolumeNode = d->SegmentationNode->GetNodeReference(
    vtkMRMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str());
  d->ReferenceVolumeComboBox->setCurrentNode(referenceVolumeNode);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::exportToFiles()
{
  Q_D(qMRMLSegmentationFileExportWidget);

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  this->updateSettingsFromWidget();

  vtkSmartPointer<vtkStringArray> segmentIds;
  if (d->VisibleSegmentsOnlyCheckBox->isChecked()
    && d->SegmentationNode != nullptr
    && vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode()) != nullptr)
    {
    segmentIds = vtkSmartPointer<vtkStringArray>::New();
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    displayNode->GetVisibleSegmentIDs(segmentIds);
    }

  bool merge = d->MergeIntoSingleSTLFileCheckBox->isChecked(); // merge is only used for STL format

  QString fileFormat = d->FileFormatComboBox->currentText();
  if (fileFormat.toUpper() == "STL" || fileFormat.toUpper() == "OBJ")
    {
    vtkSlicerSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToFiles(
      d->DestinationFolderButton->directory().toUtf8().constData(),
      d->SegmentationNode.GetPointer(),
      segmentIds.GetPointer(),
      d->FileFormatComboBox->currentText().toUtf8().constData(),
      d->CoordinateSystemComboBox->currentText() == "LPS",
      d->SizeScaleSpinBox->value(),
      merge);
    }
  else if (fileFormat.toUpper() == "NRRD" || fileFormat.toUpper() == "NIFTI")
    {
    std::string extension = fileFormat.toUtf8().constData();
    if (fileFormat == "NIFTI")
      {
      extension = "nii";
      if (d->UseCompressionCheckBox->isChecked())
        {
        extension = "nii.gz";
        }
      }

    vtkMRMLColorTableNode* labelmapConversionColorTableNode = nullptr;
    if (d->UseColorTableValuesCheckBox->isChecked())
      {
      labelmapConversionColorTableNode = vtkMRMLColorTableNode::SafeDownCast(d->ColorTableNodeSelector->currentNode());
      }

    vtkMRMLVolumeNode* referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ReferenceVolumeComboBox->currentNode());
    if (referenceVolumeNode && d->SegmentationNode &&
      vtkSlicerSegmentationsModuleLogic::IsEffectiveExentOutsideReferenceVolume(
        referenceVolumeNode, d->SegmentationNode, segmentIds))
      {
      ctkMessageBox* exportWarningMesssgeBox = new ctkMessageBox(this);
      exportWarningMesssgeBox->setAttribute(Qt::WA_DeleteOnClose);
      exportWarningMesssgeBox->setWindowTitle("Exporting may erase data");
      exportWarningMesssgeBox->addButton(QMessageBox::StandardButton::Ok);
      exportWarningMesssgeBox->addButton(QMessageBox::StandardButton::Cancel);
      exportWarningMesssgeBox->setDontShowAgainVisible(true);
      exportWarningMesssgeBox->setIcon(QMessageBox::Warning);
      exportWarningMesssgeBox->setDontShowAgainSettingsKey("Segmentations/AlwaysCropDuringSegmentationFileExport");
      exportWarningMesssgeBox->setText("The current segmentation does not completely fit into the new geometry.\n"
                                       "Do you want to crop the segmentation?\n");
      if (exportWarningMesssgeBox->exec() != QMessageBox::StandardButton::Ok)
        {
        return;
        }
      }

    vtkSlicerSegmentationsModuleLogic::ExportSegmentsBinaryLabelmapRepresentationToFiles(
      d->DestinationFolderButton->directory().toUtf8().constData(),
      d->SegmentationNode.GetPointer(),
      segmentIds.GetPointer(),
      extension,
      d->UseCompressionCheckBox->isChecked(),
      referenceVolumeNode,
      vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_AND_REFERENCE_GEOMETRY,
      labelmapConversionColorTableNode);
    }

  QApplication::restoreOverrideCursor();

  if (d->ShowDestinationFolderOnExportCompleteCheckBox->isChecked())
    {
    this->showDestinationFolder();
    }

  emit exportToFilesDone();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::showDestinationFolder()
{
  Q_D(qMRMLSegmentationFileExportWidget);
  QDesktopServices::openUrl(QUrl("file:///" + d->DestinationFolderButton->directory(), QUrl::TolerantMode));
}

//------------------------------------------------------------------------------
QString qMRMLSegmentationFileExportWidget::settingsKey()const
{
  Q_D(const qMRMLSegmentationFileExportWidget);
  return d->SettingsKey;
}

//------------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setSettingsKey(const QString& key)
{
  Q_D(qMRMLSegmentationFileExportWidget);
  d->SettingsKey = key;
  this->updateWidgetFromSettings();
}

//------------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setFileFormat(const QString& formatStr)
{
  Q_D(qMRMLSegmentationFileExportWidget);

  bool formatIsSTL = formatStr == "STL";
  bool formatIsModel = formatIsSTL || formatStr == "OBJ";

  d->MergeIntoSingleSTLFileCheckBox->setVisible(formatIsSTL);
  d->MergeIntoSingleSTLFileCheckBox->setEnabled(formatIsModel);

  d->MergeIntoSingleOBJFileCheckBox->setVisible(!formatIsSTL);
  d->MergeIntoSingleOBJFileCheckBox->setEnabled(formatIsModel);

  d->CoordinateSystemComboBox->setEnabled(formatIsModel);
  d->ReferenceVolumeComboBox->setEnabled(!formatIsModel);
  d->SizeScaleSpinBox->setEnabled(formatIsModel);
  d->UseCompressionCheckBox->setEnabled(!formatIsModel);
  d->UseColorTableValuesCheckBox->setEnabled(!formatIsModel);
  d->ColorTableNodeSelector->setEnabled(!formatIsModel && d->UseColorTableValuesCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setUseLabelsFromColorNode(bool useColorNode)
{
  Q_UNUSED(useColorNode);
  Q_D(qMRMLSegmentationFileExportWidget);
  d->ColorTableNodeSelector->setEnabled(d->UseColorTableValuesCheckBox->isEnabled() && d->UseColorTableValuesCheckBox->isChecked());
}

//------------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setColorNodeID(const QString& id)
{
  Q_D(qMRMLSegmentationFileExportWidget);
  if (!d->SegmentationNode)
  {
    return;
  }
  std::string nodeID = id.toStdString();
  d->SegmentationNode->SetLabelmapConversionColorTableNodeID(nodeID.c_str());
}
