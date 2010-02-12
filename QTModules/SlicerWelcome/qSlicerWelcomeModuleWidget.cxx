#include "qSlicerWelcomeModuleWidget.h"
#include "ui_qSlicerWelcomeModule.h"

// qCTK includes
#include "qCTKCollapsibleButton.h"
#include "qCTKButtonGroup.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerWelcomeModuleWidgetPrivate: public qCTKPrivate<qSlicerWelcomeModuleWidget>,
                                          public Ui_qSlicerWelcomeModule
{
public:
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setup()
{
  QCTK_D(qSlicerWelcomeModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerWelcomeModule::setupUi(widget);

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  //QButtonGroup * group = new QButtonGroup(widget);
  qCTKButtonGroup * group = new qCTKButtonGroup(widget);
  
  // Add all collabsibleWidgetButton to a button group
  QList<qCTKCollapsibleButton*> collapsibles = widget->findChildren<qCTKCollapsibleButton*>();
  foreach(qCTKCollapsibleButton* collapsible, collapsibles)
    {
    group->addButton(collapsible);
    }
}
