/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
#ifdef Slicer_USE_PYTHONQT
#include <ctkPythonShell.h>
#endif

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerAboutDialog.h"
#include "qSlicerActionsDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerMainWindowCore.h" 
#include "qSlicerMainWindowCore_p.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulePanel.h"
#ifdef Slicer_USE_PYTHONQT
#include "qSlicerPythonManager.h"
#endif

// MRML includes
#include <vtkMRMLScene.h>

//---------------------------------------------------------------------------
// qSlicerMainWindowCorePrivate methods

//---------------------------------------------------------------------------
qSlicerMainWindowCorePrivate::qSlicerMainWindowCorePrivate()
  {
#ifdef Slicer_USE_PYTHONQT
  this->PythonShell = 0; 
#endif
  }

//---------------------------------------------------------------------------
qSlicerMainWindowCorePrivate::~qSlicerMainWindowCorePrivate()
{
#ifdef Slicer_USE_PYTHONQT
  if (this->PythonShell)
    {
    delete this->PythonShell;
    }
#endif
}

//-----------------------------------------------------------------------------
// qSlicerMainWindowCore methods

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(qSlicerMainWindow* _parent):Superclass(_parent)
  , d_ptr(new qSlicerMainWindowCorePrivate)
{
  Q_D(qSlicerMainWindowCore);
  
  d->ParentWidget = _parent;
}

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::~qSlicerMainWindowCore()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerMainWindowCore, qSlicerMainWindow*, widget, ParentWidget);

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileAddDataActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileImportSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileLoadSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileAddVolumeActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddVolumeDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileAddTransformActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddTransformDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileSaveSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openSaveDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileCloseSceneActionTriggered()
{
  qSlicerCoreApplication::application()->mrmlScene()->Clear(false);
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onEditUndoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Undo();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onEditRedoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Redo();
}

//---------------------------------------------------------------------------
#define qSlicerMainWindowCore_onViewLayout_implementation(_NAME)                  \
  void qSlicerMainWindowCore::onViewLayout##_NAME##ActionTriggered()              \
  {                                                                               \
    qSlicerApplication::application()->layoutManager()->switchTo##_NAME##View();  \
  }
  
qSlicerMainWindowCore_onViewLayout_implementation(Conventional);
qSlicerMainWindowCore_onViewLayout_implementation(FourUp);
qSlicerMainWindowCore_onViewLayout_implementation(Dual3D);
qSlicerMainWindowCore_onViewLayout_implementation(OneUp3D);
qSlicerMainWindowCore_onViewLayout_implementation(OneUpRedSlice);
qSlicerMainWindowCore_onViewLayout_implementation(OneUpYellowSlice);
qSlicerMainWindowCore_onViewLayout_implementation(OneUpGreenSlice);
qSlicerMainWindowCore_onViewLayout_implementation(Tabbed3D);
qSlicerMainWindowCore_onViewLayout_implementation(TabbedSlice);
qSlicerMainWindowCore_onViewLayout_implementation(Compare);
qSlicerMainWindowCore_onViewLayout_implementation(SideBySideLightbox);

#undef qSlicerMainWindowCore_onViewLayout_implementation

//-----------------------------------------------------------------------------
void qSlicerMainWindowCore::onWindowPythonInteractorActionTriggered()
{
#ifdef Slicer_USE_PYTHONQT
  Q_D(qSlicerMainWindowCore);
  if (!d->PythonShell)
    {
    Q_ASSERT(qSlicerApplication::application()->pythonManager());
    d->PythonShell = new ctkPythonShell(qSlicerApplication::application()->pythonManager());
    d->PythonShell->setAttribute(Qt::WA_QuitOnClose, false);
    d->PythonShell->resize(600, 280);
    }
  Q_ASSERT(d->PythonShell);
  d->PythonShell->show();
  d->PythonShell->activateWindow();
  d->PythonShell->raise();
#endif
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onHelpKeyboardShortcutsActionTriggered()
{
  qSlicerActionsDialog actionsDialog(this->widget());
  actionsDialog.setActionsWithNoShortcutVisible(false);
  actionsDialog.setMenuActionsVisible(false);
  actionsDialog.addActions(this->widget()->findChildren<QAction*>(), "Slicer Application");

  // scan the modules for their actions
  QList<QAction*> moduleActions;
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  foreach(const QString& moduleName, moduleManager->moduleList())
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
void qSlicerMainWindowCore::onHelpBrowseTutorialsActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer3.6:Training"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onHelpInterfaceDocumentationActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Documentation"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onHelpSlicerPublicationsActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/publications"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onHelpAboutSlicerQTActionTriggered()
{
  qSlicerAboutDialog about(this->widget());
  about.exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFeedbackReportBugActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.na-mic.org/Bug/index.php"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFeedbackReportUsabilityIssueActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.na-mic.org/Bug/index.php"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFeedbackMakeFeatureRequestActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.na-mic.org/Bug/index.php"));
}
//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFeedbackCommunitySlicerVisualBlogActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer3:VisualBlog"));
}
