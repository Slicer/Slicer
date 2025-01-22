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

==============================================================================*/

// Settings Widgets includes
#include "qSlicerCornerTextSettingsWidget.h"
#include "ctkPimpl.h"
#include "ui_qSlicerCornerTextSettingsWidget.h"
#include "vtkMRMLCornerTextLogic.h"

// MRML includes
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSliceNode.h>

//-----------------------------------------------------------------------------
class qSlicerCornerTextSettingsWidgetPrivate
  : public Ui_qSlicerCornerTextSettingsWidget
{
  Q_DECLARE_PUBLIC(qSlicerCornerTextSettingsWidget);
protected:
  qSlicerCornerTextSettingsWidget* const q_ptr;

public:
  qSlicerCornerTextSettingsWidgetPrivate(
    qSlicerCornerTextSettingsWidget& object);
  virtual void setupUi(qSlicerCornerTextSettingsWidget*);
  void init();
  bool ToggleLocation(vtkMRMLCornerTextLogic::TextLocation, bool enabled);

  vtkMRMLLayoutLogic* LayoutLogic;
  vtkMRMLCornerTextLogic* CornerTextLogic;
};

// --------------------------------------------------------------------------
qSlicerCornerTextSettingsWidgetPrivate
::qSlicerCornerTextSettingsWidgetPrivate(
  qSlicerCornerTextSettingsWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidgetPrivate
::setupUi(qSlicerCornerTextSettingsWidget* widget)
{
  this->Ui_qSlicerCornerTextSettingsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidgetPrivate
::init()
{
  Q_Q(qSlicerCornerTextSettingsWidget);

  QObject::connect(this->sliceViewAnnotationsCheckBox, SIGNAL(toggled(bool)), q, SLOT(enableSliceViewAnnotations(bool)));

  // Corner Text Annotation ctk collapsible button pane.
  this->sliceViewAnnotationsCheckBox->setChecked(true);
  this->cornerTextParametersCollapsibleButton->setEnabled(true);

  QObject::connect(this->bottomLeftCheckBox, SIGNAL(toggled(bool)), q, SLOT(setBottomLeftCornerActive(bool)));
  QObject::connect(this->topLeftCheckBox, SIGNAL(toggled(bool)), q, SLOT(setTopLeftCornerActive(bool)));
  QObject::connect(this->topRightCheckBox, SIGNAL(toggled(bool)), q, SLOT(setTopRightCornerActive(bool)));

  // Amount subpanel
  this->annotationsAmountGroupBox->setEnabled(true);
  QObject::connect(this->level1RadioButton, &QRadioButton::toggled, [=](bool checked) {
      if (checked) q->setAnnotationDisplayLevel(1);
  });

  QObject::connect(this->level2RadioButton, &QRadioButton::toggled, [=](bool checked) {
      if (checked) q->setAnnotationDisplayLevel(2);
  });

  QObject::connect(this->level3RadioButton, &QRadioButton::toggled, [=](bool checked) {
      if (checked) q->setAnnotationDisplayLevel(3);
  });

  // Font Properties subpanel
  this->fontPropertiesGroupBox->setEnabled(true);
  QObject::connect(this->timesFontRadioButton, &QRadioButton::toggled, [=](bool checked) {
      if (checked) q->setFontFamily("Times");
  });

  QObject::connect(this->arialFontRadioButton, &QRadioButton::toggled, [=](bool checked) {
      if (checked) q->setFontFamily("Arial");
  });
  QObject::connect(this->fontSizeSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setFontSize(int)));

  // DICOM Annotations subpanel
  this->dicomAnnotationsCollapsibleGroupBox->setEnabled(false);
  QObject::connect(this->backgroundPersistenceCheckBox, SIGNAL(toggled(bool)), q, SLOT(setDICOMAnnotationsPersistence(bool)));

  q->updateWidgetFromCornerTextLogic();
}

//-----------------------------------------------------------------------------
// qSlicerCornerTextSettingsWidget methods

//-----------------------------------------------------------------------------
qSlicerCornerTextSettingsWidget
::qSlicerCornerTextSettingsWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCornerTextSettingsWidgetPrivate(*this) )
{
  Q_D(qSlicerCornerTextSettingsWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCornerTextSettingsWidget
::~qSlicerCornerTextSettingsWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::enableSliceViewAnnotations(bool enable)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  // For graying out the section in the GUI
  d->cornerTextParametersCollapsibleButton->setEnabled(enable);
  // For informing the logic that it should not generate annotations
  d->CornerTextLogic->SetSliceViewAnnotationsEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setTopLeftCornerActive(bool enable)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetTopLeftEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setTopRightCornerActive(bool enable)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetTopRightEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setBottomLeftCornerActive(bool enable)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetBottomLeftEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setAnnotationDisplayLevel(int level)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetDisplayStrictness(level);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setFontFamily(const QString& fontFamily)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetFontFamily(fontFamily.toStdString());
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setFontSize(int fontSize)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  d->CornerTextLogic->SetFontSize(fontSize);
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setDICOMAnnotationsPersistence(bool enable)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->CornerTextLogic)
  {
    return;
  }
  // TODO: Not supported yet.
  // d->CornerTextLogic->SetDICOMAnnotationsPersistence(enable);
}

// ----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::onLayoutLogicModifiedEvent()
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (!d->LayoutLogic)
  {
    return;
  }

  setBottomLeftCornerActive(d->bottomLeftCheckBox->isChecked());
  setTopLeftCornerActive(d->topLeftCheckBox->isChecked());
  setTopRightCornerActive(d->topRightCheckBox->isChecked());
}

CTK_GET_CPP(qSlicerCornerTextSettingsWidget, vtkMRMLLayoutLogic*, layoutLogic, LayoutLogic)

void qSlicerCornerTextSettingsWidget::setLayoutLogic(vtkMRMLLayoutLogic* newLayoutLogic)
{
  Q_D(qSlicerCornerTextSettingsWidget);
  if (d->LayoutLogic == newLayoutLogic)
  {
    return;
  }

  qvtkReconnect(d->LayoutLogic, newLayoutLogic, vtkCommand::ModifiedEvent,
                   this, SLOT(onLayoutLogicModifiedEvent()));

  d->LayoutLogic = newLayoutLogic;

  if (d->LayoutLogic && d->LayoutLogic->GetMRMLScene())
  {
    this->setMRMLScene(d->LayoutLogic->GetMRMLScene());
  }

  this->onLayoutLogicModifiedEvent();
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::setAndObserveCornerTextLogic(vtkMRMLCornerTextLogic* cornerTextLogic)
{
  Q_D(qSlicerCornerTextSettingsWidget);

  qvtkReconnect(d->CornerTextLogic, cornerTextLogic, vtkCommand::ModifiedEvent,
                   this, SLOT(onLayoutLogicModifiedEvent()));

  d->CornerTextLogic = cornerTextLogic;
  this->updateWidgetFromCornerTextLogic();
}

CTK_GET_CPP(qSlicerCornerTextSettingsWidget, vtkMRMLCornerTextLogic*, cornerTextLogic, CornerTextLogic)


//-----------------------------------------------------------------------------
void qSlicerCornerTextSettingsWidget::updateWidgetFromCornerTextLogic()
{
  Q_D(qSlicerCornerTextSettingsWidget);

  if (!d->CornerTextLogic)
  {
    return;
  }

  // From DataProbe settings

  d->sliceViewAnnotationsCheckBox->setChecked(d->CornerTextLogic->GetSliceViewAnnotationsEnabled());
  d->cornerTextParametersCollapsibleButton->setEnabled(d->CornerTextLogic->GetSliceViewAnnotationsEnabled());

  (d->CornerTextLogic->GetFontFamily() == "Arial") ? d->arialFontRadioButton->toggle() : d->timesFontRadioButton->toggle();

  d->fontSizeSpinBox->setValue(d->CornerTextLogic->GetFontSize());

  switch (d->CornerTextLogic->GetDisplayStrictness())
  {
    case 1:
    {
      d->level1RadioButton->toggle();
      break;
    }
    case 2:
    {
      d->level2RadioButton->toggle();
      break;
    }
    case 3:
    {
      d->level3RadioButton->toggle();
      break;
    }
    default:
    {
      break;
    }
  }

  d->bottomLeftCheckBox->setChecked(d->CornerTextLogic->GetBottomLeftEnabled());
  d->topLeftCheckBox->setChecked(d->CornerTextLogic->GetTopLeftEnabled());
  d->topRightCheckBox->setChecked(d->CornerTextLogic->GetTopRightEnabled());
}
