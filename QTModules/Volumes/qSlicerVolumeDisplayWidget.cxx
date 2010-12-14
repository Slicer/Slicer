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
  qvtkDisconnect(0,vtkCommand::ModifiedEvent,this,SLOT(updateFromMRML(vtkObject*)));

  if (volumeNode == 0)
    {
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(
      vtkMRMLVolumeNode::SafeDownCast(0));
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(
      vtkMRMLVolumeNode::SafeDownCast(0));
    return;
    }
  else
    {
    qvtkConnect(volumeNode, vtkCommand::ModifiedEvent, this, SLOT(updateFromMRML(vtkObject*)));
    }

  vtkMRMLScene* scene = volumeNode->GetScene();
  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
  if (scalarVolumeNode && !scalarVolumeNode->GetLabelMap())
    {
    // disable other panels 
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(
      vtkMRMLVolumeNode::SafeDownCast(0));
    
    this->ScalarVolumeDisplayWidget->setMRMLScene(scene);
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    this->setCurrentWidget(this->ScalarVolumeDisplayWidget);
    }
  else if (scalarVolumeNode && scalarVolumeNode->GetLabelMap())
    {
    // disable other panels
    this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(
      vtkMRMLVolumeNode::SafeDownCast(0));
    
    this->LabelMapVolumeDisplayWidget->setMRMLScene(scene);
    this->LabelMapVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    this->setCurrentWidget(this->LabelMapVolumeDisplayWidget);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidget::updateFromMRML(vtkObject* volume)
{
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(volume);
  this->setMRMLVolumeNode(volumeNode);
}
