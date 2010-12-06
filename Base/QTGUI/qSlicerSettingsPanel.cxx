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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
#include "ctkLogger.h"

// QtGUI includes
#include "qSlicerSettingsPanel.h"
#include "ui_qSlicerSettingsPanel.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerSettingsPanel");

//-----------------------------------------------------------------------------
class qSlicerSettingsPanelPrivate: public Ui_qSlicerSettingsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsPanel);
protected:
  qSlicerSettingsPanel* const q_ptr;

public:
  qSlicerSettingsPanelPrivate(qSlicerSettingsPanel& object);
  void init();

};

// --------------------------------------------------------------------------
qSlicerSettingsPanelPrivate::qSlicerSettingsPanelPrivate(qSlicerSettingsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsPanelPrivate::init()
{
  Q_Q(qSlicerSettingsPanel);

  this->setupUi(q);
  QObject::connect(this->FontButton, SIGNAL(currentFontChanged(const QFont&)),
                   q, SLOT(onFontChanged(const QFont&)));

  q->registerProperty("no-splash", this->ShowSplashScreenCheckBox, "checked",
                      SIGNAL(toggled(bool)));
  q->registerProperty("no-tooltip", this->ShowToolTipsCheckBox, "checked",
                      SIGNAL(toggled(bool)));
  q->registerProperty("font", this->FontButton, "currentFont",
                      SIGNAL(currentFontChanged(const QFont&)));
  q->registerProperty("MainWindow/RestoreGeometry", this->RestoreUICheckBox, "checked",
                      SIGNAL(toggled(bool)));
  q->registerProperty("MainWindow/ConfirmExit", this->ConfirmExitCheckBox, "checked",
                      SIGNAL(toggled(bool)));
}

// --------------------------------------------------------------------------
qSlicerSettingsPanel::qSlicerSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsPanelPrivate(*this))
{
  Q_D(qSlicerSettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsPanel::~qSlicerSettingsPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsPanel::onFontChanged(const QFont& font)
{
  qApp->setFont(font);
}
