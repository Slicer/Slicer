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

  This file was originally developed by Eric Larson.

==============================================================================*/

// Qt includes
#include <QDebug>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsViewsPanel.h"
#include "ui_qSlicerSettingsViewsPanel.h"

// CTK includes
#include <ctkVTKAbstractView.h>

// --------------------------------------------------------------------------
// qSlicerSettingsViewsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsViewsPanelPrivate: public Ui_qSlicerSettingsViewsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsViewsPanel);
protected:
  qSlicerSettingsViewsPanel* const q_ptr;

public:
  qSlicerSettingsViewsPanelPrivate(qSlicerSettingsViewsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSettingsViewsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsViewsPanelPrivate
::qSlicerSettingsViewsPanelPrivate(qSlicerSettingsViewsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanelPrivate::init()
{
  Q_Q(qSlicerSettingsViewsPanel);

  this->setupUi(q);

  // Default values
  this->FSAACheckBox->setChecked(false);

  // Register settings
  q->registerProperty("Views/FSAA",
                      this->FSAACheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Multisampling (FSAA)",
                      ctkSettingsPanel::OptionRequireRestart);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->FSAACheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onFSAAToggled(bool)));
}

// --------------------------------------------------------------------------
// qSlicerSettingsViewsPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsViewsPanel::qSlicerSettingsViewsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsViewsPanelPrivate(*this))
{
  Q_D(qSlicerSettingsViewsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsViewsPanel::~qSlicerSettingsViewsPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::onFSAAToggled(bool useFSAA)
{
  /// For "ctkVTKAbstractView"s (the main data views for the program),
  /// the multisampling properties should be set *before* creating any
  /// OpenGL contexts, otherwise the setting may have no effect. This
  /// means that we must read in the user's FSAA settings from QSettings
  /// before setting up the MainWindow UI, since setting up the UI
  /// creates all the view panels (and their associated OpenGL contexts).
  ///
  /// Before the MainWindow is set up, the ViewsPanel is added to the
  /// main settings dialog, . If the saved value is true (the default is
  /// false), this triggers this method to be called, allowing it to be
  /// set prior to creation of the OpenGL contexts.
  ctkVTKAbstractView::setUseMultiSamples(useFSAA);
}
