/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>

// qMRML includes
#include "qMRMLScalarInvariantComboBox.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLScalarInvariantComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLScalarInvariantComboBox);

protected:
  qMRMLScalarInvariantComboBox* const q_ptr;

public:
  qMRMLScalarInvariantComboBoxPrivate(qMRMLScalarInvariantComboBox& object);
  void init();

  QComboBox*                                   ComboBox;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* DisplayPropertiesNode;

private:
  void populateComboBox();
};

//------------------------------------------------------------------------------
qMRMLScalarInvariantComboBoxPrivate::qMRMLScalarInvariantComboBoxPrivate(
  qMRMLScalarInvariantComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = 0;
  this->DisplayPropertiesNode = 0;
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBoxPrivate::init()
{
  Q_Q(qMRMLScalarInvariantComboBox);

  this->ComboBox = new QComboBox(q);
  QHBoxLayout* layout = new QHBoxLayout(q);
  layout->addWidget(this->ComboBox);
  layout->setContentsMargins(0,0,0,0);
  q->setLayout(layout);
  q->setSizePolicy(this->ComboBox->sizePolicy());

  this->populateComboBox();
  QObject::connect(this->ComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentScalarInvariantChanged(int)));
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBoxPrivate::populateComboBox()
{
  this->ComboBox->clear();
  for (int i = vtkMRMLDiffusionTensorDisplayPropertiesNode::GetFirstScalarInvariant();
       i <= vtkMRMLDiffusionTensorDisplayPropertiesNode::GetLastScalarInvariant();
       ++i)
    {
    this->ComboBox->addItem(
      vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(i),
      QVariant(i));
    }
}

//------------------------------------------------------------------------------
qMRMLScalarInvariantComboBox::qMRMLScalarInvariantComboBox(QWidget* parentWidget)
  : QWidget(parentWidget)
  , d_ptr(new qMRMLScalarInvariantComboBoxPrivate(*this))
{
  Q_D(qMRMLScalarInvariantComboBox);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLScalarInvariantComboBox::~qMRMLScalarInvariantComboBox()
{
}

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* qMRMLScalarInvariantComboBox::displayPropertiesNode()const
{
  Q_D(const qMRMLScalarInvariantComboBox);
  return d->DisplayPropertiesNode;
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::setDisplayPropertiesNode(vtkMRMLNode* node)
{
  this->setDisplayPropertiesNode(
    vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::setDisplayPropertiesNode(
  vtkMRMLDiffusionTensorDisplayPropertiesNode* displayNode)
{
  Q_D(qMRMLScalarInvariantComboBox);
  qvtkReconnect(d->DisplayPropertiesNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->DisplayPropertiesNode = displayNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::updateWidgetFromMRML()
{
  Q_D(qMRMLScalarInvariantComboBox);
  if (!d->DisplayPropertiesNode)
    {
    return;
    }
  // The combobox has been populated on the assumption that all the scalar
  // invariant were comprised between GetFirstScalarInvariant() and
  // GetLastScalarInvariant().
  Q_ASSERT(d->DisplayPropertiesNode->GetScalarInvariant() >=
           vtkMRMLDiffusionTensorDisplayPropertiesNode::GetFirstScalarInvariant());
  Q_ASSERT(d->DisplayPropertiesNode->GetScalarInvariant() <=
           vtkMRMLDiffusionTensorDisplayPropertiesNode::GetLastScalarInvariant());
  int index = d->ComboBox->findData(QVariant(d->DisplayPropertiesNode->GetColorGlyphBy()));
  Q_ASSERT(index >= 0);
  d->ComboBox->setCurrentIndex(index);
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::onCurrentScalarInvariantChanged(int index)
{
  Q_D(qMRMLScalarInvariantComboBox);
  // if onCurrentScalarInvariantChanged() is called it's because the
  // DisplayPropertiesNode is not null.
  Q_ASSERT(d->DisplayPropertiesNode);
  int scalarInvariant = d->ComboBox->itemData(index).toInt();
  d->DisplayPropertiesNode->SetColorGlyphBy(scalarInvariant);
  emit scalarInvariantChanged(scalarInvariant);
}

//------------------------------------------------------------------------------
int qMRMLScalarInvariantComboBox::scalarInvariant()const
{
  Q_D(const qMRMLScalarInvariantComboBox);
  return d->DisplayPropertiesNode ? d->DisplayPropertiesNode->GetScalarInvariant() : -1;
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::setScalarInvariant(int value)
{
  Q_D(qMRMLScalarInvariantComboBox);
  if (!d->DisplayPropertiesNode)
    {
    return;
    }
  d->DisplayPropertiesNode->SetScalarInvariant(value);
}
