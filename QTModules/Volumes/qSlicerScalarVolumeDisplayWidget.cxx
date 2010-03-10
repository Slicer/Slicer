#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

#include "vtkMRMLScalarVolumeNode.h"
// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerScalarVolumeDisplayWidgetPrivate: public qCTKPrivate<qSlicerScalarVolumeDisplayWidget>,
                                          public Ui_qSlicerScalarVolumeDisplayWidget
{
public:
};

// --------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidget::qSlicerScalarVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qSlicerScalarVolumeDisplayWidget);
  QCTK_D(qSlicerScalarVolumeDisplayWidget);
  
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
  QCTK_D(qSlicerScalarVolumeDisplayWidget);
  
  if (volumeNode) 
  {
    d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);

    this->setEnabled(true);
  }
}
