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

// VTK includes
#include <vtkStringArray.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>

// Slicer includes
#include <vtkMRMLSliceLogic.h>
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
  d->SegmentationNode = node;
  this->setEnabled(d->SegmentationNode.GetPointer() != nullptr);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationFileExportWidget::setSegmentationNode(vtkMRMLNode* node)
{
  this->setSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
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

  d->DestinationFolderButton->setDirectory(settings.value(d->SettingsKey + "/DestinationFolder", ".").toString());
  d->VisibleSegmentsOnlyCheckBox->setChecked(settings.value(d->SettingsKey + "/VisibleSegmentsOnly", false).toBool());
  d->MergeIntoSingleFileCheckBox->setChecked(settings.value(d->SettingsKey + "/MergeIntoSingleFile", false).toBool());
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
  settings.setValue(d->SettingsKey + "/DestinationFolder", d->DestinationFolderButton->directory());
  settings.setValue(d->SettingsKey + "/VisibleSegmentsOnly", d->VisibleSegmentsOnlyCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/MergeIntoSingleFile", d->MergeIntoSingleFileCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/SizeScale", d->SizeScaleSpinBox->value());
  settings.setValue(d->SettingsKey + "/ShowDestinationFolderOnExportComplete", d->ShowDestinationFolderOnExportCompleteCheckBox->isChecked());
  settings.setValue(d->SettingsKey + "/CoordinateSystem", d->CoordinateSystemComboBox->currentText());
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

  vtkSlicerSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToFiles(
    d->DestinationFolderButton->directory().toUtf8().constData(),
    d->SegmentationNode.GetPointer(),
    segmentIds.GetPointer(),
    d->FileFormatComboBox->currentText().toUtf8().constData(),
    d->CoordinateSystemComboBox->currentText() == "LPS",
    d->SizeScaleSpinBox->value(),
    d->MergeIntoSingleFileCheckBox->isChecked());

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
  d->MergeIntoSingleFileCheckBox->setEnabled(formatStr == "STL");
}
