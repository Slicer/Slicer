/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

  ==============================================================================*/

#include "qSlicerMarkupsTestLineWidget.h"
#include "ui_qSlicerMarkupsTestLineWidget.h"

// MRML Markups includes
#include <vtkMRMLMarkupsNode.h>

#include "vtkMRMLMarkupsTestLineNode.h"

// --------------------------------------------------------------------------
class qSlicerMarkupsTestLineWidgetPrivate:
  public qSlicerMarkupsAdditionalOptionsWidgetPrivate,
  public Ui_qSlicerMarkupsTestLineWidget
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsTestLineWidget);

protected:
  qSlicerMarkupsTestLineWidget* const q_ptr;

public:
  qSlicerMarkupsTestLineWidgetPrivate(qSlicerMarkupsTestLineWidget* object);
  ~qSlicerMarkupsTestLineWidgetPrivate();

  void setupUi(qSlicerWidget* widget);

  virtual void setupUi(qSlicerMarkupsTestLineWidget*);
};

// --------------------------------------------------------------------------
qSlicerMarkupsTestLineWidgetPrivate::
qSlicerMarkupsTestLineWidgetPrivate(qSlicerMarkupsTestLineWidget* object)
  : q_ptr(object)
{

}

// --------------------------------------------------------------------------
qSlicerMarkupsTestLineWidgetPrivate::~qSlicerMarkupsTestLineWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerMarkupsTestLineWidgetPrivate::setupUi(qSlicerMarkupsTestLineWidget* widget)
{
  Q_Q(qSlicerMarkupsTestLineWidget);

  this->Ui_qSlicerMarkupsTestLineWidget::setupUi(widget);
  this->lineTestCollapsibleButton->setVisible(false);
}

// --------------------------------------------------------------------------
qSlicerMarkupsTestLineWidget::
qSlicerMarkupsTestLineWidget(QWidget *parent)
  : Superclass(* new qSlicerMarkupsTestLineWidgetPrivate(this), parent)
{
  this->setup();
}

// --------------------------------------------------------------------------
qSlicerMarkupsTestLineWidget::
qSlicerMarkupsTestLineWidget(qSlicerMarkupsTestLineWidgetPrivate &d, QWidget *parent)
  : Superclass(d, parent)
{
  this->setup();
}

// --------------------------------------------------------------------------
qSlicerMarkupsTestLineWidget::~qSlicerMarkupsTestLineWidget() = default;

// --------------------------------------------------------------------------
void qSlicerMarkupsTestLineWidget::setup()
{
  Q_D(qSlicerMarkupsTestLineWidget);
  d->setupUi(this);
}
// --------------------------------------------------------------------------
void qSlicerMarkupsTestLineWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerMarkupsTestLineWidget);

  if (!this->canManageMRMLMarkupsNode(d->MarkupsNode))
    {
    d->lineTestCollapsibleButton->setVisible(false);
    return;
    }

  d->lineTestCollapsibleButton->setVisible(true);
}


//-----------------------------------------------------------------------------
bool qSlicerMarkupsTestLineWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  Q_D(const qSlicerMarkupsTestLineWidget);

  vtkMRMLMarkupsTestLineNode* testLineNode= vtkMRMLMarkupsTestLineNode::SafeDownCast(markupsNode);
  if (!testLineNode)
    {
    return false;
    }

  return true;
}
