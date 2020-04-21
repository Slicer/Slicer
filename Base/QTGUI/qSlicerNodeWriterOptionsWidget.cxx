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

// qSlicer includes
#include "qSlicerNodeWriterOptionsWidget.h"
#include "qSlicerNodeWriterOptionsWidget_p.h"

// MRML includes
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidgetPrivate::~qSlicerNodeWriterOptionsWidgetPrivate() = default;

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidgetPrivate::setupUi(QWidget* widget)
{
  this->Ui_qSlicerNodeWriterOptionsWidget::setupUi(widget);
  QObject::connect(this->UseCompressionCheckBox, SIGNAL(toggled(bool)),
                   widget, SLOT(setUseCompression(bool)));
  QObject::connect(this->CompressionParameterSelector, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setCompressionParameter(int)));
}

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidget
::qSlicerNodeWriterOptionsWidget(qSlicerNodeWriterOptionsWidgetPrivate* pimpl,
                                   QWidget* parentWidget)
  : Superclass(pimpl, parentWidget)
{
}

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidget::qSlicerNodeWriterOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerNodeWriterOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  d->setupUi(this);
}

//------------------------------------------------------------------------------
qSlicerNodeWriterOptionsWidget::~qSlicerNodeWriterOptionsWidget() = default;

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
  if (storableNode != nullptr)
    {
    d->Properties["nodeID"] = storableNode->GetID();
    }
  else
    {
    d->Properties.remove("nodeID");
    }
  vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
  d->UseCompressionCheckBox->setEnabled(storageNode != nullptr);
  if (storageNode)
    {
    d->UseCompressionCheckBox->setChecked(
      (storageNode->GetUseCompression() == 1));

    std::vector<vtkMRMLStorageNode::CompressionPreset> presets = storageNode->GetCompressionPresets();
    d->CompressionParameterSelector->clear();
    std::vector<vtkMRMLStorageNode::CompressionPreset>::iterator presetIt;
    for (presetIt = presets.begin(); presetIt != presets.end(); ++presetIt)
      {
      QString name = QString::fromStdString(presetIt->DisplayName);
      QString parameter = QString::fromStdString(presetIt->CompressionParameter);
      d->CompressionParameterSelector->addItem(name, parameter);
      }
    this->setCompressionParameter(QString::fromStdString(storageNode->GetCompressionParameter()));
    }
  d->CompressionParameterSelector->setVisible(d->CompressionParameterSelector->count() > 0);
  d->CompressionParameterSelector->setEnabled(storageNode != nullptr && d->UseCompressionCheckBox->isChecked());

  this->updateValid();
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setUseCompression(bool use)
{
  Q_D(qSlicerNodeWriterOptionsWidget);
  d->Properties["useCompression"] = (use ? 1 : 0);
  d->CompressionParameterSelector->setEnabled(d->UseCompressionCheckBox->isChecked());
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
  d->UseCompressionCheckBox->setVisible(show);
  d->CompressionParameterSelector->setVisible(show && d->CompressionParameterSelector->count() > 0);
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setCompressionParameter(int index)
{
  Q_D(qSlicerNodeWriterOptionsWidget);

  QString parameter = d->CompressionParameterSelector->itemData(index).toString();
  d->Properties["compressionParameter"] = parameter;
}

//------------------------------------------------------------------------------
void qSlicerNodeWriterOptionsWidget::setCompressionParameter(QString parameter)
{
  Q_D(qSlicerNodeWriterOptionsWidget);

  int index = d->CompressionParameterSelector->findData(parameter);
  d->CompressionParameterSelector->setCurrentIndex(index);
  d->Properties["compressionParameter"] = parameter;
}
