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

// qSlicer includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerNodeWriterOptionsWidget.h"
#include "ui_qSlicerNodeWriterOptionsWidget.h"

// MRML includes
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

//------------------------------------------------------------------------------
class qSlicerNodeWriterOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerNodeWriterOptionsWidget
{
public:
  virtual ~qSlicerNodeWriterOptionsWidgetPrivate();
  virtual void setupUi(QWidget* widget);
};

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidgetPrivate::~qSlicerNodeWriterOptionsWidgetPrivate()
{
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidgetPrivate::setupUi(QWidget* widget)
{
  this->Ui_qSlicerNodeWriterOptionsWidget::setupUi(widget);
  QObject::connect(this->UseCompressionCheckBox, SIGNAL(toggled(bool)),
                   widget, SLOT(setUseCompression(bool)));
}

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidget::qSlicerNodeWriterOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerNodeWriterOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  d->setupUi(this);
}

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidget::~qSlicerNodeWriterOptionsWidget()
{
}

//------------------------------------------------------------------------------
bool qSlicerNodeWriterOptionsWidget::isValid()const
{
  Q_D(const qSlicerNodeWriterOptionsWidget);
  return d->Properties.contains("nodeID") &&
         d->Properties.contains("fileName");
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setObject(vtkObject* object)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(object);
  if (storableNode != 0)
    {
    d->Properties["nodeID"] = storableNode->GetID();
    }
  else
    {
    d->Properties.remove("nodeID");
    }
  vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
  d->UseCompressionCheckBox->setEnabled(storageNode != 0);
  if (storageNode)
    {
    d->UseCompressionCheckBox->setChecked(
      (storageNode->GetUseCompression() == 1));
    }

  this->updateValid();
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setUseCompression(bool use)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  d->Properties["useCompression"] = (use ? 1 : 0);
}

//------------------------------------------------------------------------------
bool qSlicerNodeWriterOptionsWidget::showUseCompression()const
{
  Q_D(const qSlicerNodeWriterOptionsWidget);
  return d->UseCompressionCheckBox->isVisibleTo(
    const_cast<qSlicerNodeWriterOptionsWidget*>(this));
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setShowUseCompression(bool show)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  return d->UseCompressionCheckBox->setVisible(show);
}
