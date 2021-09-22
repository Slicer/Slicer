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

#include "qSlicerAppMainWindow.h"
#include "qSlicerAppMainWindow_p.h"

// Qt includes
#include <QDesktopServices>
#include <QLabel>
#include <QPixmap>
#include <QStyle>
#include <QUrl>

// Slicer includes
#include "qSlicerAboutDialog.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerActionsDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerErrorReportDialog.h"
#include "qSlicerModuleManager.h"
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_MAJOR,Slicer_VERSION_MINOR

namespace
{

//-----------------------------------------------------------------------------
void setThemeIcon(QAction* action, const QString& name)
{
  action->setIcon(QIcon::fromTheme(name, action->icon()));
}

} // end of anonymous namespace

//-----------------------------------------------------------------------------
// qSlicerAppMainWindowPrivate methods

qSlicerAppMainWindowPrivate::qSlicerAppMainWindowPrivate(qSlicerAppMainWindow& object)
  : Superclass(object)
{
}

//-----------------------------------------------------------------------------
qSlicerAppMainWindowPrivate::~qSlicerAppMainWindowPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::init()
{
  Q_Q(qSlicerMainWindow);
  this->Superclass::init();
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  //----------------------------------------------------------------------------
  // Add actions
  //----------------------------------------------------------------------------
  QAction* helpKeyboardShortcutsAction = new QAction(mainWindow);
  helpKeyboardShortcutsAction->setObjectName("HelpKeyboardShortcutsAction");
  helpKeyboardShortcutsAction->setText(qSlicerAppMainWindow::tr("&Keyboard Shortcuts"));
  helpKeyboardShortcutsAction->setToolTip(qSlicerAppMainWindow::tr("Raise a window that lists commonly-used keyboard shortcuts."));

  QAction* helpInterfaceDocumentationAction = new QAction(mainWindow);
  helpInterfaceDocumentationAction->setObjectName("HelpInterfaceDocumentationAction");
  helpInterfaceDocumentationAction->setText(qSlicerAppMainWindow::tr("Interface Documentation"));
  helpInterfaceDocumentationAction->setShortcut(QKeySequence(qSlicerAppMainWindow::tr("Ctrl+1", "Interface Documentation")));

  QAction* helpBrowseTutorialsAction = new QAction(mainWindow);
  helpBrowseTutorialsAction->setObjectName("HelpBrowseTutorialsAction");
  helpBrowseTutorialsAction->setText(qSlicerAppMainWindow::tr("Browse tutorials"));
  helpBrowseTutorialsAction->setToolTip(qSlicerAppMainWindow::tr("Raise the training pages in your favorite web browser"));

  QAction* helpSlicerPublicationsAction = new QAction(mainWindow);
  helpSlicerPublicationsAction->setObjectName("HelpSlicerPublicationsAction");
  helpSlicerPublicationsAction->setText(qSlicerAppMainWindow::tr("Slicer Publications"));

  QAction* helpVisualBlogAction = new QAction(mainWindow);
  helpVisualBlogAction->setObjectName("HelpVisualBlogAction");
  helpVisualBlogAction->setText(qSlicerAppMainWindow::tr("Visual Blog"));
  helpVisualBlogAction->setToolTip(qSlicerAppMainWindow::tr("Open the Slicer Visual Blog using your favorite web browser where you can post screenshots of interesting ways you are using the Slicer software package in your research and share them with the Slicer community."));

  QAction* helpReportBugOrFeatureRequestAction = new QAction(mainWindow);
  helpReportBugOrFeatureRequestAction->setObjectName("HelpReportBugOrFeatureRequestAction");
  helpReportBugOrFeatureRequestAction->setText(qSlicerAppMainWindow::tr("Report a bug"));
  helpReportBugOrFeatureRequestAction->setToolTip(qSlicerAppMainWindow::tr("Report error or request enhancement or new feature."));

  QAction* helpAboutSlicerAppAction = new QAction(mainWindow);
  helpAboutSlicerAppAction->setObjectName("HelpAboutSlicerAppAction");
  helpAboutSlicerAppAction->setText(qSlicerAppMainWindow::tr("About 3D Slicer"));
  helpAboutSlicerAppAction->setToolTip(qSlicerAppMainWindow::tr("Provides a description of the Slicer effort and its support."));

  //----------------------------------------------------------------------------
  // Calling "setupUi()" after adding the actions above allows the call
  // to "QMetaObject::connectSlotsByName()" done in "setupUi()" to
  // successfully connect each slot with its corresponding action.
  this->Superclass::setupUi(mainWindow);

  //----------------------------------------------------------------------------
  // Configure
  //----------------------------------------------------------------------------
  mainWindow->setWindowTitle("3D Slicer");
  mainWindow->setWindowIcon(QIcon(":/Icons/Medium/DesktopIcon.png"));

  QLabel* logoLabel = new QLabel();
  logoLabel->setObjectName("LogoLabel");
  logoLabel->setPixmap(qMRMLWidget::pixmapFromIcon(QIcon(":/ModulePanelLogo.png")));
  this->PanelDockWidget->setTitleBarWidget(logoLabel);

  this->HelpMenu->addAction(helpKeyboardShortcutsAction);
  this->HelpMenu->addAction(helpInterfaceDocumentationAction);
  this->HelpMenu->addAction(helpBrowseTutorialsAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(helpSlicerPublicationsAction);
  this->HelpMenu->addAction(helpVisualBlogAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(helpReportBugOrFeatureRequestAction);
  this->HelpMenu->addAction(helpAboutSlicerAppAction);

  //----------------------------------------------------------------------------
  // Icons in the menu
  //----------------------------------------------------------------------------
  // Customize QAction icons with standard pixmaps
  // TODO: all these icons are a little bit too much.
  QIcon networkIcon = mainWindow->style()->standardIcon(QStyle::SP_DriveNetIcon);
  QIcon informationIcon = mainWindow->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  QIcon questionIcon = mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion);

  helpBrowseTutorialsAction->setIcon(networkIcon);
  helpInterfaceDocumentationAction->setIcon(networkIcon);
  helpSlicerPublicationsAction->setIcon(networkIcon);
  helpAboutSlicerAppAction->setIcon(informationIcon);
  helpReportBugOrFeatureRequestAction->setIcon(questionIcon);
  helpVisualBlogAction->setIcon(networkIcon);

  setThemeIcon(helpAboutSlicerAppAction, "help-about");
  setThemeIcon(helpReportBugOrFeatureRequestAction, "tools-report-bug");
}

//-----------------------------------------------------------------------------
// qSlicerAppMainWindow methods

//-----------------------------------------------------------------------------
qSlicerAppMainWindow::qSlicerAppMainWindow(QWidget *_parent)
  : Superclass(new qSlicerAppMainWindowPrivate(*this), _parent)
{
  Q_D(qSlicerAppMainWindow);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerAppMainWindow::qSlicerAppMainWindow(qSlicerAppMainWindowPrivate* pimpl,
                                           QWidget* windowParent)
  : Superclass(pimpl, windowParent)
{
  // init() is called by derived class.
}

//-----------------------------------------------------------------------------
qSlicerAppMainWindow::~qSlicerAppMainWindow() = default;

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpKeyboardShortcutsAction_triggered()
{
  qSlicerActionsDialog actionsDialog(this);
  actionsDialog.setActionsWithNoShortcutVisible(false);
  actionsDialog.setMenuActionsVisible(false);
  actionsDialog.addActions(this->findChildren<QAction*>(), "Slicer Application");

  // scan the modules for their actions
  QList<QAction*> moduleActions;
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  foreach(const QString& moduleName, moduleManager->modulesNames())
    {
    qSlicerAbstractModule* module =
      qobject_cast<qSlicerAbstractModule*>(moduleManager->module(moduleName));
    if (module)
      {
      moduleActions << module->action();
      }
    }
  if (moduleActions.size())
    {
    actionsDialog.addActions(moduleActions, "Modules");
    }
  // TODO add more actions
  actionsDialog.exec();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpBrowseTutorialsAction_triggered()
{
  QString url;

  // TODO: update these URLs when tutorial main page is moved to ReadTheDocs.
  if (qSlicerApplication::application()->releaseType() == "Stable")
    {
    url = QString("http://www.slicer.org/slicerWiki/index.php/Documentation/%1.%2/Training")
                    .arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
    }
  else
    {
    url = QString("http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Training");
    }
  QDesktopServices::openUrl(QUrl(url));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpInterfaceDocumentationAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl()));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpSlicerPublicationsAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://scholar.google.com/scholar?&as_sdt=1%2C22&as_vis=1&q=%28%223D+Slicer%22+OR+%22slicer+org%22+OR+Slicer3D%29+-Slic3r+&btnG="));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpAboutSlicerAppAction_triggered()
{
  qSlicerAboutDialog about(this);
  about.setLogo(QPixmap(":/Logo.png"));
  about.exec();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpReportBugOrFeatureRequestAction_triggered()
{
  qSlicerErrorReportDialog errorReport(this);
  errorReport.exec();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpVisualBlogAction_triggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer4:VisualBlog"));
}
