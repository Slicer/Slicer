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

  // MSAA Setting
  // "Off" is the default, and should be the zeroth index to ensure that
  // poorly-formatted (or old) entries in the .ini default to "Off"
  // (toInt() returns zero when formatting is bad).
  this->MSAAComboBox->addItem("Off", 0);
  this->MSAAComboBox->addItem("Auto", -1);
  this->MSAAComboBox->addItem("2x", 2);
  this->MSAAComboBox->addItem("4x", 4);
  this->MSAAComboBox->addItem("8x", 8);
  this->MSAAComboBox->addItem("16x", 16);
  this->MSAAComboBox->setCurrentIndex(this->MSAAComboBox->findText("Off"));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->MSAAComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SLOT(onMSAAChanged(QString)));
  QObject::connect(this->MSAAComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentMSAAChanged(QString)));
  q->registerProperty("Views/MSAA", q,
                      "currentMSAA", SIGNAL(currentMSAAChanged(QString)),
                      "Multisampling (MSAA)",
                      ctkSettingsPanel::OptionRequireRestart);
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
void qSlicerSettingsViewsPanel::onMSAAChanged(const QString& text)
{
  /// For "ctkVTKAbstractView"s (the main data views for the program),
  /// the multisampling properties should be set *before* creating any
  /// OpenGL contexts, otherwise the setting may have no effect. This
  /// means that we must read in the user's MSAA settings from QSettings
  /// before setting up the MainWindow UI, since setting up the UI
  /// creates all the view panels (and their associated OpenGL contexts).
  ///
  /// Before the MainWindow is set up, the ViewsPanel is added to the
  /// main settings dialog, . If the saved value is true (the default is
  /// false), this triggers this method to be called, allowing it to be
  /// set prior to creation of the OpenGL contexts.

  Q_D(const qSlicerSettingsViewsPanel);
  const int index = d->MSAAComboBox->findText(text);
  const int nSamples = d->MSAAComboBox->itemData(index).toInt();
  ctkVTKAbstractView::setMultiSamples(nSamples);
}

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::currentMSAA() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->MSAAComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setCurrentMSAA(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to "Off" (0) if conversion fails
  d->MSAAComboBox->setCurrentIndex(qMax(d->MSAAComboBox->findText(text), 0));
}
