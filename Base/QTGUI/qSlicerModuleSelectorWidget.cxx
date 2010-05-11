
// Qt includes
#include <QDebug>

// CTK includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"

// SlicerQt
#include "qSlicerModuleSelectorWidget.h"
#include "qSlicerModuleSelectorWidget_p.h"

//---------------------------------------------------------------------------
qSlicerModuleSelectorWidget::qSlicerModuleSelectorWidget(QWidget* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerModuleSelectorWidget);
  CTK_D(qSlicerModuleSelectorWidget);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidget::addModules(const QStringList& moduleNames)
{
  CTK_D(qSlicerModuleSelectorWidget);
  d->addModules(moduleNames);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidget::removeModule(const QString& name)
{
  CTK_D(qSlicerModuleSelectorWidget);
  d->removeModule(name);
}

//---------------------------------------------------------------------------
// qSlicerModuleSelectorWidgetPrivate methods

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::setupUi(QWidget* widget)
{
  this->Ui_qSlicerModuleSelectorWidget::setupUi(widget); 

  this->connect(this->ComboBox, SIGNAL(activated(const QString &)),
                SLOT(onComboBoxActivated(const QString &)));

  this->connect(this->HistoryButton, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));
  this->connect(this->PreviousButton, SIGNAL(clicked()), SLOT(onPreviousButtonClicked()));
  this->connect(this->NextButton, SIGNAL(clicked()), SLOT(onNextButtonClicked()));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::addModules(const QStringList& moduleNames)
{
  QStringList titles;

  foreach(const QString& name, moduleNames)
    {
    // Retrieve module title given its name
    titles << qSlicerApplication::application()->moduleManager()->moduleTitle(name);
    }
  titles.sort();
  this->ComboBox->addItems(titles);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::removeModule(const QString& name)
{
  // Retrieve module title given its name
  QString title = qSlicerApplication::application()->moduleManager()->moduleTitle(name);
  this->ComboBox->removeItem(this->ComboBox->findText(title));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::onComboBoxActivated(const QString& title)
{
  CTK_P(qSlicerModuleSelectorWidget);
  
  // Retrieve module name given its title
  QString name = qSlicerApplication::application()->moduleManager()->moduleName(title);
  emit p->moduleSelected(name);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::onHistoryButtonClicked()
{
  qDebug() << "qSlicerModuleSelectorWidgetPrivate::onHistoryButtonClicked - Not implemented";
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::onPreviousButtonClicked()
{
  qDebug() << "qSlicerModuleSelectorWidgetPrivate::onPreviousButtonClicked - Not implemented";
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::onNextButtonClicked()
{
  qDebug() << "qSlicerModuleSelectorWidgetPrivate::onNextButtonClicked - Not implemented";
}
