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

  QObject::connect(this->SliceOrientationMarkerTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentSliceOrientationMarkerTypeChanged(QString)));
  q->registerProperty("DefaultSliceView/OrientationMarkerType", q,
                      "sliceOrientationMarkerType", SIGNAL(currentSliceOrientationMarkerTypeChanged(QString)),
                      "Slice view orientation marker type",
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->SliceOrientationMarkerSizeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentSliceOrientationMarkerSizeChanged(QString)));
  q->registerProperty("DefaultSliceView/OrientationMarkerSize", q,
                      "sliceOrientationMarkerSize", SIGNAL(currentSliceOrientationMarkerSizeChanged(QString)),
                      "Slice view orientation marker size",
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->SliceRulerTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentSliceRulerTypeChanged(QString)));
  q->registerProperty("DefaultSliceView/RulerType", q,
                      "sliceRulerType", SIGNAL(currentSliceRulerTypeChanged(QString)),
                      "Slice view ruler type",
                      ctkSettingsPanel::OptionRequireRestart);

  q->registerProperty("Default3DView/BoxVisibility", this->ThreeDBoxVisibilityCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "3D view cube visibility");
  q->registerProperty("Default3DView/AxisLabelsVisibility", this->ThreeDAxisLabelsVisibilityCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "3D view axis label visibility");
  QObject::connect(this->ThreeDOrientationMarkerTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentThreeDOrientationMarkerTypeChanged(QString)));
  q->registerProperty("Default3DView/OrientationMarkerType", q,
                      "threeDOrientationMarkerType", SIGNAL(currentThreeDOrientationMarkerTypeChanged(QString)),
                      "3D view orientation marker type",
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->ThreeDOrientationMarkerSizeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentThreeDOrientationMarkerSizeChanged(QString)));
  q->registerProperty("Default3DView/OrientationMarkerSize", q,
                      "threeDOrientationMarkerSize", SIGNAL(currentThreeDOrientationMarkerSizeChanged(QString)),
                      "3D view orientation marker size",
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->ThreeDRulerTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SIGNAL(currentThreeDRulerTypeChanged(QString)));
  q->registerProperty("Default3DView/RulerType", q,
                      "threeDRulerType", SIGNAL(currentThreeDRulerTypeChanged(QString)),
                      "3D view ruler type",
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("Default3DView/UseDepthPeeling", this->ThreeDUseDepthPeelingCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "3D depth peeling");
  q->registerProperty("Default3DView/UseOrthographicProjection", this->ThreeDUseOrthographicProjectionCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Orthographic projection");

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
qSlicerSettingsViewsPanel::~qSlicerSettingsViewsPanel() = default;

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

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::sliceOrientationMarkerType() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->SliceOrientationMarkerTypeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setSliceOrientationMarkerType(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->SliceOrientationMarkerTypeComboBox->setCurrentIndex(qMax(d->SliceOrientationMarkerTypeComboBox->findText(text), 0));
}

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::sliceOrientationMarkerSize() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->SliceOrientationMarkerSizeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setSliceOrientationMarkerSize(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->SliceOrientationMarkerSizeComboBox->setCurrentIndex(qMax(d->SliceOrientationMarkerSizeComboBox->findText(text), 0));
}

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::sliceRulerType() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->SliceRulerTypeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setSliceRulerType(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->SliceRulerTypeComboBox->setCurrentIndex(qMax(d->SliceRulerTypeComboBox->findText(text), 0));
}


// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::threeDOrientationMarkerType() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->ThreeDOrientationMarkerTypeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setThreeDOrientationMarkerType(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->ThreeDOrientationMarkerTypeComboBox->setCurrentIndex(qMax(d->ThreeDOrientationMarkerTypeComboBox->findText(text), 0));
}

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::threeDOrientationMarkerSize() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->ThreeDOrientationMarkerSizeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setThreeDOrientationMarkerSize(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->ThreeDOrientationMarkerSizeComboBox->setCurrentIndex(qMax(d->ThreeDOrientationMarkerSizeComboBox->findText(text), 0));
}

// --------------------------------------------------------------------------
QString qSlicerSettingsViewsPanel::threeDRulerType() const
{
  Q_D(const qSlicerSettingsViewsPanel);
  return d->ThreeDRulerTypeComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::setThreeDRulerType(const QString& text)
{
  Q_D(qSlicerSettingsViewsPanel);
  // default to first item if conversion fails
  d->ThreeDRulerTypeComboBox->setCurrentIndex(qMax(d->ThreeDRulerTypeComboBox->findText(text), 0));
}
