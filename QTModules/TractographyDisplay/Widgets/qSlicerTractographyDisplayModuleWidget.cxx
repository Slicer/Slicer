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
#include <QDebug>

// CTK includes
//#include <ctkModelTester.h>

#include "qSlicerTractographyDisplayModuleWidget.h"
#include "ui_qSlicerTractographyDisplayModule.h"

// MRML includes

#include "vtkMRMLFiberBundleNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerTractographyDisplayModuleWidgetPrivate: public Ui_qSlicerTractographyDisplayModule
{
  Q_DECLARE_PUBLIC(qSlicerTractographyDisplayModuleWidget);

protected:
  qSlicerTractographyDisplayModuleWidget* const q_ptr;

public:
  qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object);
  vtkMRMLFiberBundleNode* FiberBundleNode;
};

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidgetPrivate
::qSlicerTractographyDisplayModuleWidgetPrivate(qSlicerTractographyDisplayModuleWidget& object)
  :q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidget::qSlicerTractographyDisplayModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyDisplayModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModuleWidget::~qSlicerTractographyDisplayModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModuleWidget::setup()
{
  Q_D(qSlicerTractographyDisplayModuleWidget);
  d->setupUi(this);
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
    d->LineDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetLineDisplayNode());
    d->TubeDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetTubeDisplayNode());
    d->GlyphDisplayWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    d->GlyphPropertiesWidget->setFiberBundleDisplayNode(FiberBundleNode->GetGlyphDisplayNode());
    }
}
