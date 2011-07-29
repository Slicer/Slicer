#include "qSlicerAnnotationModuleSnapShotDialog.h"

#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include <ctkVTKSliceView.h>
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLUtils.h"

#include "vtkImageData.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::qSlicerAnnotationModuleSnapShotDialog()
{

  this->m_Logic = 0;

  this->m_vtkImageData = 0;

  this->m_Id = vtkStdString("");

  ui.setupUi(this);

  createConnection();

}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::~qSlicerAnnotationModuleSnapShotDialog()
{

  if (this->m_Logic)
    {
    this->m_Logic = 0;
    }

  if (this->m_vtkImageData)
    {
    this->m_vtkImageData->Delete();
    this->m_vtkImageData = 0;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the Annotation module logic here!");
    return;
    }

  this->m_Logic = logic;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::initialize(const char* nodeId)
{
  if (!this->m_Logic)
    {
    qErrnoWarning("initialize: We need the Annotation module logic here!");
    return;
    }

  this->m_Id = vtkStdString(nodeId);

  // get the name..
  vtkStdString name = this->m_Logic->GetAnnotationName(this->m_Id.c_str());

  // ..and set it in the GUI
  this->ui.nameEdit->setText(name.c_str());

  // get the description..
  vtkStdString description = this->m_Logic->GetSnapShotDescription(this->m_Id);

  // ..and set it in the GUI
  this->ui.descriptionTextEdit->setText(description.c_str());

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSnapShotScreenshotType(this->m_Id);

  // ..and set it in the GUI
  switch (screenshotType)
    {
    case 0:
      // 3D view
      this->ui.threeDViewRadio->setChecked(true);
      break;
    case 1:
      // red Slice view
      this->ui.redSliceViewRadio->setChecked(true);
      break;
    case 2:
      // yellow Slice view
      this->ui.yellowSliceViewRadio->setChecked(true);
      break;
    case 3:
      // green Slice view
      this->ui.greenSliceViewRadio->setChecked(true);
      break;
    case 4:
      // full Layout
      this->ui.fullLayoutRadio->setChecked(true);
    default:
      // as fallback, just treat this case as a full layout
      this->ui.fullLayoutRadio->setChecked(true);
    }

  double scaleFactor = this->m_Logic->GetSnapShotScaleFactor(this->m_Id);
  this->ui.scaleFactorSpinBox->setValue(scaleFactor);
  this->ui.scaleFactorSpinBox->setEnabled(false);

  // we want to disable the modification of the screenshot since this snapshot was created earlier
  // to change a screenshot of an old snapshot, there is a workaround by restoring it, deleting it and creating a new one
  this->ui.threeDViewRadio->setEnabled(false);
  this->ui.redSliceViewRadio->setEnabled(false);
  this->ui.yellowSliceViewRadio->setEnabled(false);
  this->ui.greenSliceViewRadio->setEnabled(false);
  this->ui.fullLayoutRadio->setEnabled(false);

  // get the actual screenshot..
  this->m_vtkImageData = this->m_Logic->GetSnapShotScreenshot(this->m_Id);

  // ..and convert it from vtkImageData to QImage..
  QImage qimage;
  qMRMLUtils::vtkImageDataToQImage(this->m_vtkImageData, qimage);

  // ..and then to QPixmap..
  QPixmap screenshot;
  screenshot = QPixmap::fromImage(qimage, Qt::AutoColor);

  // ..and set it to the gui..
  ui.screenshotPlaceholder->setPixmap(screenshot.scaled(
      this->ui.screenshotPlaceholder->width(),
      this->ui.screenshotPlaceholder->height(), Qt::KeepAspectRatio,
      Qt::SmoothTransformation));

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::createConnection()
{

  // connect the OK and CANCEL button to the individual Slots
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  this->connect(ui.threeDViewRadio, SIGNAL(clicked()), this, SLOT(
      onThreeDViewRadioClicked()));
  this->connect(ui.redSliceViewRadio, SIGNAL(clicked()), this, SLOT(
      onRedSliceViewRadioClicked()));
  this->connect(ui.yellowSliceViewRadio, SIGNAL(clicked()), this, SLOT(
      onYellowSliceViewRadioClicked()));
  this->connect(ui.greenSliceViewRadio, SIGNAL(clicked()), this, SLOT(
      onGreenSliceViewRadioClicked()));
  this->connect(ui.fullLayoutRadio, SIGNAL(clicked()), this, SLOT(
      onFullLayoutRadioClicked()));
  this->connect(ui.scaleFactorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(
      onScaleFactorSpinBoxChanged()));

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onDialogRejected()
{

  // emit an event which gets caught by main GUI window
  emit dialogRejected();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onDialogAccepted()
{

  // name
  QString name = this->ui.nameEdit->text();
  QByteArray nameBytes = name.toLatin1();

  // description
  QString description = this->ui.descriptionTextEdit->toPlainText();
  QByteArray descriptionBytes = description.toLatin1();

  // we need to know of which type the screenshot is
  int screenshotType = -1;

  if (this->ui.threeDViewRadio->isChecked())
    {
    screenshotType = 0;
    }
  else if (this->ui.redSliceViewRadio->isChecked())
    {
    screenshotType = 1;
    }
  else if (this->ui.yellowSliceViewRadio->isChecked())
    {
    screenshotType = 2;
    }
  else if (this->ui.greenSliceViewRadio->isChecked())
    {
    screenshotType = 3;
    }
  else if (this->ui.fullLayoutRadio->isChecked())
    {
    screenshotType = 4;
    }

  if (!strcmp(this->m_Id, ""))
    {
    // this is a new snapshot
    this->m_Logic->CreateSnapShot(nameBytes.data(), descriptionBytes.data(),
        screenshotType, this->ui.scaleFactorSpinBox->value(), this->m_vtkImageData);
    }
  else
    {
    // this snapshot already exists
    this->m_Logic->ModifySnapShot(this->m_Id, nameBytes.data(),
        descriptionBytes.data(), screenshotType, this->ui.scaleFactorSpinBox->value(), this->m_vtkImageData);
    }

  // emit an event which gets caught by main GUI window
  emit dialogAccepted();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onThreeDViewRadioClicked()
{
  this->grabScreenShot("ThreeD");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onRedSliceViewRadioClicked()
{
  this->grabScreenShot("Red");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onYellowSliceViewRadioClicked()
{
  this->grabScreenShot("Yellow");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onGreenSliceViewRadioClicked()
{
  this->grabScreenShot("Green");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onFullLayoutRadioClicked()
{
  this->grabScreenShot("");
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onScaleFactorSpinBoxChanged()
{

  // check which screenshot should be grabbed

  if (this->ui.fullLayoutRadio->isChecked())
    {
    // Full Layout
    this->grabScreenShot("");
    }
  else if (this->ui.threeDViewRadio->isChecked())
    {
    // 3D view
    this->grabScreenShot("ThreeD");
    }
  else if (this->ui.redSliceViewRadio->isChecked())
    {
    // Red
    this->grabScreenShot("Red");
    }
  else if (this->ui.yellowSliceViewRadio->isChecked())
    {
    // Yellow
    this->grabScreenShot("Yellow");
    }
  else if (this->ui.greenSliceViewRadio->isChecked())
    {
    // Green
    this->grabScreenShot("Green");
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::reset()
{
  this->ui.fullLayoutRadio->setEnabled(true);
  this->ui.threeDViewRadio->setEnabled(true);
  this->ui.redSliceViewRadio->setEnabled(true);
  this->ui.yellowSliceViewRadio->setEnabled(true);
  this->ui.greenSliceViewRadio->setEnabled(true);

  this->ui.threeDViewRadio->setChecked(false);
  this->ui.redSliceViewRadio->setChecked(false);
  this->ui.yellowSliceViewRadio->setChecked(false);
  this->ui.greenSliceViewRadio->setChecked(false);
  this->ui.fullLayoutRadio->setChecked(true);

  this->ui.scaleFactorSpinBox->setEnabled(true);
  this->ui.scaleFactorSpinBox->setValue(1.0);

  this->grabScreenShot("");
  this->ui.descriptionTextEdit->clear();

  // we want a default name which is easily overwritable by just typing
  this->ui.nameEdit->setText(
      this->m_Logic->GetMRMLScene()->GetUniqueNameByString("Screenshot"));
  this->ui.nameEdit->setFocus();
  this->ui.nameEdit->selectAll();

  // reset the id
  this->m_Id = vtkStdString("");
}

//-----------------------------------------------------------------------------
// Grab a screenshot of the 3DView or any sliceView.
// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::grabScreenShot(QString screenshotWindow)
{
  QWidget* widget = 0;

  if (screenshotWindow.length() > 0)
    {
    if (screenshotWindow == "ThreeD")
      {
      // create a screenShot of the first 3DView
      widget
          = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView();
      }
    else
      {
      // create a screenShot of a specific sliceView
      widget
          = const_cast<ctkVTKSliceView*> (qSlicerApplication::application()->layoutManager()->sliceWidget(
              screenshotWindow)->sliceView());
      }
    }
  else
    {
    // create a screenShot of the full layout
    widget = qSlicerApplication::application()->layoutManager()->viewport();
    }

  // this is a hack right now for platforms other than mac
  // the dialog sometimes blocked the screenshot so we hide it while we take the screenshot
  this->setVisible(false);

  QPixmap screenShot = QPixmap::grabWidget(widget);

  this->setVisible(true);

  // set preview (unscaled)
  ui.screenshotPlaceholder->setPixmap(screenShot.scaled(
      this->ui.screenshotPlaceholder->width(),
      this->ui.screenshotPlaceholder->height(), Qt::KeepAspectRatio,
      Qt::SmoothTransformation));

  // Rescale the image which gets saved
  QPixmap rescaledScreenShot = screenShot.scaled(screenShot.size().width()
      * this->ui.scaleFactorSpinBox->value(), screenShot.size().height()
      * this->ui.scaleFactorSpinBox->value());

  // convert the screenshot from QPixmap to vtkImageData and store it with this class
  vtkImageData* vtkimage = vtkImageData::New();
  qMRMLUtils::qImageToVtkImageData(rescaledScreenShot.toImage(), vtkimage);
  this->m_vtkImageData = vtkimage;
}

