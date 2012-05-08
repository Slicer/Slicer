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
#include <QMessageBox>

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
#include <ctkErrorLogWidget.h>
#include <ctkFileDialog.h>
#include <ctkMessageBox.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif
#ifdef Slicer_USE_QtTesting
# include <ctkQtTestingUtility.h>
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
#include <vtkNew.h>

// vtksys includes
#include <vtksys/SystemTools.hxx>


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
  QString saveDirName = QFileDialog::getExistingDirectory(this->widget(), tr("Slicer Data Bundle Directory (Select Empty Directory)"), tempDir, QFileDialog::ShowDirsOnly);
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
void qSlicerAppMainWindowCore::onSDBSaveToMRBActionTriggered()
{
  //
  // open a file dialog to let the user choose where to save
  // make sure it was selected and add a .mrb to it if needed
  //
  QString tempDir = qSlicerCoreApplication::application()->temporaryPath();
  QString fileName = QFileDialog::getSaveFileName(this->widget(), tr("Save Data Bundle File"),
                                                    "", tr("Medical Reality Bundle (*.mrb)"));

  if (fileName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }

  if ( ! fileName.endsWith(".mrb") )
    {
    fileName += QString(".mrb");
    }

  //
  // make a temp directory to save the scene into - this will
  // be a uniquely named directory that contains a directory
  // named based on the user's selection.
  //
 
  // TODO: switch to QTemporaryDir in Qt5.
  // For now, create a named directory and use
  // kwsys calls to remove it
  QString packPath( QDir::tempPath() + 
                        QString("/__BundleSaveTemp") + 
                          QDateTime::currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz") );

  std::cerr << "packing to " << packPath.toStdString() << "\n";

  if (vtksys::SystemTools::FileIsDirectory(packPath.toLatin1()))
    {
    if ( !vtksys::SystemTools::RemoveADirectory(packPath.toLatin1()) )
      {
      QMessageBox::critical(this->widget(), tr("Save MRB"), tr("Could not remove temp directory"));
      return;
      }
    }

  if ( !vtksys::SystemTools::MakeDirectory(packPath.toLatin1()) )
    {
    QMessageBox::critical(this->widget(), tr("Save MRB"), tr("Could not make temp directory"));
    return;
    }

  // make a subdirectory with the name the user has chosen
  QFileInfo fileInfo(fileName);
  QString bundlePath = packPath + QString("/") + fileInfo.baseName();

  if ( !vtksys::SystemTools::MakeDirectory(bundlePath.toLatin1()) )
    {
    QMessageBox::critical(this->widget(), tr("Save MRB"), tr("Could not make temp directory"));
    return;
    }

  //
  // Now save the scene into the bundle directory and then make a zip (mrb) file
  // in the user's selected file location
  //
  vtkNew<vtkMRMLApplicationLogic> appLogic;
  appLogic->SetMRMLScene( qSlicerCoreApplication::application()->mrmlScene() );
  appLogic->SaveSceneToSlicerDataBundleDirectory(bundlePath.toLatin1(), NULL);
  std::cerr << "zipping to " << fileName.toStdString() << "\n";
  if ( !appLogic->Zip(fileName.toLatin1(), bundlePath.toLatin1()) )
    {
    QMessageBox::critical(this->widget(), tr("Save MRB"), tr("Could not compress bundle"));
    return;
    }

  //
  // Now clean up the temp directory
  //
  if ( !vtksys::SystemTools::RemoveADirectory(packPath.toLatin1()) )
    {
    QMessageBox::critical(this->widget(), tr("Save MRB"), tr("Could not remove temp directory"));
    return;
    }

  std::cerr << "saved " << packPath.toStdString() << "\n";
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onSDBSaveToDCMActionTriggered()
{
  // NOT IMPLEMENTED YET
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onFileCloseSceneActionTriggered()
{
  qSlicerCoreApplication::application()->mrmlScene()->Clear(false);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onEditRecordMacroActionTriggered()
{
#ifdef Slicer_USE_QtTesting
  QFileDialog fileDialog(this->widget(), tr("Macro File Name"),
                         QString("macro"), tr("XML Files (*.xml)"));
  fileDialog.setDefaultSuffix(QString("xml"));
  if(fileDialog.exec() == QDialog::Accepted)
    {
    QString filename = fileDialog.selectedFiles()[0];
    if (!filename.isEmpty())
      {
      qSlicerApplication::application()->testingUtility()->recordTests(filename);
      }
    }
#endif
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindowCore::onEditPlayMacroActionTriggered()
{
#ifdef Slicer_USE_QtTesting
  qSlicerApplication::application()->testingUtility()->openPlayerDialog();
//  QString filename = QFileDialog::getOpenFileName(this->widget(), "Test File Name",
//    QString(), "XML Files (*.xml)");
//  if (!filename.isEmpty())
//    {
//    d->TestUtility.playTests(filename);
//    }
#endif
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
