#include "qSlicerVolumeDisplayWidget.h"
#include "qSlicerScalarVolumeDisplayWidget.h"

#include "vtkMRMLVolumeNode.h"
// QT includes
#include <QDebug>

// --------------------------------------------------------------------------
qSlicerVolumeDisplayWidget::qSlicerVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  this->ScalarVolumeDisplayWidget = new qSlicerScalarVolumeDisplayWidget(this);

  this->addWidget(this->ScalarVolumeDisplayWidget);

}

// --------------------------------------------------------------------------
void qSlicerVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{  
  if (volumeNode) 
  {
    if (std::string(volumeNode->GetClassName()) == std::string("vtkMRMLScalarVolumeNode"))
    {
      this->setCurrentWidget(this->ScalarVolumeDisplayWidget);
      this->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);
    }
  }
}
