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

#include "qSlicerMarkupsAdditionalOptionsWidget.h"
#include "qSlicerMarkupsAdditionalOptionsWidget_p.h"

// Qt includes
#include <QDebug>

// MRML includes
#include "vtkMRMLMarkupsNode.h"

//-----------------------------------------------------------------------------
// qSlicerMarkupsAdditionalOptionsWidgetmethods

qSlicerMarkupsAdditionalOptionsWidgetPrivate:: qSlicerMarkupsAdditionalOptionsWidgetPrivate()
{
  this->MarkupsNode = nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsAdditionalOptionsWidgetmethods

//-----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidget::
qSlicerMarkupsAdditionalOptionsWidget(QWidget* parent)
  : Superclass(parent), d_ptr(new qSlicerMarkupsAdditionalOptionsWidgetPrivate)
{

}

//-----------------------------------------------------------------------------
qSlicerMarkupsAdditionalOptionsWidget::
qSlicerMarkupsAdditionalOptionsWidget(qSlicerMarkupsAdditionalOptionsWidgetPrivate &d, QWidget* parent)
  : Superclass(parent), d_ptr(&d)
{

}

// --------------------------------------------------------------------------
vtkMRMLMarkupsNode* qSlicerMarkupsAdditionalOptionsWidget::mrmlMarkupsNode()
{
  Q_D(qSlicerMarkupsAdditionalOptionsWidget);

  return d->MarkupsNode;
}

// --------------------------------------------------------------------------
void qSlicerMarkupsAdditionalOptionsWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qSlicerMarkupsAdditionalOptionsWidget);

  d->MarkupsNode = markupsNode;
}

// --------------------------------------------------------------------------
void qSlicerMarkupsAdditionalOptionsWidget::setMRMLMarkupsNode(vtkMRMLNode* node)
{
  Q_D(qSlicerMarkupsAdditionalOptionsWidget);

  this->setMRMLMarkupsNode(vtkMRMLMarkupsNode::SafeDownCast(node));
}
