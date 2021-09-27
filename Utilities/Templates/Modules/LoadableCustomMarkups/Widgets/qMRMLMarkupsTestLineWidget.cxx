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

#include "qMRMLMarkupsTestLineWidget.h"
#include "ui_qMRMLMarkupsTestLineWidget.h"

// MRML Nodes includes
#include "vtkMRMLMarkupsTestLineNode.h"

// VTK includes
#include <vtkWeakPointer.h>

// --------------------------------------------------------------------------
class qMRMLMarkupsTestLineWidgetPrivate:
  public Ui_qMRMLMarkupsTestLineWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsTestLineWidget);

protected:
  qMRMLMarkupsTestLineWidget* const q_ptr;

public:
  qMRMLMarkupsTestLineWidgetPrivate(qMRMLMarkupsTestLineWidget* object);
  void setupUi(qMRMLMarkupsTestLineWidget*);

  vtkWeakPointer<vtkMRMLMarkupsTestLineNode> MarkupsTestLineNode;
};

// --------------------------------------------------------------------------
qMRMLMarkupsTestLineWidgetPrivate::
qMRMLMarkupsTestLineWidgetPrivate(qMRMLMarkupsTestLineWidget* object)
  : q_ptr(object)
{

}

// --------------------------------------------------------------------------
void qMRMLMarkupsTestLineWidgetPrivate::setupUi(qMRMLMarkupsTestLineWidget* widget)
{
  Q_Q(qMRMLMarkupsTestLineWidget);

  this->Ui_qMRMLMarkupsTestLineWidget::setupUi(widget);
  this->lineTestCollapsibleButton->setVisible(false);
}

// --------------------------------------------------------------------------
qMRMLMarkupsTestLineWidget::
qMRMLMarkupsTestLineWidget(QWidget *parent)
  : Superclass(parent),
    d_ptr(new qMRMLMarkupsTestLineWidgetPrivate(this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qMRMLMarkupsTestLineWidget::~qMRMLMarkupsTestLineWidget() = default;

// --------------------------------------------------------------------------
void qMRMLMarkupsTestLineWidget::setup()
{
  Q_D(qMRMLMarkupsTestLineWidget);
  d->setupUi(this);
}
// --------------------------------------------------------------------------
void qMRMLMarkupsTestLineWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsTestLineWidget);

  if (!this->canManageMRMLMarkupsNode(d->MarkupsTestLineNode))
    {
    d->lineTestCollapsibleButton->setVisible(false);
    return;
    }

  d->lineTestCollapsibleButton->setVisible(true);
}


//-----------------------------------------------------------------------------
bool qMRMLMarkupsTestLineWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  Q_D(const qMRMLMarkupsTestLineWidget);

  vtkMRMLMarkupsTestLineNode* testLineNode= vtkMRMLMarkupsTestLineNode::SafeDownCast(markupsNode);
  if (!testLineNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsTestLineWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qMRMLMarkupsTestLineWidget);

  d->MarkupsTestLineNode = vtkMRMLMarkupsTestLineNode::SafeDownCast(markupsNode);
  this->setEnabled(markupsNode != nullptr);
}
