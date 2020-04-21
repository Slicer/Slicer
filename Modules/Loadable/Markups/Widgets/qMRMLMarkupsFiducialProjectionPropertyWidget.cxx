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
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

// qMRML includes
#include "qMRMLMarkupsFiducialProjectionPropertyWidget.h"
#include "ui_qMRMLMarkupsFiducialProjectionPropertyWidget.h"

// MRML includes
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
  : public Ui_qMRMLMarkupsFiducialProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsFiducialProjectionPropertyWidget);
protected:
  qMRMLMarkupsFiducialProjectionPropertyWidget* const q_ptr;
public:
  qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(qMRMLMarkupsFiducialProjectionPropertyWidget& object);
  void init();

  vtkMRMLMarkupsDisplayNode* FiducialDisplayNode;
};

//-----------------------------------------------------------------------------
// qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
::qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(qMRMLMarkupsFiducialProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->FiducialDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qMRMLMarkupsFiducialProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->point2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->pointProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->pointUseFiducialColorCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setUseFiducialColor(bool)));
  QObject::connect(this->pointOutlinedBehindSlicePlaneCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setOutlinedBehindSlicePlane(bool)));
  QObject::connect(this->projectionOpacitySliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(setProjectionOpacity(double)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLMarkupsFiducialProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidget
::qMRMLMarkupsFiducialProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidget
::~qMRMLMarkupsFiducialProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  vtkMRMLMarkupsDisplayNode* displayNode = (markupsNode ? markupsNode->GetMarkupsDisplayNode() : nullptr);
  this->setMRMLMarkupsDisplayNode(displayNode);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setMRMLMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (markupsDisplayNode == d->FiducialDisplayNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->FiducialDisplayNode, markupsDisplayNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromDisplayNode()));

  d->FiducialDisplayNode = markupsDisplayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
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
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode
    ->SetSliceProjectionColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setUseFiducialColor(bool useFiducialColor)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  if (useFiducialColor)
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOn();
    d->pointProjectionColorLabel->setEnabled(false);
    d->pointProjectionColorPickerButton->setEnabled(false);
    }
  else
    {
    d->FiducialDisplayNode->SliceProjectionUseFiducialColorOff();
    d->pointProjectionColorLabel->setEnabled(true);
    d->pointProjectionColorPickerButton->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setOutlinedBehindSlicePlane(bool outlinedBehind)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
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
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionOpacity(double opacity)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->FiducialDisplayNode)
    {
    return;
    }
  d->FiducialDisplayNode->SetSliceProjectionOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);

  this->setEnabled(d->FiducialDisplayNode != nullptr);

  if (!d->FiducialDisplayNode)
    {
    return;
    }

  // Update widget if different from MRML node
  // -- 2D Projection Visibility
  d->point2DProjectionCheckBox->setChecked(
    d->FiducialDisplayNode->GetSliceProjection());

  // -- Projection Color
  double pColor[3];
  d->FiducialDisplayNode->GetSliceProjectionColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->pointProjectionColorPickerButton->setColor(displayColor);

  // -- Use Fiducial Color
  bool useFiducialColor = d->FiducialDisplayNode->GetSliceProjectionUseFiducialColor();
  d->pointUseFiducialColorCheckBox->setChecked(useFiducialColor);
  d->pointProjectionColorLabel->setEnabled(!useFiducialColor);
  d->pointProjectionColorPickerButton->setEnabled(!useFiducialColor);

  // -- Outlined Behind Slice Plane
  d->pointOutlinedBehindSlicePlaneCheckBox->setChecked(
     d->FiducialDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());

  // -- Opacity
  d->projectionOpacitySliderWidget->setValue(
     d->FiducialDisplayNode->GetSliceProjectionOpacity());
}
