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
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

// CTK includes
#include <ctkBooleanMapper.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerApplicationUpdateManager.h"
#include "qSlicerRelativePathMapper.h"
#include "qSlicerSettingsGeneralPanel.h"
#include "ui_qSlicerSettingsGeneralPanel.h"

#include "vtkSlicerConfigure.h" // For Slicer_QM_OUTPUT_DIRS, Slicer_BUILD_I18N_SUPPORT, Slicer_USE_PYTHONQT

#ifdef Slicer_USE_PYTHONQT
# include "PythonQt.h"
#endif

// --------------------------------------------------------------------------
// qSlicerSettingsGeneralPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsGeneralPanelPrivate : public Ui_qSlicerSettingsGeneralPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsGeneralPanel);

protected:
  qSlicerSettingsGeneralPanel* const q_ptr;

public:
  qSlicerSettingsGeneralPanelPrivate(qSlicerSettingsGeneralPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSettingsGeneralPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsGeneralPanelPrivate::qSlicerSettingsGeneralPanelPrivate(qSlicerSettingsGeneralPanel& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanelPrivate::init()
{
  Q_Q(qSlicerSettingsGeneralPanel);

  this->setupUi(q);

#ifdef Slicer_BUILD_I18N_SUPPORT
  bool internationalizationEnabled =
    qSlicerApplication::application()->userSettings()->value("Internationalization/Enabled", true).toBool();

  this->LanguageLabel->setVisible(internationalizationEnabled);
  this->LanguageComboBox->setVisible(internationalizationEnabled);

  if (internationalizationEnabled)
  {
    // Disable showing country flags because not all regions have a flag (e.g., Latin America)
    this->LanguageComboBox->setCountryFlagsVisible(false);
    this->LanguageComboBox->setDefaultLanguage("en");
    this->LanguageComboBox->setDirectories(qSlicerCoreApplication::translationFolders());
  }
#else
  this->LanguageLabel->setVisible(false);
  this->LanguageComboBox->setVisible(false);
#endif

  bool applicationUpdateEnabled = false;
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  applicationUpdateEnabled = qSlicerApplicationUpdateManager::isApplicationUpdateEnabled();
  if (applicationUpdateEnabled)
  {
    this->ApplicationUpdateServerURLLineEdit->setText("https://download.slicer.org");

    q->registerProperty("ApplicationUpdate/AutoUpdateCheck",
                        this->ApplicationAutoUpdateCheckCheckBox,
                        /*no tr*/ "checked",
                        SIGNAL(toggled(bool)));

    q->registerProperty("ApplicationUpdate/ServerUrl",
                        this->ApplicationUpdateServerURLLineEdit,
                        /*no tr*/ "text",
                        SIGNAL(textChanged(QString)),
                        qSlicerSettingsGeneralPanel::tr("Application update server URL"));

    qSlicerApplication* app = qSlicerApplication::application();
    if (app && app->applicationUpdateManager())
    {
      QObject::connect(app->applicationUpdateManager(),
                       SIGNAL(autoUpdateCheckChanged()),
                       q,
                       SLOT(updateAutoUpdateApplicationFromManager()));

      QObject::connect(this->ApplicationAutoUpdateCheckCheckBox,
                       SIGNAL(toggled(bool)),
                       app->applicationUpdateManager(),
                       SLOT(setAutoUpdateCheck(bool)));
    }
  }
#endif
  this->ApplicationAutoUpdateCheckLabel->setVisible(applicationUpdateEnabled);
  this->ApplicationAutoUpdateCheckCheckBox->setVisible(applicationUpdateEnabled);
  this->ApplicationUpdateServerURLLabel->setVisible(applicationUpdateEnabled);
  this->ApplicationUpdateServerURLLineEdit->setVisible(applicationUpdateEnabled);

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    context.evalScript(QString("slicerrcfilename = getSlicerRCFileName()\n"));
    QVariant slicerrcFileNameVar = context.getVariable("slicerrcfilename");
    this->SlicerRCFileValueLabel->setText(slicerrcFileNameVar.toString());
    this->SlicerRCFileOpenButton->setIcon(QIcon(":Icons/Go.png"));
    QObject::connect(this->SlicerRCFileOpenButton, SIGNAL(clicked()), q, SLOT(openSlicerRCFile()));
  }
  else
  {
    this->SlicerRCFileOpenButton->setVisible(false);
    this->SlicerRCFileValueLabel->setVisible(false);
  }
#else
  this->SlicerRCFileLabel->setVisible(false);
  this->SlicerRCFileValueLabel->setVisible(false);
#endif

  // Default values

  this->DefaultScenePathButton->setDirectory(qSlicerCoreApplication::application()->defaultScenePath());
  qSlicerRelativePathMapper* relativePathMapper = new qSlicerRelativePathMapper(this->DefaultScenePathButton,
                                                                                /*no tr*/ "directory",
                                                                                SIGNAL(directoryChanged(QString)));
  q->registerProperty("DefaultScenePath",
                      relativePathMapper,
                      "relativePath",
                      SIGNAL(relativePathChanged(QString)),
                      qSlicerSettingsGeneralPanel::tr("Default scene path"));
  QObject::connect(
    this->DefaultScenePathButton, SIGNAL(directoryChanged(QString)), q, SLOT(setDefaultScenePath(QString)));

  // Since currently there is only English language documentation on readthedocs, the default URL uses "en" language.
  this->DocumentationBaseURLLineEdit->setText("https://slicer.readthedocs.io/en/{version}");
  this->ModuleDocumentationURLLineEdit->setText("{documentationbaseurl}/user_guide/modules/{lowercasemodulename}.html");

  q->registerProperty("no-splash", this->ShowSplashScreenCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));

  ctkBooleanMapper* restartMapper =
    new ctkBooleanMapper(this->ConfirmRestartCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));
  restartMapper->setTrueValue(static_cast<int>(QMessageBox::InvalidRole));
  restartMapper->setFalseValue(static_cast<int>(QMessageBox::Ok));
  q->registerProperty("MainWindow/DontConfirmRestart", restartMapper, "valueAsInt", SIGNAL(valueAsIntChanged(int)));

  ctkBooleanMapper* exitMapper =
    new ctkBooleanMapper(this->ConfirmExitCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));
  exitMapper->setTrueValue(static_cast<int>(QMessageBox::InvalidRole));
  exitMapper->setFalseValue(static_cast<int>(QMessageBox::Ok));
  q->registerProperty("MainWindow/DontConfirmExit", exitMapper, "valueAsInt", SIGNAL(valueAsIntChanged(int)));

  ctkBooleanMapper* sceneCloseMapper =
    new ctkBooleanMapper(this->ConfirmSceneCloseCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));
  sceneCloseMapper->setTrueValue(static_cast<int>(QMessageBox::InvalidRole));
  sceneCloseMapper->setFalseValue(static_cast<int>(QMessageBox::AcceptRole));
  q->registerProperty(
    "MainWindow/DontConfirmSceneClose", sceneCloseMapper, "valueAsInt", SIGNAL(valueAsIntChanged(int)));

  q->registerProperty("DocumentationBaseURL",
                      this->DocumentationBaseURLLineEdit,
                      /*no tr*/ "text",
                      SIGNAL(textChanged(QString)),
                      qSlicerSettingsGeneralPanel::tr("Documentation location"),
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("ModuleDocumentationURL",
                      this->ModuleDocumentationURLLineEdit,
                      /*no tr*/ "text",
                      SIGNAL(textChanged(QString)),
                      qSlicerSettingsGeneralPanel::tr("Documentation location"),
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("language",
                      this->LanguageComboBox,
                      "currentLanguage",
                      SIGNAL(currentLanguageNameChanged(const QString&)),
                      qSlicerSettingsGeneralPanel::tr("Enable/Disable languages"),
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("RecentlyLoadedFiles/NumberToKeep",
                      this->NumOfRecentlyLoadedFiles,
                      /*no tr*/ "value",
                      SIGNAL(valueChanged(int)),
                      qSlicerSettingsGeneralPanel::tr("Max. number of 'Recent' menu items"),
                      ctkSettingsPanel::OptionRequireRestart);
}

// --------------------------------------------------------------------------
// qSlicerSettingsGeneralPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsGeneralPanel::qSlicerSettingsGeneralPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsGeneralPanelPrivate(*this))
{
  Q_D(qSlicerSettingsGeneralPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsGeneralPanel::~qSlicerSettingsGeneralPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanel::setDefaultScenePath(const QString& path)
{
  qSlicerCoreApplication::application()->setDefaultScenePath(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanel::openSlicerRCFile()
{
  Q_D(qSlicerSettingsGeneralPanel);
  QString slicerRcFileName = d->SlicerRCFileValueLabel->text();
  QFileInfo fileInfo(slicerRcFileName);
  if (!fileInfo.exists())
  {
    QFile outputFile(slicerRcFileName);
    if (outputFile.open(QFile::WriteOnly | QFile::Truncate))
    {
      // slicerrc file does not exist, create one with some default content
      QTextStream outputStream(&outputFile);
      outputStream << "# Python commands in this file are executed on Slicer startup\n"
                      "\n"
                      "# Examples:\n"
                      "#\n"
                      "# Load a scene file\n"
                      "# slicer.util.loadScene('c:/Users/SomeUser/Documents/SlicerScenes/SomeScene.mrb')\n"
                      "#\n"
                      "# Open a module (overrides default startup module in application settings / modules)\n"
                      "# slicer.util.mainWindow().moduleSelector().selectModule('SegmentEditor')\n"
                      "#\n";
      outputFile.close();
    }
  }

  QString editor = qSlicerApplication::application()->userSettings()->value("Python/Editor").toString();
  if (editor.isEmpty())
  {
    QDesktopServices::openUrl(QUrl("file:///" + slicerRcFileName, QUrl::TolerantMode));
  }
  else
  {
    QProcess process;
    // Use the startup environment to avoid Python environment issues with text editors implemented in Python
    process.setProcessEnvironment(qSlicerApplication::application()->startupEnvironment());
    process.setProgram(editor);
    process.setArguments(QStringList() << slicerRcFileName);
    process.startDetached();
  }
}

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanel::updateAutoUpdateApplicationFromManager()
{
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  Q_D(qSlicerSettingsGeneralPanel);
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app->applicationUpdateManager())
  {
    return;
  }
  QSignalBlocker blocker1(d->ApplicationAutoUpdateCheckCheckBox);
  d->ApplicationAutoUpdateCheckCheckBox->setChecked(app->applicationUpdateManager()->autoUpdateCheck());
#endif
}
