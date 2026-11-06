/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Colors Widgets includes
#include "qMRMLColorLegendGroupBox.h"
#include "qMRMLColorLegendDisplayNodeWidget.h"

// Colors Logic includes
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
class qMRMLColorLegendGroupBoxPrivate
{
public:
  qMRMLColorLegendDisplayNodeWidget* ColorLegendDisplayNodeWidget{ nullptr };
  vtkWeakPointer<vtkMRMLDisplayNode> PrimaryDisplayNode;
  vtkWeakPointer<vtkMRMLDisplayableNode> DisplayableNode;
};

//-----------------------------------------------------------------------------
qMRMLColorLegendGroupBox::qMRMLColorLegendGroupBox(QWidget* parent)
  : Superclass(parent)
  , d_ptr(new qMRMLColorLegendGroupBoxPrivate)
{
  Q_D(qMRMLColorLegendGroupBox);
  this->setTitle(tr("Color legend"));
  this->setToolTip(tr("Color legend for the color table and range that the colors are shown with."));
  QVBoxLayout* layout = new QVBoxLayout(this);
  d->ColorLegendDisplayNodeWidget = new qMRMLColorLegendDisplayNodeWidget(this);
  d->ColorLegendDisplayNodeWidget->setObjectName("ColorLegendDisplayNodeWidget");
  layout->addWidget(d->ColorLegendDisplayNodeWidget);
  this->setCollapsed(true);
  // Opening the section is what makes the legend editable, so that is when it is created
  this->connect(this, SIGNAL(toggled(bool)), SLOT(updateWidgetFromMRML()));
  this->setEnabled(false);
}

//-----------------------------------------------------------------------------
qMRMLColorLegendGroupBox::~qMRMLColorLegendGroupBox() = default;

//-----------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLColorLegendGroupBox::primaryDisplayNode() const
{
  Q_D(const qMRMLColorLegendGroupBox);
  return d->PrimaryDisplayNode;
}

//-----------------------------------------------------------------------------
qMRMLColorLegendDisplayNodeWidget* qMRMLColorLegendGroupBox::colorLegendDisplayNodeWidget() const
{
  Q_D(const qMRMLColorLegendGroupBox);
  return d->ColorLegendDisplayNodeWidget;
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendGroupBox::setPrimaryDisplayNode(vtkMRMLNode* node)
{
  this->setPrimaryDisplayNode(vtkMRMLDisplayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendGroupBox::setPrimaryDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLColorLegendGroupBox);
  // The colors are in the display node; a legend that is added or removed is a display node of
  // the displayable node
  vtkMRMLDisplayableNode* displayableNode = (displayNode ? displayNode->GetDisplayableNode() : nullptr);
  this->qvtkReconnect(d->PrimaryDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(d->DisplayableNode, displayableNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->PrimaryDisplayNode = displayNode;
  d->DisplayableNode = displayableNode;
  d->ColorLegendDisplayNodeWidget->setMRMLScene(displayNode ? displayNode->GetScene() : nullptr);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendGroupBox::updateWidgetFromMRML()
{
  Q_D(qMRMLColorLegendGroupBox);
  vtkMRMLDisplayNode* displayNode = d->PrimaryDisplayNode;
  vtkMRMLColorLegendDisplayNode* colorLegendNode = (displayNode ? vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode) : nullptr);
  if (!colorLegendNode && displayNode && !this->collapsed())
  {
    // The section is open, so the legend has to be editable: created hidden, until the user
    // shows it
    colorLegendNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    if (colorLegendNode)
    {
      colorLegendNode->SetVisibility(false);
    }
  }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
  // A legend only says something when the colors come from a color table
  this->setEnabled(displayNode != nullptr && displayNode->GetScalarVisibility() && displayNode->GetColorNode() != nullptr);
}
