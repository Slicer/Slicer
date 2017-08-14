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
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QWeakPointer>
#else
#include <QPointer>
#endif

// CTK includes
#include <ctkVTKWidgetsUtils.h>

// qMRML includes
#include "qMRMLLayoutManager.h"
#include <qMRMLSliceView.h>
#include "qMRMLSliceWidget.h"
#include "qMRMLScreenShotDialog.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLUtils.h"

#include "ui_qMRMLScreenShotDialog.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkRendererCollection.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindow.h>
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
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  QWeakPointer<qMRMLLayoutManager>   LayoutManager;
#else
  QPointer<qMRMLLayoutManager>   LayoutManager;
#endif
  vtkSmartPointer<vtkImageData>      ImageData;
  /// The ID of the associated snapshot node.
  /// This is NULL if the dialog has no associated snapshot node (== new snapshot mode).
  QVariant                           Data;
  QButtonGroup*                      WidgetTypeGroup;

  /// The last selected thumbnail type
  int LastWidgetType;

};

//-----------------------------------------------------------------------------
qMRMLScreenShotDialogPrivate::qMRMLScreenShotDialogPrivate(qMRMLScreenShotDialog &object)
  : q_ptr(&object)
{
  qRegisterMetaType<qMRMLScreenShotDialog::WidgetType>(
      "qMRMLScreenShotDialog::WidgetType");
  this->WidgetTypeGroup = 0;

  this->LastWidgetType = qMRMLScreenShotDialog::FullLayout;
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialogPrivate::setupUi(QDialog* dialog)
{
  Q_Q(qMRMLScreenShotDialog);

  this->Ui_qMRMLScreenShotDialog::setupUi(dialog);
  this->WidgetTypeGroup = new QButtonGroup(dialog);
  this->WidgetTypeGroup->setExclusive(true);
  this->WidgetTypeGroup->addButton(this->fullLayoutRadio, qMRMLScreenShotDialog::FullLayout);
  this->WidgetTypeGroup->addButton(this->threeDViewRadio, qMRMLScreenShotDialog::ThreeD);
  this->WidgetTypeGroup->addButton(this->redSliceViewRadio, qMRMLScreenShotDialog::Red);
  this->WidgetTypeGroup->addButton(this->yellowSliceViewRadio, qMRMLScreenShotDialog::Yellow);
  this->WidgetTypeGroup->addButton(this->greenSliceViewRadio, qMRMLScreenShotDialog::Green);

  this->setCheckedRadioButton(this->LastWidgetType);

  QObject::connect(this->WidgetTypeGroup, SIGNAL(buttonClicked(int)),
                   q, SLOT(setLastWidgetType(int)));
}


//-----------------------------------------------------------------------------
void qMRMLScreenShotDialogPrivate::setCheckedRadioButton(int type)
{
  QRadioButton* widgetButton =
    qobject_cast<QRadioButton*>(this->WidgetTypeGroup->button(type));
  if (widgetButton)
    {
    // this can crash if an invalid type is passed in
    widgetButton->setChecked(true);
    }
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
  d->LayoutManager = newlayoutManager;
}

//-----------------------------------------------------------------------------
void qMRMLScreenShotDialog::setLastWidgetType(int id)
{
  Q_D(qMRMLScreenShotDialog);
  d->LastWidgetType = id;
}

//-----------------------------------------------------------------------------
qMRMLLayoutManager* qMRMLScreenShotDialog::layoutManager() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->LayoutManager.data();
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
  d->Data = newData;
  d->setWidgetEnabled(!d->Data.isValid());
}

//-----------------------------------------------------------------------------
QVariant qMRMLScreenShotDialog::data() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->Data;
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
  return qMRMLScreenShotDialog::WidgetType(d->WidgetTypeGroup->checkedId());
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
  d->ImageData = screenshot;
  QImage qimage;
  qMRMLUtils::vtkImageDataToQImage(screenshot,qimage);
  // set preview
  d->ScreenshotWidget->setPixmap(QPixmap::fromImage(qimage));
}

//-----------------------------------------------------------------------------
vtkImageData* qMRMLScreenShotDialog::imageData() const
{
  Q_D(const qMRMLScreenShotDialog);
  return d->ImageData.GetPointer();
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

  // set the widget type to the last one used
  this->setWidgetType(qMRMLScreenShotDialog::WidgetType(d->LastWidgetType));

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
      // Create a screenshot of the first 3DView
      widget = d->LayoutManager.data()->threeDWidget(0)->threeDView();
      break;
    case qMRMLScreenShotDialog::Red:
    case qMRMLScreenShotDialog::Yellow:
    case qMRMLScreenShotDialog::Green:
      // Create a screenshot of a specific sliceView
      widget = const_cast<qMRMLSliceView*>(
          d->LayoutManager.data()->sliceWidget(this->enumToString(screenshotWindow))->sliceView());
      break;
    case qMRMLScreenShotDialog::FullLayout:
    default:
      // Create a screenshot of the full layout
      widget = d->LayoutManager.data()->viewport();
      break;
    }

  double scaleFactor = d->scaleFactorSpinBox->value();

  vtkNew<vtkImageData> newImageData;
  if (scaleFactor != 1 &&
      screenshotWindow  == qMRMLScreenShotDialog::ThreeD)
    {
    // use off screen rendering to magnifiy the VTK widget's image without interpolation
    // TODO: fix VTK so that the slice windows are scaled rather than tiled
    vtkRenderer *renderer = NULL;
    renderer = vtkRenderer::SafeDownCast(d->LayoutManager.data()->threeDWidget(0)->threeDView()->renderWindow()->GetRenderers()->GetItemAsObject(0));
    vtkNew<vtkRenderLargeImage> renderLargeImage;
    renderLargeImage->SetInput(renderer);
    renderLargeImage->SetMagnification(scaleFactor);
    renderLargeImage->Update();
    newImageData.GetPointer()->DeepCopy(renderLargeImage->GetOutput());
    }
  else
    {
    // no scaling, or for not just the 3D window
    QImage screenShot = ctk::grabVTKWidget(widget);

    if (scaleFactor != 1.0)
      {
      // Rescale the image which gets saved
      QImage rescaledScreenShot = screenShot.scaled(screenShot.size().width() * scaleFactor,
                                                    screenShot.size().height() * scaleFactor);

      // convert the scaled screenshot from QPixmap to vtkImageData
      qMRMLUtils::qImageToVtkImageData(rescaledScreenShot,
                                       newImageData.GetPointer());
      }
    else
      {
      // convert the screenshot from QPixmap to vtkImageData
      qMRMLUtils::qImageToVtkImageData(screenShot,
                                       newImageData.GetPointer());
      }
    }
  // save the screen shot image to this class
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
