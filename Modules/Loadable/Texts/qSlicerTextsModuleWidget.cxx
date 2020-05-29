/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Slicer includes
#include "qSlicerTextsModuleWidget.h"
#include "ui_qSlicerTextsModuleWidget.h"

// vtkSlicerLogic includes
#include "vtkSlicerTextsLogic.h"

// MRML includes
#include "vtkMRMLTextNode.h"

//-----------------------------------------------------------------------------
class qSlicerTextsModuleWidgetPrivate: public Ui_qSlicerTextsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTextsModuleWidget);
protected:
  qSlicerTextsModuleWidget* const q_ptr;
public:
  qSlicerTextsModuleWidgetPrivate(qSlicerTextsModuleWidget& object);
  vtkSlicerTextsLogic*      logic() const;
};

//-----------------------------------------------------------------------------
qSlicerTextsModuleWidgetPrivate::qSlicerTextsModuleWidgetPrivate(qSlicerTextsModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerTextsLogic* qSlicerTextsModuleWidgetPrivate::logic()const
{
  Q_Q(const qSlicerTextsModuleWidget);
  return vtkSlicerTextsLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerTextsModuleWidget::qSlicerTextsModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerTextsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerTextsModuleWidget::~qSlicerTextsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerTextsModuleWidget::setup()
{
  Q_D(qSlicerTextsModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
bool qSlicerTextsModuleWidget::setEditedNode(
  vtkMRMLNode* node,
  QString role/*=QString()*/,
  QString context/*=QString()*/)
{
  Q_D(qSlicerTextsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLTextNode::SafeDownCast(node))
    {
    d->TextNodeSelector->setCurrentNode(node);
    return true;
    }
  return false;
}
