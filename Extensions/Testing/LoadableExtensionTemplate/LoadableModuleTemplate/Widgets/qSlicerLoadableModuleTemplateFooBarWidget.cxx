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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerLoadableModuleTemplateFooBarWidget.h"
#include "ui_qSlicerLoadableModuleTemplateFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerLoadableModuleTemplateFooBarWidgetPrivate
  : public Ui_qSlicerLoadableModuleTemplateFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerLoadableModuleTemplateFooBarWidget);
protected:
  qSlicerLoadableModuleTemplateFooBarWidget* const q_ptr;

public:
  qSlicerLoadableModuleTemplateFooBarWidgetPrivate(
    qSlicerLoadableModuleTemplateFooBarWidget& object);
  virtual void setupUi(qSlicerLoadableModuleTemplateFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerLoadableModuleTemplateFooBarWidgetPrivate
::qSlicerLoadableModuleTemplateFooBarWidgetPrivate(
  qSlicerLoadableModuleTemplateFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerLoadableModuleTemplateFooBarWidgetPrivate
::setupUi(qSlicerLoadableModuleTemplateFooBarWidget* widget)
{
  this->Ui_qSlicerLoadableModuleTemplateFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateFooBarWidget
::qSlicerLoadableModuleTemplateFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerLoadableModuleTemplateFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerLoadableModuleTemplateFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateFooBarWidget
::~qSlicerLoadableModuleTemplateFooBarWidget()
{
}
