#include "qSlicerWelcomeModule.h"
#include "ui_qSlicerWelcomeModule.h"

// CTK includes
#include "qCTKCollapsibleButton.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QtPlugin>
#include <QFontMetrics>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
struct qSlicerWelcomeModulePrivate: public qCTKPrivate<qSlicerWelcomeModule>,
                                    public Ui_qSlicerWelcomeModule
{
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModule, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerWelcomeModule::setup()
{
  this->Superclass::setup();
  QCTK_D(qSlicerWelcomeModule);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
// qSlicerWelcomeModulePrivate methods

//-----------------------------------------------------------------------------
void qSlicerWelcomeModulePrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerWelcomeModule::setupUi(widget);

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  QButtonGroup * group = new QButtonGroup(widget);
  
  // Add all collabsibleWidgetButton to a button group
  QList<qCTKCollapsibleButton*> collapsibles = widget->findChildren<qCTKCollapsibleButton*>();
  foreach(qCTKCollapsibleButton* collapsible, collapsibles)
    {
    collapsible->setCheckable(true);
    group->addButton(collapsible);
    }
}
