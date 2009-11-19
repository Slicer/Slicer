#include "QtSlicerWebKit.h"

// Qt includes
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QtWebKit>
#include "qslicerfactory.h"

#include "vtkKWApplication.h"

QtSlicerWebKit::QtSlicerWebKit(QWidget* p, vtkKWApplication *kwapp)
  : QWidget(p)
{

  this->kwapp = kwapp;

  this->group = new QGroupBox();
  this->group->setTitle("WebView");
  this->boxLayout = new QVBoxLayout(this->group);
  this->group->setLayout(this->boxLayout);

  this->lineEdit = new QLineEdit(this->group);
  this->boxLayout->addWidget(this->lineEdit);

  QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
  this->webView = new QWebView(this->group);
  this->boxLayout->addWidget(this->webView);

  // this factory handles both the text/qvtk and application/qslicer mime type plugins
  this->factory = new QSlicerFactory(webView, 0, kwapp);
  this->webView->page()->setPluginFactory(this->factory);

  this->group->show();
  this->group->raise();
  this->group->activateWindow();

  /*
  QObject::connect(this->Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(sliderChanged(int)));
  QObject::connect(this->LineEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(textChanged(const QString&)));
  QObject::connect(this->LineEdit, SIGNAL(editingFinished()),
                   this, SLOT(editingFinished()));
  */

}

//-----------------------------------------------------------------------------
QtSlicerWebKit::~QtSlicerWebKit()
{
}

//-----------------------------------------------------------------------------
const char *QtSlicerWebKit::url() const
{
  //return this->webView->;
  return (NULL);
}

//-----------------------------------------------------------------------------
void QtSlicerWebKit::setURL(const char* url)
{

  this->webView->load(QUrl(url));
  //emit this->valueChanged(this->Value);
}

//-----------------------------------------------------------------------------
  /*
void QtSlicerWebKit::urlEdited(const QString& url)
{
  if(!this->BlockUpdate)
    {
    double val = text.toDouble();
    this->BlockUpdate = true;
    double range = this->Maximum - this->Minimum;
    double fraction = (val - this->Minimum) / range;
    int sliderVal = qRound(fraction * static_cast<double>(this->Resolution));
    this->Slider->setValue(sliderVal);
    this->setValue(val);
    this->BlockUpdate = false;
    }
}
    */
