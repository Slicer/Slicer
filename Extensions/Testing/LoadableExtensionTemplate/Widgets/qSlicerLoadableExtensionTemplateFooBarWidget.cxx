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
#include "qSlicerLoadableExtensionTemplateFooBarWidget.h"
#include "ui_qSlicerLoadableExtensionTemplateFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_LoadableExtensionTemplate
class qSlicerLoadableExtensionTemplateFooBarWidgetPrivate
  : public Ui_qSlicerLoadableExtensionTemplateFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerLoadableExtensionTemplateFooBarWidget);
protected:
  qSlicerLoadableExtensionTemplateFooBarWidget* const q_ptr;

public:
  qSlicerLoadableExtensionTemplateFooBarWidgetPrivate(
    qSlicerLoadableExtensionTemplateFooBarWidget& object);
  virtual void setupUi(qSlicerLoadableExtensionTemplateFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateFooBarWidgetPrivate
::qSlicerLoadableExtensionTemplateFooBarWidgetPrivate(
  qSlicerLoadableExtensionTemplateFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerLoadableExtensionTemplateFooBarWidgetPrivate
::setupUi(qSlicerLoadableExtensionTemplateFooBarWidget* widget)
{
  this->Ui_qSlicerLoadableExtensionTemplateFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerLoadableExtensionTemplateFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateFooBarWidget
::qSlicerLoadableExtensionTemplateFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerLoadableExtensionTemplateFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerLoadableExtensionTemplateFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateFooBarWidget
::~qSlicerLoadableExtensionTemplateFooBarWidget()
{
}
