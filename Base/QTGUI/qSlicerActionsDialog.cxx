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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QGridLayout>
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
#include <QWebView>
#else
#include <QWebEngineView>
#endif

// SlicerQt includes
#include "qSlicerActionsDialog.h"
#include "ui_qSlicerActionsDialog.h"
#include "vtkSlicerVersionConfigure.h"

//-----------------------------------------------------------------------------
class qSlicerActionsDialogPrivate: public Ui_qSlicerActionsDialog
{
  Q_DECLARE_PUBLIC(qSlicerActionsDialog);
protected:
  qSlicerActionsDialog* const q_ptr;

public:
  qSlicerActionsDialogPrivate(qSlicerActionsDialog& object);
  void init();

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
  QWebView* WebView;
#else
  QWebEngineView* WebView;
#endif

};

// --------------------------------------------------------------------------
qSlicerActionsDialogPrivate::qSlicerActionsDialogPrivate(qSlicerActionsDialog& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerActionsDialogPrivate::init()
{
  Q_Q(qSlicerActionsDialog);

  this->setupUi(q);
#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
  this->WebView = new QWebView();
#else
  this->WebView = new QWebEngineView();
#endif
  this->WebView->setObjectName("WebView");
  this->gridLayout->addWidget(this->WebView, 0, 0);
  QString shortcutsUrl =
    QString("http://wiki.slicer.org/slicerWiki/index.php/Documentation/%1.%2/")
    .arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
  shortcutsUrl += "SlicerApplication/MouseandKeyboardShortcuts";
  this->WebView->setUrl( shortcutsUrl );
}

//------------------------------------------------------------------------------
qSlicerActionsDialog::qSlicerActionsDialog(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerActionsDialogPrivate(*this))
{
  Q_D(qSlicerActionsDialog);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerActionsDialog::~qSlicerActionsDialog()
{
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::addAction(QAction* action, const QString& group)
{
  Q_D(qSlicerActionsDialog);
  d->ActionsWidget->addAction(action, group);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::addActions(const QList<QAction*>& actions,
                                      const QString& group)
{
  Q_D(qSlicerActionsDialog);
  d->ActionsWidget->addActions(actions, group);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::clear()
{
  Q_D(qSlicerActionsDialog);
  d->ActionsWidget->clear();
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::setActionsWithNoShortcutVisible(bool visible)
{
  Q_D(qSlicerActionsDialog);
  d->ActionsWidget->setActionsWithNoShortcutVisible(visible);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::setMenuActionsVisible(bool visible)
{
  Q_D(qSlicerActionsDialog);
  d->ActionsWidget->setMenuActionsVisible(visible);
}
