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
#include "ui_qSlicerTractographyDisplayModuleWidget.h"
#include "qMRMLSceneTractographyDisplayModel.h"
// MRML includes

#include "vtkMRMLNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyDisplay
class qSlicerTractographyDisplayModuleWidgetPrivate: public Ui_qSlicerTractographyDisplayModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayModuleWidget);

protected:
  qSlicerTractographyDisplayModuleWidget* const q_ptr;

public:
  qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object);
  void init();

  vtkMRMLFiberBundleNode* fiberBundleNode;
  double PercentageOfFibersShown;
};

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidgetPrivate
::qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object)
  :q_ptr(&object)
{
  this->fiberBundleNode = NULL;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModuleWidgetPrivate::init()
{
  Q_Q(qSlicerTractographyDisplayModuleWidget);

  this->setupUi(q);

  this->percentageOfFibersShown->setTracking(false);

  QObject::connect(this->percentageOfFibersShown, SIGNAL(valueChanged(double)),
                   q, SLOT(setPercentageOfFibersShown(double)));
  QObject::connect(q, SIGNAL(percentageOfFibersShownChanged(double)),
                   this->percentageOfFibersShown, SLOT(setValue(double)));
  QObject::connect(this->SolidTubeColorCheckbox, SIGNAL(clicked(bool)),
                   q, SLOT(setSolidTubeColor(bool)));

  QObject::connect(this->TractographyDisplayTreeView, SIGNAL(visibilityChanged(int)),
                   this->TractDisplayModesTabWidget, SLOT(setCurrentIndex (int)));

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

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModuleWidget::exit()
{
  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(
        this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
  {
    interactionNode->SetEnableFiberEdit(0);
  }

  this->Superclass::exit();
}


void qSlicerTractographyDisplayModuleWidget::setFiberBundleNode(vtkMRMLNode* inputNode)
{
  this->setFiberBundleNode(vtkMRMLFiberBundleNode::SafeDownCast(inputNode));
}

void qSlicerTractographyDisplayModuleWidget::setFiberBundleNode(vtkMRMLFiberBundleNode* fiberBundleNode)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);

  if (d->fiberBundleNode == fiberBundleNode)
    return;

  d->fiberBundleNode = fiberBundleNode;


  d->LineDisplayWidget->setFiberBundleNode(fiberBundleNode);
  d->TubeDisplayWidget->setFiberBundleNode(fiberBundleNode);
  d->GlyphDisplayWidget->setFiberBundleNode(fiberBundleNode);

  if (fiberBundleNode)
  {
    d->LineDisplayWidget->setFiberBundleDisplayNode(fiberBundleNode->GetLineDisplayNode());
    d->TubeDisplayWidget->setFiberBundleDisplayNode(fiberBundleNode->GetTubeDisplayNode());
    d->GlyphDisplayWidget->setFiberBundleDisplayNode(fiberBundleNode->GetGlyphDisplayNode());
    d->GlyphPropertiesWidget->setFiberBundleDisplayNode(fiberBundleNode->GetGlyphDisplayNode());
    d->PercentageOfFibersShown = fiberBundleNode->GetSubsamplingRatio() * 100.;
  }

  emit currentNodeChanged(d->fiberBundleNode);
  emit percentageOfFibersShownChanged(d->PercentageOfFibersShown);
}

void qSlicerTractographyDisplayModuleWidget::setPercentageOfFibersShown(double percentage)
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  if (vtkMRMLFiberBundleNode::SafeDownCast(d->fiberBundleNode))
    {
    d->PercentageOfFibersShown = percentage;
    d->fiberBundleNode->SetSubsamplingRatio(d->PercentageOfFibersShown / 100.);
    emit percentageOfFibersShownChanged(d->PercentageOfFibersShown);
    }
}
void qSlicerTractographyDisplayModuleWidget::setSolidTubeColor(bool solid)
{
  std::vector<vtkMRMLNode *> nodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLFiberBundleTubeDisplayNode", nodes);

  vtkMRMLFiberBundleTubeDisplayNode *node = 0;
  for (unsigned int i=0; i<nodes.size(); i++)
    {
    node = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(nodes[i]);
    if (solid)
      {
      node->SetColorMode(vtkMRMLFiberBundleDisplayNode::colorModeSolid);
      }
    else
      {
      node->SetColorMode(vtkMRMLFiberBundleDisplayNode::colorModeScalar);
      }
    }

}

