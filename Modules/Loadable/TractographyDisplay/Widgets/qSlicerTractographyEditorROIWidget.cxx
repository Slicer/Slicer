// QT includes
#include <QDebug>
#include <QColor>
#include <QMessageBox>

// qMRML includes
#include "qSlicerTractographyEditorROIWidget.h"
#include "ui_qSlicerTractographyEditorROIWidget.h"

// MRML includes
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleDisplayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include "vtkPolyData.h"
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qSlicerTractographyEditorROIWidgetPrivate: 
  public Ui_qSlicerTractographyEditorROIWidget
{
  Q_DECLARE_PUBLIC(qSlicerTractographyEditorROIWidget);

protected:
  qSlicerTractographyEditorROIWidget* const q_ptr;

public:
  qSlicerTractographyEditorROIWidgetPrivate(qSlicerTractographyEditorROIWidget& object);
  void init();

  vtkMRMLFiberBundleNode* FiberBundleNode;
  vtkMRMLAnnotationNode* AnnotationMRMLNodeForFiberSelection;
};

//------------------------------------------------------------------------------
qSlicerTractographyEditorROIWidgetPrivate::qSlicerTractographyEditorROIWidgetPrivate
                                      (qSlicerTractographyEditorROIWidget& object)
  : q_ptr(&object)
{
  this->FiberBundleNode = 0;
  this->AnnotationMRMLNodeForFiberSelection = NULL;
}

//------------------------------------------------------------------------------
void qSlicerTractographyEditorROIWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyEditorROIWidget);
  this->setupUi(q);

  QObject::connect(this->ROIForFiberSelectionMRMLNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setAnnotationMRMLNodeForFiberSelection(vtkMRMLNode*)));
  QObject::connect(this->ROIForFiberSelectionMRMLNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                   q, SLOT(setAnnotationROIMRMLNodeToFiberBundleEnvelope(vtkMRMLNode*)));
  QObject::connect(this->CreateNewFiberBundle, SIGNAL(clicked()),
                   q, SLOT(createNewBundleFromSelection()));
  QObject::connect(this->UpdateBundleFromSelection, SIGNAL(clicked()),
                   q, SLOT(updateBundleFromSelection()));
  QObject::connect(this->DisableROI, SIGNAL(toggled(bool)),
                   q, SLOT(disableROISelection(bool)));
  QObject::connect(this->PositiveROI, SIGNAL(toggled(bool)),
                   q, SLOT(positiveROISelection(bool)));
  QObject::connect(this->NegativeROI, SIGNAL(toggled(bool)),
                   q, SLOT(negativeROISelection(bool)));
}


//------------------------------------------------------------------------------
qSlicerTractographyEditorROIWidget::qSlicerTractographyEditorROIWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyEditorROIWidgetPrivate(*this))
{
  Q_D(qSlicerTractographyEditorROIWidget);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerTractographyEditorROIWidget::~qSlicerTractographyEditorROIWidget()
{
}

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* qSlicerTractographyEditorROIWidget::fiberBundleNode()const
{
  Q_D(const qSlicerTractographyEditorROIWidget);
  return d->FiberBundleNode;
}

//------------------------------------------------------------------------------
void qSlicerTractographyEditorROIWidget::setFiberBundleNode(vtkMRMLNode* node)
{
  this->setFiberBundleNode(vtkMRMLFiberBundleNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qSlicerTractographyEditorROIWidget::
  setFiberBundleNode
  (vtkMRMLFiberBundleNode* fiberBundleNode)
{
  Q_D(qSlicerTractographyEditorROIWidget);

  if (d->FiberBundleNode == fiberBundleNode)
    return;

  d->FiberBundleNode = fiberBundleNode;

  // TODO: move this to updateWidgetFromMRML section.
  if (!fiberBundleNode)
  {
    return;
  }

  if (fiberBundleNode->GetNumberOfDisplayNodes() > 1)
  {
    d->AnnotationMRMLNodeForFiberSelection = fiberBundleNode->GetAnnotationNode();
    d->ROIForFiberSelectionMRMLNodeSelector->setCurrentNode(d->AnnotationMRMLNodeForFiberSelection);
    if (!fiberBundleNode->GetSelectWithAnnotationNode())
    {
      d->DisableROI->setChecked(true);
    }
    else if (fiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection)
    {
      d->PositiveROI->setChecked(true);
    }
    else if (fiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::NegativeAnnotationNodeSelection)
    {
      d->NegativeROI->setChecked(true);
    }

  }

//  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerTractographyEditorROIWidget::
  updateWidgetFromMRML()
{
  Q_D(qSlicerTractographyEditorROIWidget);
  if ( !d->FiberBundleNode )
    {
    return;
    }

  if (d->FiberBundleNode->GetNumberOfDisplayNodes() > 1)
  {
    if (d->AnnotationMRMLNodeForFiberSelection != d->FiberBundleNode->GetAnnotationNode())
    {
      d->AnnotationMRMLNodeForFiberSelection = d->FiberBundleNode->GetAnnotationNode();
      d->ROIForFiberSelectionMRMLNodeSelector->setCurrentNode(d->AnnotationMRMLNodeForFiberSelection);
    }
    if (!d->FiberBundleNode->GetSelectWithAnnotationNode())
    {
      d->DisableROI->setChecked(true);
    }
    else if (d->FiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection)
    {
      d->PositiveROI->setChecked(true);
    }
    else if (d->FiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::NegativeAnnotationNodeSelection)
    {
      d->NegativeROI->setChecked(true);
    }
  }
}

void qSlicerTractographyEditorROIWidget::setAnnotationMRMLNodeForFiberSelection(vtkMRMLNode* AnnotationMRMLNodeForFiberSelection)
{
  Q_D(qSlicerTractographyEditorROIWidget);
  if (d->FiberBundleNode)
  {
    vtkMRMLAnnotationNode* AnnotationNode = vtkMRMLAnnotationNode::SafeDownCast(AnnotationMRMLNodeForFiberSelection);
    if (AnnotationNode)
      {
      d->AnnotationMRMLNodeForFiberSelection = AnnotationNode;
      d->FiberBundleNode->SetAndObserveAnnotationNodeID(d->AnnotationMRMLNodeForFiberSelection->GetID());
      }
  }
}

void qSlicerTractographyEditorROIWidget::setAnnotationROIMRMLNodeToFiberBundleEnvelope(vtkMRMLNode* AnnotationMRMLNodeForFiberSelection)
{
  Q_D(qSlicerTractographyEditorROIWidget);
  if (d->FiberBundleNode)
  {
    vtkMRMLAnnotationROINode* AnnotationNode = vtkMRMLAnnotationROINode::SafeDownCast(AnnotationMRMLNodeForFiberSelection);
    if (AnnotationNode)
      {
      double xyz[3];
      double bounds[6];
      double radius[3];
      vtkPolyData *PolyData = d->FiberBundleNode->GetPolyData();

      PolyData->ComputeBounds();
      PolyData->GetCenter(xyz);
      PolyData->GetBounds(bounds);

      radius[0] = (bounds[1] - bounds[0]) / 2.;
      radius[1] = (bounds[3] - bounds[2]) / 2.;
      radius[2] = (bounds[5] - bounds[4]) / 2.;

      AnnotationNode->SetXYZ(xyz);
      AnnotationNode->SetRadiusXYZ(radius);
      AnnotationNode->UpdateReferences();

      d->ROIForFiberSelectionMRMLNodeSelector->setCurrentNode(AnnotationNode);

      d->PositiveROI->setChecked(true);
      }
  }
}

void qSlicerTractographyEditorROIWidget::disableROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyEditorROIWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOff();
    }
  }
}

void qSlicerTractographyEditorROIWidget::positiveROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyEditorROIWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOn();
      d->FiberBundleNode->SetSelectionWithAnnotationNodeModeToPositive();
    }
  }
}

void qSlicerTractographyEditorROIWidget::negativeROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyEditorROIWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOn();
      d->FiberBundleNode->SetSelectionWithAnnotationNodeModeToNegative();
    }
  }
}


void qSlicerTractographyEditorROIWidget::createNewBundleFromSelection()
{
  Q_D(qSlicerTractographyEditorROIWidget);

  vtkMRMLFiberBundleNode *fiberBundleFromSelection = vtkMRMLFiberBundleNode::SafeDownCast(d->FiberBundleFromSelection->currentNode());
  if (d->FiberBundleNode && fiberBundleFromSelection && (d->FiberBundleNode != fiberBundleFromSelection))
  {
    if (mrmlScene()) mrmlScene()->SaveStateForUndo();
    vtkPolyData *FilteredPolyData = vtkPolyData::New();
    FilteredPolyData->DeepCopy(d->FiberBundleNode->GetPolyData());
    fiberBundleFromSelection->SetAndObservePolyData(FilteredPolyData);
    FilteredPolyData->Delete();

    if (!fiberBundleFromSelection->GetDisplayNode())
    {
      fiberBundleFromSelection->CreateDefaultDisplayNodes();

      if (fiberBundleFromSelection->GetStorageNode() == NULL) 
        {
          fiberBundleFromSelection->CreateDefaultStorageNode();
        }
      
      fiberBundleFromSelection->SetAndObserveTransformNodeID(d->FiberBundleNode->GetTransformNodeID());
      fiberBundleFromSelection->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
    }

  } else {
     QMessageBox::warning(this, tr("Create Bundle From ROI"),
                                  tr("You can not use the source Fiber Bundle\n"
                                     "as destination fiber bundle.\n"
                                     "Use Update Bundle From ROI for this."
                                     ),
                                  QMessageBox::Ok);

  }
}

void qSlicerTractographyEditorROIWidget::updateBundleFromSelection()
{
  Q_D(qSlicerTractographyEditorROIWidget);
  int proceedWithUpdate = 0;

  if (d->FiberBundleNode)
  {
    if (d->ConfirmFiberBundleUpdate->checkState() != Qt::Checked)
    {
     int ret = QMessageBox::warning(this, tr("Update Bundle From ROI"),
                                    tr("This will replace the actual fiber bundle\n"
                                       "with the results of the selection.\n"
                                       "Are you sure this is what you want?"
                                       ),
                                    QMessageBox::Ok | QMessageBox::Cancel);
     if (ret == QMessageBox::Ok)
        proceedWithUpdate = 1;
     } else {
        proceedWithUpdate = 0;
     }
    }

    if (proceedWithUpdate || (d->ConfirmFiberBundleUpdate->checkState() == Qt::Checked))
    {
      d->FiberBundleNode->GetScene()->SaveStateForUndo();
      vtkPolyData *FilteredPolyData = vtkPolyData::New();
      FilteredPolyData->DeepCopy(d->FiberBundleNode->GetPolyData());
      d->FiberBundleNode->SetAndObservePolyData(FilteredPolyData);
      FilteredPolyData->Delete();
      d->FiberBundleNode->SetSubsamplingRatio(1);
    }
}

