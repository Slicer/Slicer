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
class qSlicerWelcomeModule::qInternal: public Ui::qSlicerWelcomeModule
{
public:
  void setupUi(qSlicerWidget* widget);
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerWelcomeModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
void qSlicerWelcomeModule::setup()
{
  this->Superclass::setup();
  Q_ASSERT(this->Internal != 0);

  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
// Internal methods

//-----------------------------------------------------------------------------
void qSlicerWelcomeModule::qInternal::setupUi(qSlicerWidget* widget)
{
  this->Ui::qSlicerWelcomeModule::setupUi(widget);

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
