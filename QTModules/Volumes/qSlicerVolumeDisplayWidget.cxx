// QT includes
#include <QDebug>

// SlicerQT includes
#include "qSlicerVolumeDisplayWidget.h"
#include "qSlicerLabelMapVolumeDisplayWidget.h"
#include "qSlicerScalarVolumeDisplayWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVolumeNode.h"

// --------------------------------------------------------------------------
qSlicerVolumeDisplayWidget::qSlicerVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  this->ScalarVolumeDisplayWidget = new qSlicerScalarVolumeDisplayWidget(this);
  this->addWidget(this->ScalarVolumeDisplayWidget);

  this->LabelMapVolumeDisplayWidget = new qSlicerLabelMapVolumeDisplayWidget(this);
  this->addWidget(this->LabelMapVolumeDisplayWidget);
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  if (!volumeNode)
    {
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    return;
    }

  vtkMRMLScene* scene = volumeNode->GetScene();
  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
  if (scalarVolumeNode && !scalarVolumeNode->GetLabelMap())
    {
    this->ScalarVolumeDisplayWidget->setMRMLScene(scene);
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    this->setCurrentWidget(this->ScalarVolumeDisplayWidget);
    }
  else if (scalarVolumeNode && scalarVolumeNode->GetLabelMap())
    {
    this->LabelMapVolumeDisplayWidget->setMRMLScene(scene);
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    this->setCurrentWidget(this->LabelMapVolumeDisplayWidget);
    }
}
