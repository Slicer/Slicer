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

// Slicer includes
#include "qSlicerEventBrokerModuleWidget.h"
#include "ui_qSlicerEventBrokerModuleWidget.h"

#include <sstream>
#include <iostream>

//-----------------------------------------------------------------------------
class qSlicerEventBrokerModuleWidgetPrivate: public Ui_qSlicerEventBrokerModuleWidget
{
public:
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
void qSlicerEventBrokerModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerEventBrokerModuleWidget::setupUi(widget);
  QObject::connect(this->EventBrokerWidget, SIGNAL(currentObjectChanged(vtkObject*)),
          widget, SLOT(onCurrentObjectChanged(vtkObject*)));
}

//-----------------------------------------------------------------------------
qSlicerEventBrokerModuleWidget::qSlicerEventBrokerModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerEventBrokerModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEventBrokerModuleWidget::~qSlicerEventBrokerModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerEventBrokerModuleWidget::setup()
{
  Q_D(qSlicerEventBrokerModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEventBrokerModuleWidget::onCurrentObjectChanged(vtkObject* object)
{
  Q_D(qSlicerEventBrokerModuleWidget);
  if (!object)
    {
    return;
    }
  std::stringstream dumpStream;
  object->Print(dumpStream);
  d->TextEdit->setText(QString::fromStdString(dumpStream.str()));
}
