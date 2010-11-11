#include "qSlicerAnnotationModuleSnapShotDialog.h"

#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include <ctkVTKSliceView.h>
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLThreeDView.h"


#include "vtkImageData.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::qSlicerAnnotationModuleSnapShotDialog()
{

  this->m_Logic = 0;

  this->m_vtkImageData = 0;

  this->m_Id = 0;

  ui.setupUi(this);
  createConnection();

}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::~qSlicerAnnotationModuleSnapShotDialog()
{

  if(this->m_Logic)
    {
    this->m_Logic->Delete();
    this->m_Logic = 0;
    }

  if(this->m_vtkImageData)
    {
    this->m_vtkImageData->Delete();
    this->m_vtkImageData = 0;
    }

  if(this->m_Id)
    {
    delete this->m_Id;
    this->m_Id = 0;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("We need the Annotation module logic here!");
    return;
    }

  this->m_Logic = logic;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::createConnection()
{

  // connect the OK and CANCEL button to the individual Slots
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  this->connect(ui.threeDViewRadio, SIGNAL(clicked()), this, SLOT(onThreeDViewRadioClicked()));
  this->connect(ui.redSliceViewRadio, SIGNAL(clicked()), this, SLOT(onRedSliceViewRadioClicked()));
  this->connect(ui.yellowSliceViewRadio, SIGNAL(clicked()), this, SLOT(onYellowSliceViewRadioClicked()));
  this->connect(ui.greenSliceViewRadio, SIGNAL(clicked()), this, SLOT(onGreenSliceViewRadioClicked()));

  this->connect(ui.restoreButton, SIGNAL(clicked()), this, SLOT(onRestoreButtonClicked()));

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
  QString name = ui.nameEdit->text();
  QByteArray nameBytes = name.toAscii();

  // description
  QString description = ui.descriptionTextEdit->toPlainText();
  QByteArray descriptionBytes = description.toAscii();

  this->m_Logic->CreateSnapShot(nameBytes.data(),descriptionBytes.data(),this->m_vtkImageData);

  // emit an event which gets caught by main GUI window
  emit dialogAccepted();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::onThreeDViewRadioClicked()
{
  this->grabScreenShot("");
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
void qSlicerAnnotationModuleSnapShotDialog::onRestoreButtonClicked()
{
  this->m_Logic->RestoreSnapShot(this->m_Id);

  emit dialogAccepted();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::reset()
{
  this->ui.threeDViewRadio->setChecked(true);
  this->ui.redSliceViewRadio->setChecked(false);
  this->ui.yellowSliceViewRadio->setChecked(false);
  this->ui.greenSliceViewRadio->setChecked(false);
  this->grabScreenShot("");
  this->ui.descriptionTextEdit->clear();
  this->ui.nameEdit->clear();
}

//-----------------------------------------------------------------------------
// Grab a screenshot of the 3DView or any sliceView.
// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::grabScreenShot(QString screenshotWindow)
{

  QWidget* widget = 0;

  if(screenshotWindow.length()>0)
    {
    // create a screenShot of a specific sliceView
     widget = static_cast<QWidget*>(qSlicerApplication::application()->layoutManager()->sliceWidget(screenshotWindow)->getSliceView());
    }
  else
    {
    // create a screenShot of the first 3DView
    widget = static_cast<QWidget*>(qSlicerApplication::application()->layoutManager()->threeDView(0));
    }

  QPixmap screenShot = QPixmap::grabWidget(widget);

  ui.screenshotPlaceholder->setPixmap(screenShot.scaled(this->ui.screenshotPlaceholder->width(),this->ui.screenshotPlaceholder->height(),
      Qt::KeepAspectRatio,Qt::SmoothTransformation));


  // convert the screenshot from QPixmap to vtkImageData and store it with this class
  vtkImageData* vtkimage = vtkImageData::New();
  this->m_Logic->QImageToVtkImageData(screenShot.toImage(),vtkimage);
  this->m_vtkImageData = vtkimage;

}


