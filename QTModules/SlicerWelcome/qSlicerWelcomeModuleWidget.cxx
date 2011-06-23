/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// Qt includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>

// SlicerQt includes
#include "qSlicerWelcomeModuleWidget.h"
#include "ui_qSlicerWelcomeModule.h"

// CTK includes
#include "ctkCollapsibleButton.h"
#include "ctkButtonGroup.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SlicerWelcome
class qSlicerWelcomeModuleWidgetPrivate: public Ui_qSlicerWelcomeModule
{
public:
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::qSlicerWelcomeModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWelcomeModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::~qSlicerWelcomeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setup()
{
  Q_D(qSlicerWelcomeModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerWelcomeModule::setupUi(widget);

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup * group = new ctkButtonGroup(widget);
  
  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach(ctkCollapsibleButton* collapsible, collapsibles)
    {
    group->addButton(collapsible);
    }
}
