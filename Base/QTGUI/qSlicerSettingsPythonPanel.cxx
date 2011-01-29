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
#include <QMainWindow>
#include <QPointer>

// CTK includes
#include <ctkLogger.h>
#include <ctkPythonConsole.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsPythonPanel.h"
#include "ui_qSlicerSettingsPythonPanel.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerSettingsPythonPanel");

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

  // Lookup reference of 'PythonConsole' widget
  foreach(QWidget * widget, qApp->topLevelWidgets())
    {
    if(widget->objectName().compare(QLatin1String("pythonConsole")) == 0)
      {
      this->PythonConsole = QPointer<ctkPythonConsole>(qobject_cast<ctkPythonConsole*>(widget));
      break;
      }
    }
  Q_ASSERT(!this->PythonConsole.isNull());

  // Set default properties

  this->BackgroundColorPicker->setColor(this->PythonConsole->backgroundColor());

  this->PromptColorPicker->setColor(this->PythonConsole->promptColor());

  this->OutputTextColorPicker->setColor(this->PythonConsole->outputTextColor());

  this->ErrorTextColorPicker->setColor(this->PythonConsole->errorTextColor());

  this->StdinTextColorPicker->setColor(this->PythonConsole->stdinTextColor());

  this->CommandTextColorPicker->setColor(this->PythonConsole->commandTextColor());

  this->WelcomeTextColorPicker->setColor(this->PythonConsole->welcomeTextColor());

  this->ScrollBarPolicyComboBox->setCurrentIndex(this->PythonConsole->scrollBarPolicy());

  //
  // Connect panel widgets with associated slots
  //

  QObject::connect(this->BackgroundColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onBackgroundColorChanged(const QColor&)));

  QObject::connect(this->PromptColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onPromptColorChanged(const QColor&)));

  QObject::connect(this->OutputTextColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onOutputTextColorChanged(const QColor&)));

  QObject::connect(this->ErrorTextColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onErrorTextColorChanged(const QColor&)));

  QObject::connect(this->StdinTextColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onStdinTextColorChanged(const QColor&)));

  QObject::connect(this->CommandTextColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onCommandTextColorChanged(const QColor&)));

  QObject::connect(this->WelcomeTextColorPicker, SIGNAL(colorChanged(const QColor&)),
                   q, SLOT(onWelcomeTextColorChanged(const QColor&)));

  QObject::connect(this->ScrollBarPolicyComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onScrollBarPolicyChanged(int)));

  //
  // Register settings with their corresponding widgets
  //

  q->registerProperty("Python/BackgroundColor", this->BackgroundColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/PromptColor", this->PromptColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/OutputTextColor", this->OutputTextColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/ErrorTextColor", this->ErrorTextColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/StdinTextColor", this->StdinTextColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/CommandTextColor", this->CommandTextColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/WelcomeTextColor", this->WelcomeTextColorPicker, "color",
                      SIGNAL(colorChanged(const QColor&)));

  q->registerProperty("Python/ScrollBarPolicy", this->ScrollBarPolicyComboBox, "currentIndex",
                      SIGNAL(currentIndexChanged(int)));
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
qSlicerSettingsPythonPanel::~qSlicerSettingsPythonPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onBackgroundColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setBackgroundColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onPromptColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setPromptColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onOutputTextColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setOutputTextColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onErrorTextColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setErrorTextColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onStdinTextColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setStdinTextColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onCommandTextColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setCommandTextColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onWelcomeTextColorChanged(const QColor& newColor)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setWelcomeTextColor(newColor);
}

// --------------------------------------------------------------------------
void qSlicerSettingsPythonPanel::onScrollBarPolicyChanged(int index)
{
  Q_D(qSlicerSettingsPythonPanel);
  d->PythonConsole->setScrollBarPolicy(static_cast<Qt::ScrollBarPolicy>(index));
}
