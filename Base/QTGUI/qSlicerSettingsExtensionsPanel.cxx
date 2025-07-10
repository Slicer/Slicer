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
#include <QMainWindow>
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsManagerModel.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerRelativePathMapper.h"
#include "qSlicerSettingsExtensionsPanel.h"
#include "ui_qSlicerSettingsExtensionsPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsExtensionsPanelPrivate : public Ui_qSlicerSettingsExtensionsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsExtensionsPanel);

protected:
  qSlicerSettingsExtensionsPanel* const q_ptr;

public:
  qSlicerSettingsExtensionsPanelPrivate(qSlicerSettingsExtensionsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanelPrivate::qSlicerSettingsExtensionsPanelPrivate(qSlicerSettingsExtensionsPanel& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanelPrivate::init()
{
  Q_Q(qSlicerSettingsExtensionsPanel);

  this->setupUi(q);

  qSlicerApplication* app = qSlicerApplication::application();

  // Default values
  this->ExtensionsManagerEnabledCheckBox->setChecked(true);
  if (app->extensionsManagerModel()->serverAPI() == qSlicerExtensionsManagerModel::Girder_v1)
  {
    this->ExtensionsServerUrlLineEdit->setText("https://slicer-packages.kitware.com");
    this->ExtensionsFrontendServerUrlLineEdit->setText("https://extensions.slicer.org");
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI"
               << app->extensionsManagerModel()->serverAPI();
  }
  this->ExtensionsInstallPathButton->setDirectory(app->defaultExtensionsInstallPath());
#ifdef Q_OS_MAC
  this->ExtensionsInstallPathButton->setDisabled(true);
#endif
  this->AutoUpdateCheckCheckBox->setChecked(false);
  this->AutoUpdateInstallCheckBox->setChecked(false);
  this->AutoInstallDependenciesCheckBox->setChecked(true);

  bool extensionsManagerEnabled = app && app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool();
  this->OpenExtensionsManagerPushButton->setVisible(extensionsManagerEnabled);

  // Register settings
  q->registerProperty("Extensions/ManagerEnabled",
                      this->ExtensionsManagerEnabledCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsExtensionsPanel::tr("Enable/Disable extensions manager"),
                      ctkSettingsPanel::OptionRequireRestart,
                      app->revisionUserSettings());
  q->registerProperty("Extensions/ServerUrl",
                      this->ExtensionsServerUrlLineEdit,
                      /*no tr*/ "text",
                      SIGNAL(textChanged(QString)),
                      QString(),
                      ctkSettingsPanel::OptionNone,
                      app->revisionUserSettings());
  q->registerProperty("Extensions/FrontendServerUrl",
                      this->ExtensionsFrontendServerUrlLineEdit,
                      /*no tr*/ "text",
                      SIGNAL(textChanged(QString)),
                      QString(),
                      ctkSettingsPanel::OptionNone,
                      app->revisionUserSettings());

  q->registerProperty("Extensions/AutoUpdateCheck",
                      this->AutoUpdateCheckCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsExtensionsPanel::tr("Automatic update check"));
  q->registerProperty("Extensions/AutoUpdateInstall",
                      this->AutoUpdateInstallCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsExtensionsPanel::tr("Automatic update install"));
  q->registerProperty("Extensions/AutoInstallDependencies",
                      this->AutoInstallDependenciesCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsExtensionsPanel::tr("Automatic install of dependencies"));

  qSlicerRelativePathMapper* relativePathMapper =
    new qSlicerRelativePathMapper(this->ExtensionsInstallPathButton, "directory", SIGNAL(directoryChanged(QString)));
  q->registerProperty("Extensions/InstallPath",
                      relativePathMapper,
                      "relativePath",
                      SIGNAL(relativePathChanged(QString)),
                      QString(),
                      ctkSettingsPanel::OptionNone,
                      app->revisionUserSettings());

  // Actions to propagate to the application when settings are changed
  QObject::connect(
    this->ExtensionsManagerEnabledCheckBox, SIGNAL(toggled(bool)), q, SLOT(onExtensionsManagerEnabled(bool)));
  QObject::connect(
    this->ExtensionsServerUrlLineEdit, SIGNAL(textChanged(QString)), q, SIGNAL(extensionsServerUrlChanged(QString)));
  QObject::connect(this->ExtensionsFrontendServerUrlLineEdit,
                   SIGNAL(textChanged(QString)),
                   q,
                   SIGNAL(extensionsFrontendServerUrlChanged(QString)));
  QObject::connect(
    this->ExtensionsInstallPathButton, SIGNAL(directoryChanged(QString)), q, SLOT(onExtensionsPathChanged(QString)));
  QObject::connect(this->OpenExtensionsManagerPushButton, SIGNAL(clicked()), app, SLOT(openExtensionsManagerDialog()));
  QObject::connect(
    this->OpenExtensionsCatalogWebsitePushButton, SIGNAL(clicked()), app, SLOT(openExtensionsCatalogWebsite()));
  QObject::connect(
    app->extensionsManagerModel(), SIGNAL(autoUpdateSettingsChanged()), q, SLOT(updateAutoUpdateWidgetsFromModel()));
  QObject::connect(this->AutoUpdateCheckCheckBox,
                   SIGNAL(toggled(bool)),
                   app->extensionsManagerModel(),
                   SLOT(setAutoUpdateCheck(bool)));
  QObject::connect(this->AutoUpdateInstallCheckBox,
                   SIGNAL(toggled(bool)),
                   app->extensionsManagerModel(),
                   SLOT(setAutoUpdateInstall(bool)));
  QObject::connect(this->AutoInstallDependenciesCheckBox,
                   SIGNAL(toggled(bool)),
                   app->extensionsManagerModel(),
                   SLOT(setAutoInstallDependencies(bool)));
}

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanel::qSlicerSettingsExtensionsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsExtensionsPanelPrivate(*this))
{
  Q_D(qSlicerSettingsExtensionsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanel::~qSlicerSettingsExtensionsPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExtensionsManagerEnabled(bool value)
{
  Q_UNUSED(value);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExtensionsPathChanged(const QString& path)
{
  qSlicerCoreApplication::application()->setExtensionsInstallPath(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::updateAutoUpdateWidgetsFromModel()
{
  Q_D(qSlicerSettingsExtensionsPanel);
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app->extensionsManagerModel())
  {
    return;
  }
  QSignalBlocker blocker1(d->AutoUpdateCheckCheckBox);
  QSignalBlocker blocker2(d->AutoUpdateInstallCheckBox);
  QSignalBlocker blocker3(d->AutoInstallDependenciesCheckBox);
  d->AutoUpdateCheckCheckBox->setChecked(app->extensionsManagerModel()->autoUpdateCheck());
  d->AutoUpdateInstallCheckBox->setChecked(app->extensionsManagerModel()->autoUpdateInstall());
  d->AutoInstallDependenciesCheckBox->setChecked(app->extensionsManagerModel()->autoInstallDependencies());
}
