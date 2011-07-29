#include "qSlicerSceneViewsModuleDialog.h"

#include <vtkSlicerSceneViewsModuleLogic.h>
#include <ctkVTKSliceView.h>
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLUtils.h"

#include "vtkImageData.h"
#include "vtkSmartPointer.h"

// QT includes
#include <QPushButton>
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::qSlicerSceneViewsModuleDialog()
{

  this->m_Logic = 0;

  this->m_vtkImageData = 0;

  this->m_Id = vtkStdString("");

  ui.setupUi(this);

  // The restore button has to be configured since it is the reset button in the buttonbox
  // so we set Icons and Text here
  QPushButton* restoreButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  restoreButton->setText("Restore");
  restoreButton->setIcon(QIcon(":/Icons/Restore.png"));

  createConnection();

}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog::~qSlicerSceneViewsModuleDialog()
{

  if(this->m_Logic)
    {
    this->m_Logic = 0;
    }

  if(this->m_vtkImageData)
    {
    this->m_vtkImageData->Delete();
    this->m_vtkImageData = 0;
    }

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::setLogic(vtkSlicerSceneViewsModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the SceneViews module logic here!");
    return;
    }

  this->m_Logic = logic;

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::initialize(const QString& nodeId)
{
  if (!this->m_Logic)
    {
    qErrnoWarning("initialize: We need the SceneViews module logic here!");
    return;
    }

  this->m_Id = vtkStdString(nodeId.toLatin1());

  // get the name..
  vtkStdString name = this->m_Logic->GetSceneViewName(this->m_Id.c_str());

  // ..and set it in the GUI
  this->ui.nameEdit->setText(name.c_str());

  // get the description..
  vtkStdString description = this->m_Logic->GetSceneViewDescription(this->m_Id);

  // ..and set it in the GUI
  this->ui.descriptionTextEdit->setText(description.c_str());

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSceneViewScreenshotType(this->m_Id);

  // ..and set it in the GUI
  switch(screenshotType)
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

  // we want to disable the modification of the screenshot since this SceneView was created earlier
  // to change a screenshot of an old SceneView, there is a workaround by restoring it, deleting it and creating a new one
  this->ui.threeDViewRadio->setEnabled(false);
  this->ui.redSliceViewRadio->setEnabled(false);
  this->ui.yellowSliceViewRadio->setEnabled(false);
  this->ui.greenSliceViewRadio->setEnabled(false);
  this->ui.fullLayoutRadio->setEnabled(false);

  // get the actual screenshot..
//  this->m_vtkImageData = this->m_Logic->GetSceneViewScreenshot(this->m_Id);
//  vtkImageData *imageData = this->m_Logic->GetSceneViewScreenshot(this->m_Id);
  vtkSmartPointer<vtkImageData> copyScreenshot = this->m_Logic->GetSceneViewScreenshot(this->m_Id);
  if (!this->m_vtkImageData)
    {
    this->m_vtkImageData = vtkImageData::New();
    }
  this->m_vtkImageData->DeepCopy(copyScreenshot);

  // ..and convert it from vtkImageData to QImage..
  QImage qimage;
  qMRMLUtils::vtkImageDataToQImage(this->m_vtkImageData,qimage);
//  qMRMLUtils::vtkImageDataToQImage(imageData,qimage);

  // ..and then to QPixmap..
  QPixmap screenshot;
  screenshot = QPixmap::fromImage(qimage, Qt::AutoColor);

  // ..and set it to the gui..
  ui.screenshotPlaceholder->setPixmap(screenshot.scaled(this->ui.screenshotPlaceholder->width(),this->ui.screenshotPlaceholder->height(),
        Qt::KeepAspectRatio,Qt::SmoothTransformation));

  // now we are able to restore a SceneView, so enable the button
  QPushButton* restoreButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  restoreButton->setVisible(true);

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::createConnection()
{
  // connect the OK and CANCEL button to the individual Slots
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  this->connect(ui.threeDViewRadio, SIGNAL(clicked()), this, SLOT(onThreeDViewRadioClicked()));
  this->connect(ui.redSliceViewRadio, SIGNAL(clicked()), this, SLOT(onRedSliceViewRadioClicked()));
  this->connect(ui.yellowSliceViewRadio, SIGNAL(clicked()), this, SLOT(onYellowSliceViewRadioClicked()));
  this->connect(ui.greenSliceViewRadio, SIGNAL(clicked()), this, SLOT(onGreenSliceViewRadioClicked()));
  this->connect(ui.fullLayoutRadio, SIGNAL(clicked()), this, SLOT(onFullLayoutRadioClicked()));

  QPushButton* restoreButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  this->connect(restoreButton, SIGNAL(clicked()), this, SLOT(onRestoreButtonClicked()));
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onDialogAccepted()
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

  if (!strcmp(this->m_Id,""))
    {
    // this is a new SceneView
    this->m_Logic->CreateSceneView(nameBytes.data(),descriptionBytes.data(),screenshotType,this->m_vtkImageData);

    //QMessageBox::information(this, "3D Slicer SceneView created",
    //                           "A new SceneView was created and the current scene was attached.");

    }
  else
    {
    // this SceneView already exists
    this->m_Logic->ModifySceneView(this->m_Id,nameBytes.data(),descriptionBytes.data(),screenshotType,this->m_vtkImageData);

    //QMessageBox::information(this, "3D Slicer SceneView updated",
    //                               "The SceneView was updated without changing the attached scene.");

    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onThreeDViewRadioClicked()
{
  this->grabScreenShot("ThreeD");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onRedSliceViewRadioClicked()
{
  this->grabScreenShot("Red");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onYellowSliceViewRadioClicked()
{
  this->grabScreenShot("Yellow");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onGreenSliceViewRadioClicked()
{
  this->grabScreenShot("Green");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onFullLayoutRadioClicked()
{
  this->grabScreenShot("");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::onRestoreButtonClicked()
{
  this->m_Logic->RestoreSceneView(this->m_Id);

  //QMessageBox::information(this, "3D Slicer SceneView updated",
  //                               "The SceneView was restored including the attached scene.");
  this->accept();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::reset()
{
  this->ui.threeDViewRadio->setEnabled(true);
  this->ui.redSliceViewRadio->setEnabled(true);
  this->ui.yellowSliceViewRadio->setEnabled(true);
  this->ui.greenSliceViewRadio->setEnabled(true);
  this->ui.fullLayoutRadio->setEnabled(true);
  this->ui.threeDViewRadio->setChecked(false);
  this->ui.redSliceViewRadio->setChecked(false);
  this->ui.yellowSliceViewRadio->setChecked(false);
  this->ui.greenSliceViewRadio->setChecked(false);
  this->ui.fullLayoutRadio->setChecked(true);
  this->grabScreenShot("");
  this->ui.descriptionTextEdit->clear();

  QPushButton* restoreButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  restoreButton->setVisible(false);

  // we want a default name which is easily overwritable by just typing
  QString name = this->ui.nameEdit->text();
  if (name.size() > 0)
    {
    // check to see if it's an already used name for a node (redrawing the
    // dialog causes it to reset and calling GetUniqueNameByString increments
    // the number each time).
    QByteArray nameBytes = name.toLatin1();
    vtkCollection *col = this->m_Logic->GetMRMLScene()->GetNodesByName(nameBytes.data());
    if (col->GetNumberOfItems() > 0)
      {
      // get a new unique name
      this->ui.nameEdit->setText(this->m_Logic->GetMRMLScene()->GetUniqueNameByString("SceneView"));
      }
    col->RemoveAllItems();
    col->Delete();
    }
  else
    {
    this->ui.nameEdit->setText(this->m_Logic->GetMRMLScene()->GetUniqueNameByString("SceneView"));
    }
  this->ui.nameEdit->setFocus();
  this->ui.nameEdit->selectAll();



  // reset the id
  this->m_Id = vtkStdString("");

}

//-----------------------------------------------------------------------------
// Grab a screenshot of the 3DView or any sliceView.
// The screenshotWindow is Red, Green, Yellow for a sliceView or empty for a ThreeDView
//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleDialog::grabScreenShot(QString screenshotWindow)
{

  QWidget* widget = 0;

  if(screenshotWindow.length()>0)
    {
    if (screenshotWindow=="ThreeD")
      {
      // create a screenShot of the first 3DView
      widget = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView();
      }
    else
      {
      // create a screenShot of a specific sliceView
      widget = const_cast<ctkVTKSliceView*>(qSlicerApplication::application()->layoutManager()->sliceWidget(screenshotWindow)->sliceView());
      }
    }
  else
    {
    // create a screenShot of the full layout
    widget = qSlicerApplication::application()->layoutManager()->viewport();
    }

  // this is a hack right now for platforms other than mac
  // the dialog sometimes blocked the screenshot so we hide it while we take the screenshot
  //this->setVisible(false);
  this->hide();

  QPixmap screenShot = QPixmap::grabWidget(widget);

  //this->setVisible(true);
  this->show();


  ui.screenshotPlaceholder->setPixmap(screenShot.scaled(this->ui.screenshotPlaceholder->width(),this->ui.screenshotPlaceholder->height(),
      Qt::KeepAspectRatio,Qt::SmoothTransformation));


  // convert the screenshot from QPixmap to vtkImageData and store it with this class
  if ( this->m_vtkImageData == NULL)
    {
    this->m_vtkImageData =  vtkImageData::New();
    }
  qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), this->m_vtkImageData);

}
