/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QComboBox>
#include <QHBoxLayout>

// qMRML includes
#include "qMRMLScalarInvariantComboBox.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>

// VTK includes

//------------------------------------------------------------------------------
class qMRMLScalarInvariantComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLScalarInvariantComboBox);

protected:
  qMRMLScalarInvariantComboBox* const q_ptr;

public:
  qMRMLScalarInvariantComboBoxPrivate(qMRMLScalarInvariantComboBox& object);
  void init();
  void setScalarInvariantToComboBox(int scalarInvariant);

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
  this->ComboBox = nullptr;
  this->DisplayPropertiesNode = nullptr;
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
  for (int i = 0;
        i < vtkMRMLDiffusionTensorVolumeDisplayNode::GetNumberOfScalarInvariants();
       ++i)
    {
    const int scalarInvariant = vtkMRMLDiffusionTensorVolumeDisplayNode::GetNthScalarInvariant(i);
    this->ComboBox->addItem(
      vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(scalarInvariant),
      QVariant(scalarInvariant));
    }

}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBoxPrivate::setScalarInvariantToComboBox(int scalarInvariant)
{ // The combobox has been populated on the assumption that all the scalar
  // invariant were comprised between GetFirstScalarInvariant() and
  // GetLastScalarInvariant().
  Q_ASSERT(scalarInvariant >=
           vtkMRMLDiffusionTensorDisplayPropertiesNode::GetFirstScalarInvariant());
  Q_ASSERT(scalarInvariant <=
           vtkMRMLDiffusionTensorDisplayPropertiesNode::GetLastScalarInvariant());
  int index = this->ComboBox->findData(QVariant(scalarInvariant));
  Q_ASSERT(index >= 0);
  this->ComboBox->setCurrentIndex(index);
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
qMRMLScalarInvariantComboBox::~qMRMLScalarInvariantComboBox() = default;

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
  d->setScalarInvariantToComboBox(d->DisplayPropertiesNode->GetColorGlyphBy());
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::onCurrentScalarInvariantChanged(int index)
{
  Q_D(qMRMLScalarInvariantComboBox);
  int scalarInvariant = d->ComboBox->itemData(index).toInt();
  this->setScalarInvariant(scalarInvariant);
  emit scalarInvariantChanged(scalarInvariant);
}

//------------------------------------------------------------------------------
int qMRMLScalarInvariantComboBox::scalarInvariant()const
{
  Q_D(const qMRMLScalarInvariantComboBox);
  QVariant scalarInvariant = d->ComboBox->itemData(d->ComboBox->currentIndex());
  return scalarInvariant.isValid() ? scalarInvariant.toInt() : -1;
}

//------------------------------------------------------------------------------
void qMRMLScalarInvariantComboBox::setScalarInvariant(int value)
{
  Q_D(qMRMLScalarInvariantComboBox);
  if (!d->DisplayPropertiesNode)
    {
    d->setScalarInvariantToComboBox(value);
    }
  else
    {
    // SetColorGlyphBy will eventually call updateWidgetFromMRML
    d->DisplayPropertiesNode->SetColorGlyphBy(value);
    }
}
