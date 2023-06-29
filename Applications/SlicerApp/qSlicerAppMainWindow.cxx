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
#include <QStyleFactory>
#include <QUrl>

// CTK includes
#include <ctkWidgetsUtils.h>

// Slicer includes
#include "qSlicerAboutDialog.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerActionsDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerErrorReportDialog.h"
#include "qSlicerModuleManager.h"

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
  this->HelpKeyboardShortcutsAction = new QAction(mainWindow);
  this->HelpKeyboardShortcutsAction->setObjectName("HelpKeyboardShortcutsAction");
  this->HelpKeyboardShortcutsAction->setText(qSlicerAppMainWindow::tr("&Keyboard Shortcuts Reference"));
  this->HelpKeyboardShortcutsAction->setToolTip(qSlicerAppMainWindow::tr("Raise a window that lists commonly-used keyboard shortcuts."));

  this->HelpDocumentationAction = new QAction(mainWindow);
  this->HelpDocumentationAction->setObjectName("HelpDocumentationAction");
  this->HelpDocumentationAction->setText(qSlicerAppMainWindow::tr("Documentation"));
  this->HelpDocumentationAction->setShortcut(QKeySequence(qSlicerAppMainWindow::tr("Ctrl+1", "Documentation")));

  QAction* helpQuickStartAction = new QAction(mainWindow);
  helpQuickStartAction->setObjectName("HelpQuickStartAction");
  helpQuickStartAction->setText(qSlicerAppMainWindow::tr("Quick Start"));

  QAction* helpGetHelpAction = new QAction(mainWindow);
  helpGetHelpAction->setObjectName("HelpGetHelpAction");
  helpGetHelpAction->setText(qSlicerAppMainWindow::tr("Get Help"));

  QAction* helpUserInterfaceAction = new QAction(mainWindow);
  helpUserInterfaceAction->setObjectName("HelpUserInterfaceAction");
  helpUserInterfaceAction->setText(qSlicerAppMainWindow::tr("User Interface"));

  this->HelpVisitSlicerForumAction = new QAction(mainWindow);
  this->HelpVisitSlicerForumAction->setObjectName("HelpVisitSlicerForumAction");
  this->HelpVisitSlicerForumAction->setText(qSlicerAppMainWindow::tr("Visit the Slicer Forum"));

  QAction* helpBrowseTutorialsAction = new QAction(mainWindow);
  helpBrowseTutorialsAction->setObjectName("HelpBrowseTutorialsAction");
  helpBrowseTutorialsAction->setText(qSlicerAppMainWindow::tr("Browse Tutorials"));
  helpBrowseTutorialsAction->setToolTip(qSlicerAppMainWindow::tr("Raise the training pages in your favorite web browser"));

  QAction* helpJoinUsOnTwitterAction = new QAction(mainWindow);
  helpJoinUsOnTwitterAction->setObjectName("HelpJoinUsOnTwitterAction");
  helpJoinUsOnTwitterAction->setText(qSlicerAppMainWindow::tr("Join Us on Twitter"));

  QAction* helpSearchFeatureRequestsAction = new QAction(mainWindow);
  helpSearchFeatureRequestsAction->setObjectName("HelpSearchFeatureRequestsAction");
  helpSearchFeatureRequestsAction->setText(qSlicerAppMainWindow::tr("Search Feature Requests"));

  QAction* helpViewLicenseAction = new QAction(mainWindow);
  helpViewLicenseAction->setObjectName("HelpViewLicenseAction");
  helpViewLicenseAction->setText(qSlicerAppMainWindow::tr("View License"));

  QAction* helpHowToCiteAction = new QAction(mainWindow);
  helpHowToCiteAction->setObjectName("HelpHowToCiteAction");
  helpHowToCiteAction->setText(qSlicerAppMainWindow::tr("How to Cite"));

  QAction* helpSlicerPublicationsAction = new QAction(mainWindow);
  helpSlicerPublicationsAction->setObjectName("HelpSlicerPublicationsAction");
  helpSlicerPublicationsAction->setText(qSlicerAppMainWindow::tr("Slicer Publications"));

  QAction* helpAcknowledgmentsAction = new QAction(mainWindow);
  helpAcknowledgmentsAction->setObjectName("HelpAcknowledgmentsAction");
  helpAcknowledgmentsAction->setText(qSlicerAppMainWindow::tr("Acknowledgments"));

  this->HelpReportBugOrFeatureRequestAction = new QAction(mainWindow);
  this->HelpReportBugOrFeatureRequestAction->setObjectName("HelpReportBugOrFeatureRequestAction");
  this->HelpReportBugOrFeatureRequestAction->setText(qSlicerAppMainWindow::tr("Report a Bug"));
  this->HelpReportBugOrFeatureRequestAction->setToolTip(qSlicerAppMainWindow::tr("Report error or request enhancement or new feature."));

  this->HelpAboutSlicerAppAction = new QAction(mainWindow);
  this->HelpAboutSlicerAppAction->setObjectName("HelpAboutSlicerAppAction");
  this->HelpAboutSlicerAppAction->setText(qSlicerAppMainWindow::tr("About 3D Slicer"));
  this->HelpAboutSlicerAppAction->setToolTip(qSlicerAppMainWindow::tr("Provides a description of the Slicer effort and its support."));

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

  this->HelpMenu->addAction(this->HelpDocumentationAction);
  this->HelpMenu->addAction(helpQuickStartAction);
  this->HelpMenu->addAction(helpGetHelpAction);
  this->HelpMenu->addAction(helpUserInterfaceAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(this->HelpKeyboardShortcutsAction);
  this->HelpMenu->addAction(helpBrowseTutorialsAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(this->HelpVisitSlicerForumAction);
  this->HelpMenu->addAction(helpJoinUsOnTwitterAction);
  this->HelpMenu->addAction(helpSearchFeatureRequestsAction);
  this->HelpMenu->addAction(this->HelpReportBugOrFeatureRequestAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(helpViewLicenseAction);
  this->HelpMenu->addAction(helpHowToCiteAction);
  this->HelpMenu->addAction(helpSlicerPublicationsAction);
  this->HelpMenu->addAction(helpAcknowledgmentsAction);
  this->HelpMenu->addSeparator();
  this->HelpMenu->addAction(this->HelpAboutSlicerAppAction);

  //----------------------------------------------------------------------------
  // Icons in the menu
  //----------------------------------------------------------------------------
  // Customize QAction icons with standard pixmaps
  this->updateIconPalette();
  setThemeIcon(this->HelpAboutSlicerAppAction, "help-about");
  setThemeIcon(this->HelpReportBugOrFeatureRequestAction, "tools-report-bug");
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::updateIconPalette()
{
  QPalette palette = qSlicerApplication::application()->palette();
  QStyle* lightStyle = QStyleFactory::create("Light Slicer");
  QString hexColor;
  if (palette == lightStyle->standardPalette())
  {
    hexColor = lightStyle->standardPalette().color(QPalette::Text).name();
  }
  else
  {
    QStyle* darkStyle = QStyleFactory::create("Dark Slicer");
    hexColor = darkStyle->standardPalette().color(QPalette::Text).name();
  }
  this->HelpDocumentationAction->setIcon(ctk::getColorizedIcon(":/Icons/Scalable/OpenExternal.svg", QColor(hexColor)));
  this->HelpKeyboardShortcutsAction->setIcon(ctk::getColorizedIcon(":/Icons/Scalable/Keyboard.svg", QColor(hexColor)));
  this->HelpVisitSlicerForumAction->setIcon(ctk::getColorizedIcon(":/Icons/Scalable/Forum.svg", QColor(hexColor)));
  this->HelpReportBugOrFeatureRequestAction->setIcon(ctk::getColorizedIcon(":/Icons/Scalable/Report.svg", QColor(hexColor)));
  this->HelpAboutSlicerAppAction->setIcon(ctk::getColorizedIcon(":/Icons/Scalable/Info.svg", QColor(hexColor)));
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
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/getting_started.html#tutorials"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpDocumentationAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl()));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpQuickStartAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/getting_started.html#quick-start"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpGetHelpAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/get_help.html"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpUserInterfaceAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/user_interface.html"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpVisitSlicerForumAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://discourse.slicer.org/"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpJoinUsOnTwitterAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://twitter.com/3dslicerapp"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpSearchFeatureRequestsAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://discourse.slicer.org/c/support/feature-requests/9"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpViewLicenseAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://github.com/Slicer/Slicer/blob/main/License.txt"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpHowToCiteAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/about.html#how-to-cite"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpSlicerPublicationsAction_triggered()
{
  QDesktopServices::openUrl(QUrl("https://scholar.google.com/scholar?&as_sdt=1%2C22&as_vis=1&q=%28%223D+Slicer%22+OR+%22slicer+org%22+OR+Slicer3D%29+-Slic3r+&btnG="));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::on_HelpAcknowledgmentsAction_triggered()
{
  QDesktopServices::openUrl(QUrl(qSlicerApplication::application()->documentationBaseUrl() + "/user_guide/about.html#acknowledgments"));
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
void qSlicerAppMainWindow::changeEvent(QEvent* event)
{
  Q_D(qSlicerAppMainWindow);
  this->Superclass::changeEvent(event);
  switch (event->type())
    {
    case QEvent::PaletteChange:
      {
      d->updateIconPalette();
      break;
      }
    default:
      break;
    }
}
