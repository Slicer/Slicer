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
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>

// CTK includes
#include <ctkBooleanMapper.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsGeneralPanel.h"
#include "ui_qSlicerSettingsGeneralPanel.h"

#include "vtkSlicerConfigure.h" // For Slicer_QM_OUTPUT_DIRS, Slicer_BUILD_I18N_SUPPORT

// --------------------------------------------------------------------------
// qSlicerSettingsGeneralPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsGeneralPanelPrivate: public Ui_qSlicerSettingsGeneralPanel
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
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanelPrivate::init()
{
  Q_Q(qSlicerSettingsGeneralPanel);

  this->setupUi(q);

#ifdef Slicer_BUILD_I18N_SUPPORT
  bool internationalizationEnabled =
      qSlicerApplication::application()->userSettings()->value("Internationalization/Enabled").toBool();

  this->LanguageLabel->setVisible(internationalizationEnabled);
  this->LanguageComboBox->setVisible(internationalizationEnabled);

  if (internationalizationEnabled)
    {
    /// Default values
    this->LanguageComboBox->setDefaultLanguage("en");
    /// set the directory where all the translations files are.
    this->LanguageComboBox->setDirectory(
        QString(Slicer_QM_OUTPUT_DIRS).split(";").at(0));
    }
#else
  this->LanguageLabel->setVisible(false);
  this->LanguageComboBox->setVisible(false);
#endif

  // Default values

  this->DefaultScenePathButton->setDirectory(qSlicerCoreApplication::application()->defaultScenePath());
  q->registerProperty("DefaultScenePath", this->DefaultScenePathButton,"directory",
                      SIGNAL(directoryChanged(QString)),
                      "Default scene path",
                     ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->DefaultScenePathButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(setDefaultScenePath(QString)));

  this->SlicerWikiURLLineEdit->setText("http://www.slicer.org/slicerWiki/index.php");

  q->registerProperty("no-splash", this->ShowSplashScreenCheckBox, "checked",
                      SIGNAL(toggled(bool)));
  ctkBooleanMapper* restartMapper = new ctkBooleanMapper(this->ConfirmRestartCheckBox, "checked", SIGNAL(toggled(bool)));
  restartMapper->setTrueValue(static_cast<int>(QMessageBox::InvalidRole));
  restartMapper->setFalseValue(static_cast<int>(QMessageBox::Ok));
  q->registerProperty("MainWindow/DontConfirmRestart",
                      restartMapper,"valueAsInt", SIGNAL(valueAsIntChanged(int)));
  ctkBooleanMapper* exitMapper = new ctkBooleanMapper(this->ConfirmExitCheckBox, "checked", SIGNAL(toggled(bool)));
  exitMapper->setTrueValue(static_cast<int>(QMessageBox::InvalidRole));
  exitMapper->setFalseValue(static_cast<int>(QMessageBox::Ok));
  q->registerProperty("MainWindow/DontConfirmExit",
                      exitMapper, "valueAsInt", SIGNAL(valueAsIntChanged(int)));
  q->registerProperty("SlicerWikiURL", this->SlicerWikiURLLineEdit, "text",
                      SIGNAL(textChanged(QString)));
  q->registerProperty("language", this->LanguageComboBox, "currentLanguage",
                      SIGNAL(currentLanguageNameChanged(const QString&)),
                      "Enable/Disable languages",
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("RecentlyLoadedFiles/NumberToKeep", this->NumOfRecentlyLoadedFiles, "value",
                      SIGNAL(valueChanged(int)),
                      "Max. number of 'Recently Loaded' menu items",
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
qSlicerSettingsGeneralPanel::~qSlicerSettingsGeneralPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsGeneralPanel::setDefaultScenePath(const QString& path)
{
  qSlicerCoreApplication::application()->setDefaultScenePath(path);
}
