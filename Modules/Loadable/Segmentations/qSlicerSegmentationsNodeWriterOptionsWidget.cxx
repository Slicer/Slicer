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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

// Qt includes

// qSlicer includes
#include "qSlicerNodeWriterOptionsWidget_p.h"
#include "qSlicerSegmentationsNodeWriterOptionsWidget.h"

// MRML includes
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLSegmentationStorageNode.h>

//------------------------------------------------------------------------------
class qSlicerSegmentationsNodeWriterOptionsWidgetPrivate
  : public qSlicerNodeWriterOptionsWidgetPrivate
{
public:
  void setupUi(QWidget* widget) override;
  QCheckBox* UseReferenceGeometryCheckBox;
};

//------------------------------------------------------------------------------
void qSlicerSegmentationsNodeWriterOptionsWidgetPrivate::setupUi(QWidget* widget)
{
  this->qSlicerNodeWriterOptionsWidgetPrivate::setupUi(widget);
  this->UseReferenceGeometryCheckBox = new QCheckBox(widget);
  this->UseReferenceGeometryCheckBox->setObjectName(QStringLiteral("CropToMinimumExtentCheckBox"));
  this->UseReferenceGeometryCheckBox->setText("Crop to minimum extent");
  this->UseReferenceGeometryCheckBox->setToolTip("If enabled then segmentation labelmap representation is"
    " cropped to the minimum necessary size. This saves storage space but changes voxel coordinate system"
    " (physical coordinate system is not affected).");
  horizontalLayout->addWidget(UseReferenceGeometryCheckBox);
  QObject::connect(this->UseReferenceGeometryCheckBox, SIGNAL(toggled(bool)),
    widget, SLOT(setCropToMinimumExtent(bool)));
}

//------------------------------------------------------------------------------
qSlicerSegmentationsNodeWriterOptionsWidget::qSlicerSegmentationsNodeWriterOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerSegmentationsNodeWriterOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerSegmentationsNodeWriterOptionsWidget);
  d->setupUi(this);
}

//------------------------------------------------------------------------------
qSlicerSegmentationsNodeWriterOptionsWidget::~qSlicerSegmentationsNodeWriterOptionsWidget() = default;

//------------------------------------------------------------------------------
void qSlicerSegmentationsNodeWriterOptionsWidget::setObject(vtkObject* object)
{
  Q_D(qSlicerSegmentationsNodeWriterOptionsWidget);
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(object);
  if (storableNode)
    {
    vtkMRMLSegmentationStorageNode* storageNode = vtkMRMLSegmentationStorageNode::SafeDownCast(
      storableNode->GetStorageNode());
    if (storageNode)
      {
      d->UseReferenceGeometryCheckBox->setChecked(storageNode->GetCropToMinimumExtent());
      }
    }
  Superclass::setObject(object);
}

//------------------------------------------------------------------------------
void qSlicerSegmentationsNodeWriterOptionsWidget::setCropToMinimumExtent(bool crop)
{
  Q_D(qSlicerSegmentationsNodeWriterOptionsWidget);
  d->Properties["cropToMinimumExtent"] = crop;
}
