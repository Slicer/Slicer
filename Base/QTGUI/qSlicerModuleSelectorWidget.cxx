#include "qSlicerModuleSelectorWidget.h"

#include "qSlicerModuleSelectorWidget_p.h"

// qCTK includes

// QT includes
#include <QDebug>

//---------------------------------------------------------------------------
qSlicerModuleSelectorWidget::qSlicerModuleSelectorWidget(QWidget* parent)
  :Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerModuleSelectorWidget);
  QCTK_D(qSlicerModuleSelectorWidget);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidget::addModules(const QStringList& moduleNames)
{
  QCTK_D(qSlicerModuleSelectorWidget);
  d->ComboBox->addItems(moduleNames);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidget::removeModule(const QString& name)
{
  QCTK_D(qSlicerModuleSelectorWidget);
  d->removeModule(name);
}

//---------------------------------------------------------------------------
// qSlicerModuleSelectorWidgetPrivate methods

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::setupUi(QWidget* widget)
{
  QCTK_P(qSlicerModuleSelectorWidget);

  this->Ui_qSlicerModuleSelectorWidget::setupUi(widget); 

  QObject::connect(this->ComboBox, SIGNAL(activated(const QString &)),
                   p, SIGNAL(moduleSelected(const QString &)));

  this->connect(this->HistoryButton, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));
  this->connect(this->PreviousButton, SIGNAL(clicked()), SLOT(onPreviousButtonClicked()));
  this->connect(this->NextButton, SIGNAL(clicked()), SLOT(onNextButtonClicked()));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::removeModule(const QString& name)
{
  this->ComboBox->removeItem(this->ComboBox->findText(name));
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
