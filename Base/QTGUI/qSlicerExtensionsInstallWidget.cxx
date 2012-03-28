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

// Qt includes
#include <QSettings>
#include <QWebFrame>

// CTK includes
#include <ctkPimpl.h>

// QtGUI includes
#include "qSlicerExtensionsInstallWidget.h"
#include "qSlicerExtensionsManagerModel.h"
#include "ui_qSlicerExtensionsInstallWidget.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsInstallWidgetPrivate: public Ui_qSlicerExtensionsInstallWidget
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsInstallWidget);
protected:
  qSlicerExtensionsInstallWidget* const q_ptr;

public:
  qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object);

  void init();

  /// Return the URL allowing to retrieve the extension list page
  /// associated with the current architecture, operating system and slicer revision.
  QUrl extensionsListUrl();

  void setFailurePage(const QUrl &faultyUrl);

  /// Convenient function to return the mainframe
  QWebFrame* mainFrame();

  /// Convenient function to evaluate JS in main frame context
  QString evalJS(const QString &js);

  qSlicerExtensionsManagerModel * ExtensionsManagerModel;

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;
};

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidgetPrivate::qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object)
  :q_ptr(&object)
{
  this->ExtensionsManagerModel = 0;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsInstallWidget);

  this->setupUi(q);

  QObject::connect(this->WebView, SIGNAL(loadFinished(bool)),
                   q, SLOT(onLoadFinished(bool)));

  QObject::connect(this->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                   q, SLOT(initJavascript()));
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsInstallWidgetPrivate::extensionsListUrl()
{
  QUrl url(this->ExtensionsManagerModel->serverUrlWithExtensionsStorePath());
  url.setQueryItems(QList<QPair<QString, QString> >()
                    << QPair<QString, QString>("layout", "empty")
                    << QPair<QString, QString>("os", this->SlicerOs)
                    << QPair<QString, QString>("arch", this->SlicerArch)
                    << QPair<QString, QString>("revision", this->SlicerRevision));
  return url;
}

// --------------------------------------------------------------------------
QWebFrame* qSlicerExtensionsInstallWidgetPrivate::mainFrame()
{
  return this->WebView->page()->mainFrame();
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionsInstallWidgetPrivate::evalJS(const QString &js)
{
  return this->mainFrame()->evaluateJavaScript(js).toString();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::setFailurePage(const QUrl& faultyUrl)
{
  QString html =
      "<style type='text/css'>"
      "  div.viewWrapperSlicer{"
      "    font-family:'Lucida Grande','Lucida Sans Unicode',helvetica,arial,Verdana,sans-serif;"
      "    font-size:13px;margin-left:8px;color:#777777;"
      "  }"
      "  div.extensionsHeader,div.extensionsBody{margin-right:10px;}"
      "  div.extensionsHeader{height:45px;border-bottom:1px solid #d0d0d0;}"
      "  div.extensionsTitle{float:left;font-size:24px;font-weight:bolder;margin-top:10px;}"
      "  div.extensionsBodyLeftColumn{float:left;width:230px;border-right:1px solid #d0d0d0;min-height:450px;}"
      "  div.extensionsBodyRightColumn{margin-left:230px;}"
      "</style>"
      "<div class='viewWrapperSlicer'>"
      "  <div class='extensionsHeader'>"
      "    <div class='extensionsTitle'>Slicer Extensions</div>"
      "  </div>"
      "  <div class='extensionsBody'>"
      "    <p>Failed to load extension page using the following URL:<br>%1</p>"
      "  </div>"
      "</div>";

  this->WebView->setHtml(html.arg(faultyUrl.toString()));
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::qSlicerExtensionsInstallWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsInstallWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::~qSlicerExtensionsInstallWidget()
{
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsInstallWidget::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsInstallWidget);
  return d->ExtensionsManagerModel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsInstallWidget);

  if (this->extensionsManagerModel() == model)
    {
    return;
    }

  disconnect(this, SLOT(onExtensionInstalled(QString)));
  disconnect(this, SLOT(onExtensionUninstalled(QString)));
  disconnect(this, SLOT(onSlicerRequirementsChanged(QString,QString,QString)));

  d->ExtensionsManagerModel = model;

  if (model)
    {
    this->onSlicerRequirementsChanged(
          model->slicerRevision(), model->slicerOs(), model->slicerArch());

    QObject::connect(model, SIGNAL(extensionInstalled(QString)),
                     this, SLOT(onExtensionInstalled(QString)));

    QObject::connect(model, SIGNAL(extensionUninstalled(QString)),
                     this, SLOT(onExtensionUninstalled(QString)));

    QObject::connect(model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)),
                     this, SLOT(onSlicerRequirementsChanged(QString,QString,QString)));
    }
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerRevision, SlicerRevision)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerRevision, SlicerRevision)

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerOs, SlicerOs)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerOs, SlicerOs)

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerArch, SlicerArch)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerArch, SlicerArch)

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::refresh()
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->WebView->setUrl(d->extensionsListUrl());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'uninstall')").arg(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionUninstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'install')").arg(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onSlicerRequirementsChanged(const QString& revision,const QString& os,const QString& arch)
{
  this->setSlicerRevision(revision);
  this->setSlicerOs(os);
  this->setSlicerArch(arch);
  this->refresh();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::initJavascript()
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->mainFrame()->addToJavaScriptWindowObject("extensions_manager_model", d->ExtensionsManagerModel);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLoadFinished(bool ok)
{
  Q_D(qSlicerExtensionsInstallWidget);
  if(!ok)
    {
    d->setFailurePage(d->extensionsListUrl().toString());
    }
}

