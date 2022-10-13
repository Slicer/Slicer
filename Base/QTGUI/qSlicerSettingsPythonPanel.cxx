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
#include <QDebug>
#include <QPointer>

// CTK includes
#include <ctkPythonConsole.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsPythonPanel.h"
#include "ui_qSlicerSettingsPythonPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsPythonPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsPythonPanelPrivate: public Ui_qSlicerSettingsPythonPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsPythonPanel);
protected:
  qSlicerSettingsPythonPanel* const q_ptr;

public:
  qSlicerSettingsPythonPanelPrivate(qSlicerSettingsPythonPanel& object);
  void init();

  QPointer<ctkPythonConsole> PythonConsole;

};

// --------------------------------------------------------------------------
// qSlicerSettingsPythonPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsPythonPanelPrivate::qSlicerSettingsPythonPanelPrivate(qSlicerSettingsPythonPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanelPrivate::init()
{
  Q_Q(qSlicerSettingsPythonPanel);

  this->setupUi(q);
  this->PythonConsole = qSlicerApplication::application()->pythonConsole();
  if (this->PythonConsole.isNull())
    {
    qWarning() << "qSlicerSettingsPythonPanelPrivate requires a python console";
    return;
    }

  // Set default properties

  this->ConsoleFontButton->setCurrentFont(this->PythonConsole->shellFont());

  this->ConsoleLogLevelComboBox->addItem(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::None));
  this->ConsoleLogLevelComboBox->addItem(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Error));
  this->ConsoleLogLevelComboBox->addItem(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Warning));
  this->ConsoleLogLevelComboBox->addItem(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Info));
  this->ConsoleLogLevelComboBox->addItem(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Debug));
  this->ConsoleLogLevelComboBox->setCurrentText(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Warning));

  //
  // Connect panel widgets with associated slots
  //

  QObject::connect(this->ConsoleFontButton, SIGNAL(currentFontChanged(QFont)),
                   q, SLOT(onFontChanged(QFont)));
  QObject::connect(this->ConsoleLogLevelComboBox, SIGNAL(currentIndexChanged(QString)),
    q, SLOT(onConsoleLogLevelChanged(QString)));

  //
  // Register settings with their corresponding widgets
  //

  q->registerProperty("Python/DockableWindow", this->DockableWindowCheckBox,
    "checked", SIGNAL(toggled(bool)),
    "Display Python interactor in a window that can be placed inside the main window.",
    ctkSettingsPanel::OptionRequireRestart);

  q->registerProperty("Python/Font", this->ConsoleFontButton, "currentFont",
                      SIGNAL(currentFontChanged(QFont)));

  q->registerProperty("Python/ConsoleLogLevel", q,
    "consoleLogLevel", SIGNAL(consoleLogLevelChanged(QString)));
  QObject::connect(this->ConsoleLogLevelComboBox, SIGNAL(currentIndexChanged(QString)),
    q, SIGNAL(consoleLogLevelChanged(QString)));

}

// --------------------------------------------------------------------------
// qSlicerSettingsPythonPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsPythonPanel::qSlicerSettingsPythonPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsPythonPanelPrivate(*this))
{
  Q_D(qSlicerSettingsPythonPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsPythonPanel::~qSlicerSettingsPythonPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onFontChanged(const QFont& font)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setShellFont(font);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onConsoleLogLevelChanged(const QString& levelStr)
{
  Q_D(const qSlicerSettingsPythonPanel);
  if (qSlicerApplication::application())
    {
    qSlicerApplication::application()->setPythonConsoleLogLevel(ctkErrorLogLevel::logLevelFromString(levelStr));
    }
}

// --------------------------------------------------------------------------
QString qSlicerSettingsPythonPanel::consoleLogLevel() const
{
  Q_D(const qSlicerSettingsPythonPanel);
  return d->ConsoleLogLevelComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::setConsoleLogLevel(const QString& text)
{
  Q_D(qSlicerSettingsPythonPanel);
  int selectedIndex = d->ConsoleLogLevelComboBox->findText(text);
  if (selectedIndex < 0)
    {
    // this text is for developers and so it is not translated
    selectedIndex = d->ConsoleLogLevelComboBox->findText(ctkErrorLogLevel::logLevelAsString(ctkErrorLogLevel::Warning));
    }
  // default to first item if conversion fails
  d->ConsoleLogLevelComboBox->setCurrentIndex(selectedIndex);
}
