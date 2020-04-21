/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

// qMRML includes
#include "qMRMLAnnotationFiducialProjectionPropertyWidget.h"
#include "ui_qMRMLAnnotationFiducialProjectionPropertyWidget.h"

// MRML includes
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLAnnotationPointDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate
  : public Ui_qMRMLAnnotationFiducialProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationFiducialProjectionPropertyWidget);
protected:
  qMRMLAnnotationFiducialProjectionPropertyWidget* const q_ptr;
public:
  qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate(qMRMLAnnotationFiducialProjectionPropertyWidget& object);
  void init();

  vtkMRMLAnnotationPointDisplayNode* FiducialDisplayNode;
};

//-----------------------------------------------------------------------------
// qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate
::qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate(qMRMLAnnotationFiducialProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->FiducialDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qMRMLAnnotationFiducialProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->Point2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->PointProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->PointUseFiducialColorCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setUseFiducialColor(bool)));
  QObject::connect(this->PointOutlinedBehindSlicePlaneCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setOutlinedBehindSlicePlane(bool)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLAnnotationFiducialProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLAnnotationFiducialProjectionPropertyWidget
::qMRMLAnnotationFiducialProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLAnnotationFiducialProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLAnnotationFiducialProjectionPropertyWidget
::~qMRMLAnnotationFiducialProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::setMRMLFiducialNode(vtkMRMLAnnotationFiducialNode* fiducialNode)
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  vtkMRMLAnnotationPointDisplayNode* displayNode
    = fiducialNode->GetAnnotationPointDisplayNode();

  qvtkReconnect(d->FiducialDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->FiducialDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (showProjection)
    {
    d->FiducialDisplayNode->SliceProjectionOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode
    ->SetProjectedColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::setUseFiducialColor(bool useFiducialColor)
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (useFiducialColor)
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::setOutlinedBehindSlicePlane(bool outlinedBehind)
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (outlinedBehind)
    {
    d->FiducialDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOn();
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationFiducialProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLAnnotationFiducialProjectionPropertyWidget);

  this->setEnabled(d->FiducialDisplayNode != nullptr);

  if (!d->FiducialDisplayNode)
    {
    return;
    }

  // Update widget if different from MRML node
  // -- 2D Projection Visibility
  d->Point2DProjectionCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationDisplayNode::ProjectionOn);

  // -- Projection Color
  double pColor[3];
  d->FiducialDisplayNode->GetProjectedColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->PointProjectionColorPickerButton->setColor(displayColor);

  // -- Use Fiducial Color
  d->PointUseFiducialColorCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationPointDisplayNode::ProjectionUseFiducialColor);

  // -- Outlined Behind Slice Plane
  d->PointOutlinedBehindSlicePlaneCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationPointDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

