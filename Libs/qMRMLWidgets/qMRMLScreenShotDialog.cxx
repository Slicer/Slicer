/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// QT includes
#include <QButtonGroup>
#include <QMetaProperty>
#include <QWeakPointer>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkVTKWidgetsUtils.h>

// qMRML includes
#include "qMRMLLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLScreenShotDialog.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLUtils.h"

#include "ui_qMRMLScreenShotDialog.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qMRMLScreenShotDialogPrivate : public Ui_qMRMLScreenShotDialog
{
  Q_DECLARE_PUBLIC(qMRMLScreenShotDialog);
protected:
  qMRMLScreenShotDialog* const q_ptr;
public:
  qMRMLScreenShotDialogPrivate(qMRMLScreenShotDialog& object);

  void setupUi(QDialog* dialog);
  void setCheckedRadioButton(int type);
  void setWidgetEnabled(bool state);

  QWeakPointer<qMRMLLayoutManager>   layoutManager;
  vtkSmartPointer<vtkImageData>      imageData;
  /// The ID of the associated snapshot node.
  /// This is NULL if the dialog has no associated snapshot node (== new snapshot mode).
  QVariant                           data;
  QButtonGroup*                      widgetTypeGroup;
};

//-----------------------------------------------------------------------------
qMRMLScreenShotDialogPrivate::qMRMLScreenShotDialogPrivate(qMRMLScreenShotDialog &object)
  : q_ptr(&object)
{
  qRegisterMetaType<qMRMLScreenShotDialog::WidgetType>(
      "qMRMLScreenShotDialog::WidgetType");
  this->widgetTypeGroup = 0;
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialogPrivate::setupUi(QDialog* dialog)
{
  this->Ui_qMRMLScreenShotDialog::setupUi(dialog);
  this->widgetTypeGroup = new QButtonGroup(dialog);
  this->widgetTypeGroup->setExclusive(true);
  this->widgetTypeGroup->addButton(this->fullLayoutRadio, qMRMLScreenShotDialog::FullLayout);
  this->widgetTypeGroup->addButton(this->threeDViewRadio, qMRMLScreenShotDialog::ThreeD);
  this->widgetTypeGroup->addButton(this->redSliceViewRadio, qMRMLScreenShotDialog::Red);
  this->widgetTypeGroup->addButton(this->yellowSliceViewRadio, qMRMLScreenShotDialog::Yellow);
  this->widgetTypeGroup->addButton(this->greenSliceViewRadio, qMRMLScreenShotDialog::Green);
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialogPrivate::setCheckedRadioButton(int type)
{
  QRadioButton* widgetButton =
    qobject_cast<QRadioButton*>(this->widgetTypeGroup->button(type));
  widgetButton->setChecked(true);
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialogPrivate::setWidgetEnabled(bool state)
{
  this->threeDViewRadio->setEnabled(state);
  this->redSliceViewRadio->setEnabled(state);
  this->yellowSliceViewRadio->setEnabled(state);
  this->greenSliceViewRadio->setEnabled(state);
  this->fullLayoutRadio->setEnabled(state);
  this->scaleFactorSpinBox->setEnabled(state);
}

//-----------------------------------------------------------------------------
// qMRMLScreenShotDialog methods

//-----------------------------------------------------------------------------
qMRMLScreenShotDialog::qMRMLScreenShotDialog(QWidget * _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLScreenShotDialogPrivate(*this))
{
  Q_D(qMRMLScreenShotDialog);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qMRMLScreenShotDialog::~qMRMLScreenShotDialog()
{
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setLayoutManager(qMRMLLayoutManager* newlayoutManager)
{
  Q_D(qMRMLScreenShotDialog);
  d->layoutManager = newlayoutManager;
}

//-----------------------------------------------------------------------------
qMRMLLayoutManager* qMRMLScreenShotDialog::layoutManager() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->layoutManager.data();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setNameEdit(const QString& newName)
{
  Q_D(qMRMLScreenShotDialog);
  d->nameEdit->setText(newName);
  d->nameEdit->setFocus();
  d->nameEdit->selectAll();
}

//-----------------------------------------------------------------------------
QString qMRMLScreenShotDialog::nameEdit() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->nameEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setDescription(const QString& newDescription)
{
  Q_D(qMRMLScreenShotDialog);
  d->descriptionTextEdit->setPlainText(newDescription);
}

//-----------------------------------------------------------------------------
QString qMRMLScreenShotDialog::description() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->descriptionTextEdit->toPlainText();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setData(const QVariant& newData)
{
  Q_D(qMRMLScreenShotDialog);
  d->data = newData;
  d->setWidgetEnabled(!d->data.isValid());
}

//-----------------------------------------------------------------------------
QVariant qMRMLScreenShotDialog::data() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->data;
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setWidgetType(qMRMLScreenShotDialog::WidgetType newType)
{
  Q_D(qMRMLScreenShotDialog);
  d->setCheckedRadioButton(newType);
}

//-----------------------------------------------------------------------------
qMRMLScreenShotDialog::WidgetType qMRMLScreenShotDialog::widgetType() const
{
  Q_D(const qMRMLScreenShotDialog);
  return qMRMLScreenShotDialog::WidgetType(d->widgetTypeGroup->checkedId());
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setScaleFactor(const double& newScaleFactor)
{
  Q_D(qMRMLScreenShotDialog);
  d->scaleFactorSpinBox->setValue(newScaleFactor);
}

//-----------------------------------------------------------------------------
double qMRMLScreenShotDialog::scaleFactor() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->scaleFactorSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setImageData(vtkImageData* screenshot)
{
  Q_D(qMRMLScreenShotDialog);
  d->imageData = screenshot;
  QImage qimage;
  qMRMLUtils::vtkImageDataToQImage(screenshot,qimage);
  // set preview
  d->ScreenshotWidget->setPixmap(QPixmap::fromImage(qimage));
}

//-----------------------------------------------------------------------------
vtkImageData* qMRMLScreenShotDialog::imageData() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->imageData.GetPointer();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setShowScaleFactorSpinBox(const bool& state)
{
  Q_D(qMRMLScreenShotDialog);
  d->scaleFactorSpinBox->setVisible(state);
  d->scaleFactorLabel->setVisible(state);
}

//-----------------------------------------------------------------------------
bool qMRMLScreenShotDialog::showScaleFactorSpinBox() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->scaleFactorSpinBox->isVisible();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::resetDialog()
{
  Q_D(qMRMLScreenShotDialog);

  d->descriptionTextEdit->clear();
  // we want a default name which is easily overwritable by just typing
  // We set the name
  d->nameEdit->clear();

  this->setWidgetType(qMRMLScreenShotDialog::FullLayout);
  this->setScaleFactor(1.0);

  // reset the id
  this->setData(QVariant());
  this->grabScreenShot();
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::grabScreenShot()
{
  if (this->data().isValid())
    {
    // If a data is set, we are in "review" mode, no screenshot can be taken
    return;
    }
  this->grabScreenShot(this->widgetType());
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::grabScreenShot(int screenshotWindow)
{
  Q_D(qMRMLScreenShotDialog);
  QWidget* widget = 0;
  switch (screenshotWindow)
    {
    case qMRMLScreenShotDialog::ThreeD:
      // Create a scrennshot of the first 3DView
      widget = d->layoutManager.data()->threeDWidget(0)->threeDView();
      break;
    case qMRMLScreenShotDialog::Red:
    case qMRMLScreenShotDialog::Yellow:
    case qMRMLScreenShotDialog::Green:
      // Create a screenshot of a specific sliceView
      widget = const_cast<ctkVTKSliceView*>(
          d->layoutManager.data()->sliceWidget(this->enumToString(screenshotWindow))->sliceView());
      break;
    case qMRMLScreenShotDialog::FullLayout:
    default:
      // Create a screenshot of the full layout
      widget = d->layoutManager.data()->viewport();
      break;
    }

  QImage screenShot = ctk::grabVTKWidget(widget);

  // Rescale the image which gets saved
  QImage rescaledScreenShot = screenShot.scaled(screenShot.size().width()
      * d->scaleFactorSpinBox->value(), screenShot.size().height()
      * d->scaleFactorSpinBox->value());

  // convert the screenshot from QPixmap to vtkImageData and store it with this class
  vtkNew<vtkImageData> newImageData;
  qMRMLUtils::qImageToVtkImageData(rescaledScreenShot,
                                   newImageData.GetPointer());
  this->setImageData(newImageData.GetPointer());
}

//-----------------------------------------------------------------------------
QString qMRMLScreenShotDialog::enumToString(int type)
{
  int propIndex = this->metaObject()->indexOfProperty("widgetType");
  QMetaProperty widgetTypeProperty = this->metaObject()->property(propIndex);
  QMetaEnum widgetTypeEnum = widgetTypeProperty.enumerator();
  return widgetTypeEnum.valueToKey(type);
}
