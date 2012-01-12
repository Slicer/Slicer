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

// MRML includes

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyDisplay
class qSlicerTractographyDisplayModuleWidgetPrivate: public Ui_qSlicerTractographyDisplayModule
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

  QObject::connect(this->percentageOfFibersShown, SIGNAL(valueChanged(double)),
                   q, SLOT(setPercentageOfFibersShown(double)));
  QObject::connect(q, SIGNAL(percentageOfFibersShownChanged(double)),
                   this->percentageOfFibersShown, SLOT(setValue(double)));
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
    d->fiberBundleNode = FiberBundleNode;

    d->LineDisplayWidget->setFiberBundleNode(FiberBundleNode);
    d->TubeDisplayWidget->setFiberBundleNode(FiberBundleNode);
    d->GlyphDisplayWidget->setFiberBundleNode(FiberBundleNode);

    d->LineDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetLineDisplayNode());
    d->TubeDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetTubeDisplayNode());
    d->GlyphDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    d->GlyphPropertiesWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    
    d->PercentageOfFibersShown = FiberBundleNode->GetSubsamplingRatio() * 100.;
    emit percentageOfFibersShownChanged(d->PercentageOfFibersShown);
    }
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
