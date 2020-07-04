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

  this->pythonFontButton->setCurrentFont(this->PythonConsole->shellFont());

  //
  // Connect panel widgets with associated slots
  //

  QObject::connect(this->pythonFontButton, SIGNAL(currentFontChanged(QFont)),
                   q, SLOT(onFontChanged(QFont)));

  //
  // Register settings with their corresponding widgets
  //
  q->registerProperty("Python/DockableWindow", this->DockableWindowCheckBox,
    "checked", SIGNAL(toggled(bool)),
    "Display Python interactor in a window that can be placed inside the main window.",
    ctkSettingsPanel::OptionRequireRestart);

  q->registerProperty("Python/Font", this->pythonFontButton, "currentFont",
                      SIGNAL(currentFontChanged(QFont)));
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
