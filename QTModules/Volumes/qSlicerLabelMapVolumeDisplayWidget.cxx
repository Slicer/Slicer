#include "qSlicerLabelMapVolumeDisplayWidget.h"
#include "ui_qSlicerLabelMapVolumeDisplayWidget.h"

// Qt includes
#include <QDebug>

// MRML includes
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerLabelMapVolumeDisplayWidgetPrivate: public ctkPrivate<qSlicerLabelMapVolumeDisplayWidget>,
                                          public Ui_qSlicerLabelMapVolumeDisplayWidget
{
public:
  qSlicerLabelMapVolumeDisplayWidgetPrivate();
  ~qSlicerLabelMapVolumeDisplayWidgetPrivate();
  void init();

  vtkMRMLScalarVolumeNode* VolumeNode;
};

//-----------------------------------------------------------------------------
qSlicerLabelMapVolumeDisplayWidgetPrivate::qSlicerLabelMapVolumeDisplayWidgetPrivate()
{
  this->VolumeNode = 0;
}

//-----------------------------------------------------------------------------
qSlicerLabelMapVolumeDisplayWidgetPrivate::~qSlicerLabelMapVolumeDisplayWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerLabelMapVolumeDisplayWidgetPrivate::init()
{
  CTK_P(qSlicerLabelMapVolumeDisplayWidget);

  this->setupUi(p);
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   p, SLOT(setColorNode(vtkMRMLNode*)));
  // disable as there is not MRML Node associated with the widget
  p->setEnabled(false);
}

// --------------------------------------------------------------------------
qSlicerLabelMapVolumeDisplayWidget::qSlicerLabelMapVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerLabelMapVolumeDisplayWidget);
  CTK_D(qSlicerLabelMapVolumeDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerLabelMapVolumeDisplayWidget::volumeNode()const
{
  CTK_D(const qSlicerLabelMapVolumeDisplayWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLLabelMapVolumeDisplayNode* qSlicerLabelMapVolumeDisplayWidget::volumeDisplayNode()const
{
  CTK_D(const qSlicerLabelMapVolumeDisplayWidget);
  return d->VolumeNode ? vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(
    d->VolumeNode->GetDisplayNode()) : 0;
}

// --------------------------------------------------------------------------
void qSlicerLabelMapVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerLabelMapVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  CTK_D(qSlicerLabelMapVolumeDisplayWidget);
  vtkMRMLLabelMapVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  qvtkReconnect(oldVolumeDisplayNode, volumeNode->GetDisplayNode(), vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->VolumeNode = volumeNode;
  this->setEnabled(volumeNode != 0);
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerLabelMapVolumeDisplayWidget::updateWidgetFromMRML()
{
  CTK_D(qSlicerLabelMapVolumeDisplayWidget);
  vtkMRMLLabelMapVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
    {
    d->ColorTableComboBox->setCurrentNode(displayNode->GetColorNode());
    }
}

// --------------------------------------------------------------------------
void qSlicerLabelMapVolumeDisplayWidget::setColorNode(vtkMRMLNode* colorNode)
{
  vtkMRMLLabelMapVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode || !colorNode)
    {
    return;
    }
  Q_ASSERT(vtkMRMLColorNode::SafeDownCast(colorNode));
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}
