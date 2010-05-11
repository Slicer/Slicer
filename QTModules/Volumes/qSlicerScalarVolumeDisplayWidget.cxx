#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

// Qt includes
#include <QDebug>

#include "vtkMRMLScalarVolumeNode.h"

//-----------------------------------------------------------------------------
class qSlicerScalarVolumeDisplayWidgetPrivate: public ctkPrivate<qSlicerScalarVolumeDisplayWidget>,
                                          public Ui_qSlicerScalarVolumeDisplayWidget
{
public:
};

// --------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidget::qSlicerScalarVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerScalarVolumeDisplayWidget);
  CTK_D(qSlicerScalarVolumeDisplayWidget);
  
  d->setupUi(this);

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  CTK_D(qSlicerScalarVolumeDisplayWidget);
  
  if (volumeNode) 
  {
    d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);
    d->MRMLVolumeThresholdWidget->setMRMLVolumeNode(volumeNode);
    this->setEnabled(true);
  }
}
