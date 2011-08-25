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

// VTK includes
#include "vtkPolyData.h"

// MRML includes

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationROINode.h"

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
  QObject::connect(this->activateROISelectionCheckBox, SIGNAL(stateChanged(int)),
                   q, SLOT(filterByAnnotationNode(int)));

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
      if (FiberBundleNode->GetSelectWithAnnotationNode())
      {
        d->activateROISelectionCheckBox->setCheckState(Qt::Checked);
      }
      else
      {
        d->activateROISelectionCheckBox->setCheckState(Qt::Unchecked);
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
      vtkPolyData *PolyData = d->FiberBundleNode->GetPolyData();

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

void qSlicerTractographyDisplayModuleWidget::filterByAnnotationNode(int value)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  if (d->FiberBundleNode and d->AnnotationMRMLNodeForFiberSelection)
  {
    d->FiberBundleNode->SetSelectWithAnnotationNode(value == Qt::Checked);
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
