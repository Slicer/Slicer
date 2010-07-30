
// Qt includes
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include "vtkSlicerConfigure.h" // For Slicer3_USE_PYTHONQT

// CTK includes
#ifdef Slicer3_USE_PYTHONQT
#include <ctkPythonShell.h>
#endif

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerAboutDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerMainWindowCore.h" 
#include "qSlicerMainWindowCore_p.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulePanel.h"
#ifdef Slicer3_USE_PYTHONQT
#include "qSlicerPythonManager.h"
#endif

// MRML includes
#include <vtkMRMLScene.h>

//---------------------------------------------------------------------------
// qSlicerMainWindowCorePrivate methods

//---------------------------------------------------------------------------
qSlicerMainWindowCorePrivate::qSlicerMainWindowCorePrivate()
  {
  this->PythonShell = 0; 
  }

//-----------------------------------------------------------------------------
// qSlicerMainWindowCore methods

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(qSlicerMainWindow* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerMainWindowCore);
  CTK_D(qSlicerMainWindowCore);
  
  d->ParentWidget = _parent;
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindowCore, qSlicerMainWindow*, widget, ParentWidget);


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
qSlicerMainWindowCore_onViewLayout_implementation(SideBySideCompare);

#undef qSlicerMainWindowCore_onViewLayout_implementation

//-----------------------------------------------------------------------------
void qSlicerMainWindowCore::onWindowPythonInteractorActionTriggered()
{
#ifdef Slicer3_USE_PYTHONQT
  CTK_D(qSlicerMainWindowCore);
  if (!d->PythonShell)
    {
    Q_ASSERT(qSlicerApplication::application()->pythonManager());
    d->PythonShell = new ctkPythonShell(qSlicerApplication::application()->pythonManager()/*, d->ParentWidget*/);
    d->PythonShell->setAttribute(Qt::WA_QuitOnClose, false);
    d->PythonShell->resize(600, 280);
    }
  Q_ASSERT(d->PythonShell);
  d->PythonShell->show();
  d->PythonShell->raise();
#endif
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
  qSlicerAboutDialog about(0);
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
