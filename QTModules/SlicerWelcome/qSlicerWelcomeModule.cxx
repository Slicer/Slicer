#include "qSlicerWelcomeModule.h"
#include "ui_qSlicerWelcomeModule.h"

// QT includes
#include <QtPlugin>
#include <QFontMetrics>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
class qSlicerWelcomeModule::qInternal: public Ui::qSlicerWelcomeModule
{
public:
//   void setupUi(qSlicerWidget* widget);
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

// //-----------------------------------------------------------------------------
// // Internal methods
//
// //-----------------------------------------------------------------------------
// void qSlicerWelcomeModule::qInternal::setupUi(qSlicerWidget* widget)
// {
//   this->Ui::qSlicerWelcomeModule::setupUi(widget);
//
//
//   // Update veritcal size policy and minimum height of all text browsers
// //   QList<QTextBrowser*> textBrowsers = widget->findChild<QTextBrowser*>();
// //   foreach(QTextBrowser* textBrowser, textBrowsers)
// //     {
// //     // Overwrite vertical size policy
// //     textBrowser->setSizePolicy(textBrowser->sizePolicy().horizontalPolicy(), QSizePolicy::Ignored);
// //
// //     textBrowser->setMinimumHeight(QFontMetrics fm(font).xHeight() * 28);
// //     }
//
// }
