/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes

// CTK includes
//#include <ctkModelTester.h>

#include "qSlicerTractographyDisplayModuleWidget.h"
#include "ui_qSlicerTractographyDisplayModule.h"
#include <QMessageBox>

// VTK includes
#include "vtkPolyData.h"

// MRML includes

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerTractographyDisplayModuleWidgetPrivate: public Ui_qSlicerTractographyDisplayModule
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayModuleWidget);

protected:
  qSlicerTractographyDisplayModuleWidget* const q_ptr;

public:
  qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object);
  void init();

  vtkMRMLFiberBundleNode* FiberBundleNode;
  vtkMRMLAnnotationNode* AnnotationMRMLNodeForFiberSelection;
  double PercentageOfFibersShown;
};

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidgetPrivate
::qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object)
  :q_ptr(&object)
{
  this->FiberBundleNode = NULL;
  this->AnnotationMRMLNodeForFiberSelection = NULL;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModuleWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyDisplayModuleWidget);

  this->setupUi(q);

  QObject::connect(this->percentageOfFibersShown, SIGNAL(valueChanged(double)),
                   q, SLOT(setPercentageOfFibersShown(double)));
  QObject::connect(q, SIGNAL(percentageOfFibersShownChanged(double)),
                   this->percentageOfFibersShown, SLOT(setValue(double)));
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


//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidget::qSlicerTractographyDisplayModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyDisplayModuleWidgetPrivate(*this))
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidget::~qSlicerTractographyDisplayModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModuleWidget::setup()
{
//  Q_D(qSlicerTractographyDisplayModuleWidget);
//  d->setupUi(this);
}

void qSlicerTractographyDisplayModuleWidget::setFiberBundleNode(vtkMRMLNode* inputNode)
{
  this->setFiberBundleNode(vtkMRMLFiberBundleNode::SafeDownCast(inputNode));
}

void qSlicerTractographyDisplayModuleWidget::setFiberBundleNode(vtkMRMLFiberBundleNode* FiberBundleNode)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  if (vtkMRMLFiberBundleNode::SafeDownCast(FiberBundleNode))
    {
    d->FiberBundleNode = FiberBundleNode;
    d->LineDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetLineDisplayNode());
    d->TubeDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetTubeDisplayNode());
    d->GlyphDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    d->GlyphPropertiesWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    d->PercentageOfFibersShown = FiberBundleNode->GetSubsamplingRatio() * 100.;

    vtkMRMLFiberBundleDisplayNode *FiberBundleDisplayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast(FiberBundleNode->GetLineDisplayNode());
    if (FiberBundleDisplayNode)
    {
      d->AnnotationMRMLNodeForFiberSelection = FiberBundleNode->GetAnnotationNode();
      d->ROIForFiberSelectionMRMLNodeSelector->setCurrentNode(d->AnnotationMRMLNodeForFiberSelection);
      if (!FiberBundleNode->GetSelectWithAnnotationNode())
      {
        d->DisableROI->setChecked(true);
      }
      else if (FiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::PositiveAnnotationNodeSelection)
      {
        d->PositiveROI->setChecked(true);
      }
      else if (FiberBundleNode->GetSelectionWithAnnotationNodeMode() == vtkMRMLFiberBundleNode::NegativeAnnotationNodeSelection)
      {
        d->NegativeROI->setChecked(true);
      }

    }

    emit percentageOfFibersShownChanged(d->PercentageOfFibersShown);
    }
}


void qSlicerTractographyDisplayModuleWidget::setAnnotationMRMLNodeForFiberSelection(vtkMRMLNode* AnnotationMRMLNodeForFiberSelection)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
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

void qSlicerTractographyDisplayModuleWidget::setAnnotationROIMRMLNodeToFiberBundleEnvelope(vtkMRMLNode* AnnotationMRMLNodeForFiberSelection)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  if (d->FiberBundleNode)
  {
    vtkMRMLAnnotationROINode* AnnotationNode = vtkMRMLAnnotationROINode::SafeDownCast(AnnotationMRMLNodeForFiberSelection);
    if (AnnotationNode)
      {
      double xyz[3];
      double bounds[6];
      double radius[3];
      vtkPolyData *PolyData = d->FiberBundleNode->GetFilteredPolyData();

      PolyData->ComputeBounds();
      PolyData->GetCenter(xyz);
      PolyData->GetBounds(bounds);
      radius[0] = bounds[1] - bounds[0];
      radius[1] = bounds[3] - bounds[2];
      radius[2] = bounds[5] - bounds[4];

      AnnotationNode->SetXYZ(xyz);
      AnnotationNode->SetRadiusXYZ(radius);
      AnnotationNode->UpdateReferences();

      d->ROIForFiberSelectionMRMLNodeSelector->setCurrentNode(AnnotationNode);
      }
  }
}

void qSlicerTractographyDisplayModuleWidget::disableROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyDisplayModuleWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOff();
    }
  }
}

void qSlicerTractographyDisplayModuleWidget::positiveROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyDisplayModuleWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOn();
      d->FiberBundleNode->SetSelectionWithAnnotationNodeModeToPositive();
    }
  }
}

void qSlicerTractographyDisplayModuleWidget::negativeROISelection(bool arg)
{
  if (arg)
  {
    Q_D(qSlicerTractographyDisplayModuleWidget);
    if (d->FiberBundleNode && d->AnnotationMRMLNodeForFiberSelection)
    {
      d->FiberBundleNode->SelectWithAnnotationNodeOn();
      d->FiberBundleNode->SetSelectionWithAnnotationNodeModeToNegative();
    }
  }
}


void qSlicerTractographyDisplayModuleWidget::createNewBundleFromSelection()
{
  Q_D(qSlicerTractographyDisplayModuleWidget);

  vtkMRMLFiberBundleNode *fiberBundleFromSelection = vtkMRMLFiberBundleNode::SafeDownCast(d->FiberBundleFromSelection->currentNode());
  if (d->FiberBundleNode && fiberBundleFromSelection && (d->FiberBundleNode != fiberBundleFromSelection))
  {
    d->FiberBundleNode->GetScene()->SaveStateForUndo();
    vtkPolyData *FilteredPolyData = vtkPolyData::New();
    FilteredPolyData->DeepCopy(d->FiberBundleNode->GetFilteredPolyData());
    fiberBundleFromSelection->SetAndObservePolyData(FilteredPolyData);
    FilteredPolyData->Delete();

    if (!fiberBundleFromSelection->GetDisplayNode())
    {
      fiberBundleFromSelection->CreateDefaultDisplayNodes();

      if (fiberBundleFromSelection->GetStorageNode() == NULL) 
        {
          fiberBundleFromSelection->CreateDefaultStorageNode();
        }
      
      fiberBundleFromSelection->SetModifiedSinceRead(1);
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

void qSlicerTractographyDisplayModuleWidget::updateBundleFromSelection()
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
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
      FilteredPolyData->DeepCopy(d->FiberBundleNode->GetFilteredPolyData());
      d->FiberBundleNode->SetAndObservePolyData(FilteredPolyData);
      FilteredPolyData->Delete();
      this->setPercentageOfFibersShown(100.);
    }
}


void qSlicerTractographyDisplayModuleWidget::setPercentageOfFibersShown(double percentage)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  if (vtkMRMLFiberBundleNode::SafeDownCast(d->FiberBundleNode))
    {
    d->PercentageOfFibersShown = percentage;
    d->FiberBundleNode->SetSubsamplingRatio(d->PercentageOfFibersShown / 100.);
    emit percentageOfFibersShownChanged(d->PercentageOfFibersShown);
    }
}
