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
#include <ctkComboBox.h>

// --------------------------------------------------------------------------
// qSlicerSettingsViewsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsViewsPanelPrivate : public Ui_qSlicerSettingsViewsPanel
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
qSlicerSettingsViewsPanelPrivate::qSlicerSettingsViewsPanelPrivate(qSlicerSettingsViewsPanel& object)
  : q_ptr(&object)
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
  QObject::connect(this->MSAAComboBox, SIGNAL(currentIndexChanged(QString)), q, SLOT(onMSAAChanged(QString)));
  QObject::connect(this->MSAAComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentMSAAChanged(QString)));
  q->registerProperty(
    "Views/MSAA", q, "currentMSAA", SIGNAL(currentMSAAChanged(QString)), qSlicerSettingsViewsPanel::tr("Multisampling (MSAA)"), ctkSettingsPanel::OptionRequireRestart);

  this->SliceOrientationMarkerTypeComboBox->addItem(qSlicerSettingsViewsPanel::tr("none"), QString(/*no tr*/ "none"));
  this->SliceOrientationMarkerTypeComboBox->addItem(qSlicerSettingsViewsPanel::tr("cube"), QString(/*no tr*/ "cube"));
  this->SliceOrientationMarkerTypeComboBox->addItem(qSlicerSettingsViewsPanel::tr("human"), QString(/*no tr*/ "human"));
  this->SliceOrientationMarkerTypeComboBox->addItem(qSlicerSettingsViewsPanel::tr("axes"), QString(/*no tr*/ "axes"));

  QObject::connect(this->SliceOrientationMarkerTypeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentSliceOrientationMarkerTypeChanged(QString)));
  q->registerProperty("DefaultSliceView/OrientationMarkerType",
                      q,
                      "sliceOrientationMarkerType",
                      SIGNAL(currentSliceOrientationMarkerTypeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("Slice view orientation marker type"),
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->SliceOrientationMarkerSizeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentSliceOrientationMarkerSizeChanged(QString)));
  q->registerProperty("DefaultSliceView/OrientationMarkerSize",
                      q,
                      "sliceOrientationMarkerSize",
                      SIGNAL(currentSliceOrientationMarkerSizeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("Slice view orientation marker size"),
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->SliceRulerTypeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentSliceRulerTypeChanged(QString)));
  q->registerProperty("DefaultSliceView/RulerType",
                      q,
                      "sliceRulerType",
                      SIGNAL(currentSliceRulerTypeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("Slice view ruler type"),
                      ctkSettingsPanel::OptionRequireRestart);

  this->SliceViewOrientationComboBox->addItem(qSlicerSettingsViewsPanel::tr("patient right is screen left (default)"), QString("PatientRightIsScreenLeft"));
  this->SliceViewOrientationComboBox->addItem(qSlicerSettingsViewsPanel::tr("patient right is screen right"), QString("PatientRightIsScreenRight"));
  q->registerProperty("DefaultSliceView/Orientation",
                      this->SliceViewOrientationComboBox,
                      "currentUserDataAsString",
                      SIGNAL(currentIndexChanged(int)),
                      qSlicerSettingsViewsPanel::tr("Default slice view orientation"),
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->SliceViewOrientationComboBox, SIGNAL(activated(int)), q, SLOT(sliceViewOrientationChangedByUser()));

  q->registerProperty("DefaultSliceView/SliceEdgeVisibility3D",
                      this->SliceEdgeVisibility3DCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("Slice edge visibility in 3D views"),
                      ctkSettingsPanel::OptionRequireRestart);

  q->registerProperty("Default3DView/BoxVisibility",
                      this->ThreeDBoxVisibilityCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("3D view cube visibility"));
  q->registerProperty("Default3DView/AxisLabelsVisibility",
                      this->ThreeDAxisLabelsVisibilityCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("3D view axis label visibility"));
  QObject::connect(this->ThreeDOrientationMarkerTypeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentThreeDOrientationMarkerTypeChanged(QString)));
  q->registerProperty("Default3DView/OrientationMarkerType",
                      q,
                      "threeDOrientationMarkerType",
                      SIGNAL(currentThreeDOrientationMarkerTypeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("3D view orientation marker type"),
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->ThreeDOrientationMarkerSizeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentThreeDOrientationMarkerSizeChanged(QString)));
  q->registerProperty("Default3DView/OrientationMarkerSize",
                      q,
                      "threeDOrientationMarkerSize",
                      SIGNAL(currentThreeDOrientationMarkerSizeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("3D view orientation marker size"),
                      ctkSettingsPanel::OptionRequireRestart);
  QObject::connect(this->ThreeDRulerTypeComboBox, SIGNAL(currentIndexChanged(QString)), q, SIGNAL(currentThreeDRulerTypeChanged(QString)));
  q->registerProperty("Default3DView/RulerType",
                      q,
                      "threeDRulerType",
                      SIGNAL(currentThreeDRulerTypeChanged(QString)),
                      qSlicerSettingsViewsPanel::tr("3D view ruler type"),
                      ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("Default3DView/UseDepthPeeling",
                      this->ThreeDUseDepthPeelingCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("3D depth peeling"));
  q->registerProperty("Default3DView/UseOrthographicProjection",
                      this->ThreeDUseOrthographicProjectionCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("Orthographic projection"));
  q->registerProperty("Default3DView/ShadowsVisibility",
                      this->ThreeDShadowsVisibilityCheckBox,
                      /*no tr*/ "checked",
                      SIGNAL(toggled(bool)),
                      qSlicerSettingsViewsPanel::tr("Shadows visibility"));
  q->registerProperty("Default3DView/AmbientShadowsSizeScale",
                      this->ThreeDAmbientShadowsSizeScaleSlider,
                      /*no tr*/ "value",
                      SIGNAL(valueChanged(double)),
                      qSlicerSettingsViewsPanel::tr("Ambient shadows size scale"));
  q->registerProperty("Default3DView/AmbientShadowsVolumeOpacityThreshold",
                      this->ThreeDAmbientShadowsVolumeOpacityThresholdSlider,
                      /*no tr*/ "value",
                      SIGNAL(valueChanged(double)),
                      qSlicerSettingsViewsPanel::tr("Ambient shadows volume opacity threshold"));
  q->registerProperty("Default3DView/AmbientShadowsIntensityScale",
                      this->ThreeDAmbientShadowsIntensityScaleSlider,
                      /*no tr*/ "value",
                      SIGNAL(valueChanged(double)),
                      qSlicerSettingsViewsPanel::tr("Ambient shadows intensity scale"));
  q->registerProperty("Default3DView/AmbientShadowsIntensityShift",
                      this->ThreeDAmbientShadowsIntensityShiftSlider,
                      /*no tr*/ "value",
                      SIGNAL(valueChanged(double)),
                      qSlicerSettingsViewsPanel::tr("Ambient shadows intensity shift"));
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

// --------------------------------------------------------------------------
void qSlicerSettingsViewsPanel::sliceViewOrientationChangedByUser()
{
  Q_D(qSlicerSettingsViewsPanel);
  if (d->SliceViewOrientationComboBox->currentUserDataAsString() == "PatientRightIsScreenRight")
  {
    if (d->SliceOrientationMarkerTypeComboBox->currentData() == /*no tr*/ "none")
    {
      // Non-default orientation is chosen and no orientation marker is displayed.
      // To ensure that there is no accidental mixup of orientations, show the orientation marker.
      int index = d->SliceOrientationMarkerTypeComboBox->findData(/*no tr*/ "axes");
      d->SliceOrientationMarkerTypeComboBox->setCurrentIndex(index);
    }
  }
}
