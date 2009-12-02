#include "qSlicerModuleSelectorWidget.h"

// qCTK includes

// QT includes
#include <QComboBox>
#include <QHBoxLayout>

//---------------------------------------------------------------------------
struct qSlicerModuleSelectorWidgetPrivate: public qCTKPrivate<qSlicerModuleSelectorWidget>
{
  QCTK_DECLARE_PUBLIC(qSlicerModuleSelectorWidget);
  
  qSlicerModuleSelectorWidgetPrivate()
    {
    this->ComboBox = 0;
    }
    
  void setupUi(QWidget* widget);
  
  void removeModule(const QString& name);
  
  QComboBox * ComboBox; 
};

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
  
  this->ComboBox = new QComboBox;
  
  QHBoxLayout * layout = new QHBoxLayout;
  layout->addWidget(this->ComboBox);
  
  widget->setLayout(layout);

  QObject::connect(this->ComboBox, SIGNAL(activated(const QString &)),
                   p, SIGNAL(moduleSelected(const QString &)));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorWidgetPrivate::removeModule(const QString& name)
{
  this->ComboBox->removeItem(this->ComboBox->findText(name));
}
