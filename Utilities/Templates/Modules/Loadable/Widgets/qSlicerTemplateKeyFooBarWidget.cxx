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
#include "qSlicerTemplateKeyFooBarWidget.h"
#include "ui_qSlicerTemplateKeyFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TemplateKey
class qSlicerTemplateKeyFooBarWidgetPrivate
  : public Ui_qSlicerTemplateKeyFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerTemplateKeyFooBarWidget);
protected:
  qSlicerTemplateKeyFooBarWidget* const q_ptr;

public:
  qSlicerTemplateKeyFooBarWidgetPrivate(
    qSlicerTemplateKeyFooBarWidget& object);
  virtual void setupUi(qSlicerTemplateKeyFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerTemplateKeyFooBarWidgetPrivate
::qSlicerTemplateKeyFooBarWidgetPrivate(
  qSlicerTemplateKeyFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerTemplateKeyFooBarWidgetPrivate
::setupUi(qSlicerTemplateKeyFooBarWidget* widget)
{
  this->Ui_qSlicerTemplateKeyFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyFooBarWidget
::qSlicerTemplateKeyFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerTemplateKeyFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerTemplateKeyFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerTemplateKeyFooBarWidget
::~qSlicerTemplateKeyFooBarWidget()
{
}
