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
#include <QAction>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
#include <ctkErrorLogWidget.h>
#include <ctkMessageBox.h>
#ifdef Slicer_USE_PYTHONQT
#include <ctkPythonConsole.h>
#endif


// SlicerApp includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAppAboutDialog.h"
#include "qSlicerActionsDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerAppMainWindowCore_p.h"
#include "qSlicerModuleManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
#endif
#include "qMRMLUtils.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

//---------------------------------------------------------------------------
// qSlicerAppMainWindowCorePrivate methods

//---------------------------------------------------------------------------
qSlicerAppMainWindowCorePrivate::qSlicerAppMainWindowCorePrivate()
  {
#ifdef Slicer_USE_PYTHONQT
  this->PythonConsole = 0;
#endif
  this->ErrorLogWidget = 0;
  }

//---------------------------------------------------------------------------
qSlicerAppMainWindowCorePrivate::~qSlicerAppMainWindowCorePrivate()
{
  delete this->ErrorLogWidget;
}

//-----------------------------------------------------------------------------
// qSlicerAppMainWindowCore methods

//-----------------------------------------------------------------------------
qSlicerAppMainWindowCore::qSlicerAppMainWindowCore(qSlicerAppMainWindow* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAppMainWindowCorePrivate)
{
  Q_D(qSlicerAppMainWindowCore);
  
  d->ParentWidget = _parent;
  d->ErrorLogWidget = new ctkErrorLogWidget;
  d->ErrorLogWidget->setErrorLogModel(
    qSlicerCoreApplication::application()->errorLogModel());
}

//-----------------------------------------------------------------------------
qSlicerAppMainWindowCore::~qSlicerAppMainWindowCore()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAppMainWindowCore, qSlicerAppMainWindow*, widget, ParentWidget);

#ifdef Slicer_USE_PYTHONQT
//---------------------------------------------------------------------------
ctkPythonConsole* qSlicerAppMainWindowCore::pythonConsole()const
{
  Q_D(const qSlicerAppMainWindowCore);
  if (!d->PythonConsole)
    {
    // Lookup reference of 'PythonConsole' widget
    // and cache the value
    foreach(QWidget * widget, qApp->topLevelWidgets())
      {
      if(widget->objectName().compare(QLatin1String("pythonConsole")) == 0)
        {
        const_cast<qSlicerAppMainWindowCorePrivate*>(d)
          ->PythonConsole = qobject_cast<ctkPythonConsole*>(widget);
        break;
        }
      }
    }
  return d->PythonConsole;
}
#endif

//---------------------------------------------------------------------------
ctkErrorLogWidget* qSlicerAppMainWindowCore::errorLogWidget()const
{
  Q_D(const qSlicerAppMainWindowCore);
  return d->ErrorLogWidget;
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileAddDataActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileLoadDataActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}


//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileImportSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileLoadSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileAddVolumeActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddVolumesDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileAddTransformActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openAddTransformDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileSaveSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openSaveDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onSDBSaveToDirectoryActionTriggered()
{
   Q_D(qSlicerAppMainWindowCore);
  // open a file dialog to let the user choose where to save
  QString tempDir = qSlicerCoreApplication::application()->temporaryPath();
  QString saveDirName = QFileDialog::getExistingDirectory(this->widget(), tr("Slicer Data Bundle Directory (All Existing Files in this Directory Will Be Removed!)"), tempDir, QFileDialog::ShowDirsOnly);
  //qDebug() << "saveDirName = " << qPrintable(saveDirName);
  if (saveDirName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }
  // double check that user is sure they want to save to this directory if
  // there are already files in it
  QDir testSaveDir = QDir(saveDirName);
  int numFiles = testSaveDir.count() - 2;
  if (numFiles > 0)
    {
    ctkMessageBox *emptyMessageBox = new ctkMessageBox(d->ParentWidget);
    QString plurals = QString("\ncontains ");
    if (numFiles == 1)
      {
      plurals += QString("1 file or directory.\n");
      }
    else
      {
      plurals += QString("%1 files or directories.\n").arg(numFiles);
      }
    QString message = QString("Selected directory\n" + saveDirName + plurals +
                              "Please choose an empty directory.");
    emptyMessageBox->setAttribute( Qt::WA_DeleteOnClose, true );
    emptyMessageBox->setIcon(QMessageBox::Warning);
    emptyMessageBox->setText(message);
    emptyMessageBox->exec();
    return;
    }
  
  // pass in a screen shot
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewport();
  QPixmap screenShot = QPixmap::grabWidget(widget);
  // convert to vtkImageData
  vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
  qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), imageData);

  const char *retval = qSlicerCoreApplication::application()->applicationLogic()->SaveSceneToSlicerDataBundleDirectory(saveDirName.toLatin1(), imageData);
  if (retval)
    {
    QString returnFileName = QString(retval);
    std::cout << "Saved scene to file " << qPrintable(returnFileName) << std::endl;
    }
  else
    {
    std::cerr << "Error saving scene to file!" << std::endl;
    }
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onSDBZipDirectoryActionTriggered()
{
  // NOT IMPLEMENTED YET
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onSDBZipToDCMActionTriggered()
{
  // NOT IMPLEMENTED YET
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileCloseSceneActionTriggered()
{
  qSlicerCoreApplication::application()->mrmlScene()->Clear(false);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onEditUndoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Undo();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onEditRedoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Redo();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::setLayout(int layout)
{
  qSlicerApplication::application()->layoutManager()->setLayout(layout);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::setLayoutNumberOfCompareViewRows(int num)
{
  qSlicerApplication::application()->layoutManager()->setLayoutNumberOfCompareViewRows(num);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::setLayoutNumberOfCompareViewColumns(int num)
{
  qSlicerApplication::application()->layoutManager()->setLayoutNumberOfCompareViewColumns(num);
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onWindowErrorLogActionTriggered(bool show)
{
  Q_D(qSlicerAppMainWindowCore);
  if (show)
    {
    d->ErrorLogWidget->show();
    d->ErrorLogWidget->activateWindow();
    d->ErrorLogWidget->raise();
    }
  else
    {
    d->ErrorLogWidget->close();
    }
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onWindowPythonInteractorActionTriggered(bool show)
{
#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole* console = this->pythonConsole();
  Q_ASSERT(console);
  if (show)
    {
    console->show();
    console->activateWindow();
    console->raise();
    }
  else
    {
    console->close();
    }
#endif
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpKeyboardShortcutsActionTriggered()
{
  qSlicerActionsDialog actionsDialog(this->widget());
  actionsDialog.setActionsWithNoShortcutVisible(false);
  actionsDialog.setMenuActionsVisible(false);
  actionsDialog.addActions(this->widget()->findChildren<QAction*>(), "Slicer Application");

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
void qSlicerAppMainWindowCore::onHelpBrowseTutorialsActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer3.6:Training"));
}
//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpInterfaceDocumentationActionTriggered()
{
  QDesktopServices::openUrl(QUrl(QString("http://wiki.slicer.org/slicerWiki/index.php/Documentation/%1.%2").arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR)));
}
//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpSlicerPublicationsActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/publications"));
}
//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpAboutSlicerAppActionTriggered()
{
  qSlicerAppAboutDialog about(this->widget());
  about.exec();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpReportBugOrFeatureRequestActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.na-mic.org/Bug/index.php"));
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onHelpVisualBlogActionTriggered()
{
  QDesktopServices::openUrl(QUrl("http://www.slicer.org/slicerWiki/index.php/Slicer4:VisualBlog"));
}
