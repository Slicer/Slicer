/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavCalibrationStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkAppendPolyData.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerVolumesLogic.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkCollection.h"

#include "vtkSlicerColorLogic.h"

#include "igtlMath.h"

#include "newmat.h"

#define DEBUG_ZFRAME

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavCalibrationStep);
vtkCxxRevisionMacro(vtkProstateNavCalibrationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::vtkProstateNavCalibrationStep()
{
  this->SetName("3/5. Calibration");
  this->SetDescription("Perform Z-frame calibration.");

  this->SelectImageFrame  = NULL;
  this->SelectImageButton = NULL;
  this->CalibrateButton   = NULL;

  this->ZFrameSettingFrame = NULL;
  this->ShowZFrameCheckButton = NULL;

  //ZFrameModelNode = NULL;
  /*
  this->ZFrameModelNodeID = "";
  this->ZFrameTransformNodeID = "";
  */
}

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::~vtkProstateNavCalibrationStep()
{
  if (this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }
}

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowUserInterface()
{

  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // Create frame
  if (!this->SelectImageFrame)
    {
    this->SelectImageFrame = vtkKWFrame::New();
    this->SelectImageFrame->SetParent(parent);
    this->SelectImageFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->SelectImageFrame->GetWidgetName());

  if (!this->SelectImageButton)
    {
    this->SelectImageButton = vtkKWLoadSaveButtonWithLabel::New();
    this->SelectImageButton->SetParent(this->SelectImageFrame);
    this->SelectImageButton->Create();
    this->SelectImageButton->SetWidth(50);
    this->SelectImageButton->GetWidget()->SetText ("Browse Image File");
    /*
    this->SelectImageButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
      "{ {ProstateNav} {*.dcm} }");
    */
    this->SelectImageButton->GetWidget()->GetLoadSaveDialog()
      ->RetrieveLastPathFromRegistry("OpenPath");
    }

  this->Script("pack %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->SelectImageButton->GetWidgetName());

  if (!this->CalibrateButton)
    {
    this->CalibrateButton = vtkKWPushButton::New();
    this->CalibrateButton->SetParent (this->SelectImageFrame);
    this->CalibrateButton->Create ( );
    this->CalibrateButton->SetText ("Perform Calibration");
    this->CalibrateButton->SetBalloonHelpString("Send Calibration Data to the Robot");
    this->CalibrateButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                       (vtkCommand *)this->GUICallbackCommand);
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->CalibrateButton->GetWidgetName());

  
  if (!this->ZFrameSettingFrame)
    {
    this->ZFrameSettingFrame = vtkKWFrame::New();
    this->ZFrameSettingFrame->SetParent(parent);
    this->ZFrameSettingFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->ZFrameSettingFrame->GetWidgetName());
  
  if (!this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton = vtkKWCheckButton::New();
    this->ShowZFrameCheckButton->SetParent(this->ZFrameSettingFrame);
    this->ShowZFrameCheckButton->Create();
    this->ShowZFrameCheckButton->SelectedStateOff();
    this->ShowZFrameCheckButton->SetText("Show ZFrame");
    this->ShowZFrameCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->ShowZFrameCheckButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ProcessGUIEvents(vtkObject *caller,
                                           unsigned long event, void *callData)
{

  if (this->ShowZFrameCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int checked = this->ShowZFrameCheckButton->GetSelectedState(); 
    if (checked)
      {
      ShowZFrameModel();
      }
    else
      {
      HideZFrameModel();
      }
    }
  if (this->CalibrateButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    const char *filename = this->SelectImageButton->GetWidget()->GetFileName();
    PerformZFrameCalibration(filename);
    }
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowZFrameModel()
{

  //if (this->ZFrameModelNodeID.length() == 0 ||
  //    this->MRMLScene->GetNodeByID(this->ZFrameModelNodeID) == NULL)
  //  {
  //  this->ZFrameModelNodeID = AddZFrameModel("ZFrame");
  //  }

  vtkMRMLModelNode*  modelNode =
    //vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ZFrameModelNodeID.c_str()));
    vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameModelNodeID()));

  vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
  displayNode->SetVisibility(1);
  modelNode->Modified();
  this->MRMLScene->Modified();
  
  //if (this->ZFrameTransformNodeID.length() == 0 ||
  //    this->MRMLScene->GetNodeByID(this->ZFrameTransformNodeID) == NULL)
  //  {
  //  this->ZFrameTransformNodeID = AddZFrameTransform("ZFrameTransform");
  //  }
  //
  //modelNode->SetAndObserveTransformNodeID(this->ZFrameTransformNodeID.c_str());

}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::HideZFrameModel()
{

  vtkMRMLModelNode*  modelNode =
    vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameModelNodeID()));

  if (modelNode)
    {
    vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
    displayNode->SetVisibility(0);
    modelNode->Modified();
    this->MRMLScene->Modified();
    }
}


//----------------------------------------------------------------------------
const char* vtkProstateNavCalibrationStep::AddZFrameModel(const char* nodeName)
{
  std::string rstr;

  vtkMRMLModelNode           *zframeModel;
  vtkMRMLModelDisplayNode    *zframeDisp;

  zframeModel = vtkMRMLModelNode::New();
  zframeDisp = vtkMRMLModelDisplayNode::New();

  this->MRMLScene->SaveStateForUndo();
  this->MRMLScene->AddNode(zframeDisp);
  this->MRMLScene->AddNode(zframeModel);  

  zframeDisp->SetScene(this->MRMLScene);
  zframeModel->SetName(nodeName);
  zframeModel->SetScene(this->MRMLScene);
  zframeModel->SetAndObserveDisplayNodeID(zframeDisp->GetID());
  zframeModel->SetHideFromEditors(0);

  // construct Z-frame model
  const double length = 60; // mm

  //----- cylinder 1 (R-A) -----
  vtkCylinderSource *cylinder1 = vtkCylinderSource::New();
  cylinder1->SetRadius(1.5);
  cylinder1->SetHeight(length);
  cylinder1->SetCenter(0, 0, 0);
  cylinder1->Update();
  
  vtkTransformPolyDataFilter *tfilter1 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans1 =   vtkTransform::New();
  trans1->Translate(length/2.0, length/2.0, 0.0);
  trans1->RotateX(90.0);
  trans1->Update();
  tfilter1->SetInput(cylinder1->GetOutput());
  tfilter1->SetTransform(trans1);
  tfilter1->Update();


  //----- cylinder 2 (R-center) -----
  vtkCylinderSource *cylinder2 = vtkCylinderSource::New();
  cylinder2->SetRadius(1.5);
  cylinder2->SetHeight(length*1.4142135);
  cylinder2->SetCenter(0, 0, 0);
  cylinder2->Update();

  vtkTransformPolyDataFilter *tfilter2 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans2 =   vtkTransform::New();
  trans2->Translate(length/2.0, 0.0, 0.0);
  trans2->RotateX(90.0);
  trans2->RotateX(-45.0);
  trans2->Update();
  tfilter2->SetInput(cylinder2->GetOutput());
  tfilter2->SetTransform(trans2);
  tfilter2->Update();


  //----- cylinder 3 (R-P) -----
  vtkCylinderSource *cylinder3 = vtkCylinderSource::New();
  cylinder3->SetRadius(1.5);
  cylinder3->SetHeight(length);
  cylinder3->SetCenter(0, 0, 0);
  cylinder3->Update();

  vtkTransformPolyDataFilter *tfilter3 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans3 =   vtkTransform::New();
  trans3->Translate(length/2.0, -length/2.0, 0.0);
  trans3->RotateX(90.0);
  trans3->Update();
  tfilter3->SetInput(cylinder3->GetOutput());
  tfilter3->SetTransform(trans3);
  tfilter3->Update();


  //----- cylinder 4 (center-P) -----  
  vtkCylinderSource *cylinder4 = vtkCylinderSource::New();
  cylinder4->SetRadius(1.5);
  cylinder4->SetHeight(length*1.4142135);
  cylinder4->SetCenter(0, 0, 0);
  cylinder4->Update();

  vtkTransformPolyDataFilter *tfilter4 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans4 =   vtkTransform::New();
  trans4->Translate(0.0, -length/2.0, 0.0);
  trans4->RotateX(90.0);
  trans4->RotateZ(-45.0);
  trans4->Update();
  tfilter4->SetInput(cylinder4->GetOutput());
  tfilter4->SetTransform(trans4);
  tfilter4->Update();


  //----- cylinder 5 (L-P) -----  
  vtkCylinderSource *cylinder5 = vtkCylinderSource::New();
  cylinder5->SetRadius(1.5);
  cylinder5->SetHeight(length);
  cylinder5->SetCenter(0, 0, 0);
  cylinder5->Update();

  vtkTransformPolyDataFilter *tfilter5 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans5 =   vtkTransform::New();
  trans5->Translate(-length/2.0, -length/2.0, 0.0);
  trans5->RotateX(90.0);
  trans5->Update();
  tfilter5->SetInput(cylinder5->GetOutput());
  tfilter5->SetTransform(trans5);
  tfilter5->Update();


  //----- cylinder 6 (L-center) -----  
  vtkCylinderSource *cylinder6 = vtkCylinderSource::New();
  cylinder6->SetRadius(1.5);
  cylinder6->SetHeight(length*1.4142135);
  cylinder6->SetCenter(0, 0, 0);
  cylinder6->Update();

  vtkTransformPolyDataFilter *tfilter6 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans6 =   vtkTransform::New();
  trans6->Translate(-length/2.0, 0.0, 0.0);
  trans6->RotateX(90.0);
  trans6->RotateX(45.0);
  trans6->Update();
  tfilter6->SetInput(cylinder6->GetOutput());
  tfilter6->SetTransform(trans6);
  tfilter6->Update();


  //----- cylinder 7 (L-A) -----  
  vtkCylinderSource *cylinder7 = vtkCylinderSource::New();
  cylinder7->SetRadius(1.5);
  cylinder7->SetHeight(length);
  cylinder7->SetCenter(0, 0, 0);
  cylinder7->Update();

  vtkTransformPolyDataFilter *tfilter7 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans7 =   vtkTransform::New();
  trans7->Translate(-length/2.0, length/2.0, 0.0);
  trans7->RotateX(90.0);
  trans7->Update();
  tfilter7->SetInput(cylinder7->GetOutput());
  tfilter7->SetTransform(trans7);
  tfilter7->Update();

  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(tfilter1->GetOutput());
  apd->AddInput(tfilter2->GetOutput());
  apd->AddInput(tfilter3->GetOutput());
  apd->AddInput(tfilter4->GetOutput());
  apd->AddInput(tfilter5->GetOutput());
  apd->AddInput(tfilter6->GetOutput());
  apd->AddInput(tfilter7->GetOutput());
  apd->Update();
  
  zframeModel->SetAndObservePolyData(apd->GetOutput());

  double color[3];
  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.0;
  zframeDisp->SetPolyData(zframeModel->GetPolyData());
  zframeDisp->SetColor(color);

  rstr = zframeModel->GetID();
  
  trans1->Delete();
  trans2->Delete();
  trans3->Delete();
  trans4->Delete();
  trans5->Delete();
  trans6->Delete();
  trans7->Delete();
  tfilter1->Delete();
  tfilter2->Delete();
  tfilter3->Delete();
  tfilter4->Delete();
  tfilter5->Delete();
  tfilter6->Delete();
  tfilter7->Delete();
  cylinder1->Delete();
  cylinder2->Delete();
  cylinder3->Delete();
  cylinder4->Delete();
  cylinder5->Delete();
  cylinder6->Delete();
  cylinder7->Delete();

  apd->Delete();

  zframeDisp->Delete();
  zframeModel->Delete();

  return rstr.c_str();

}


//----------------------------------------------------------------------------
const char* vtkProstateNavCalibrationStep::AddZFrameTransform(const char* nodeName)
{
  std::string rstr;

  vtkMRMLLinearTransformNode* transformNode;

  transformNode = vtkMRMLLinearTransformNode::New();
  transformNode->SetName(nodeName);
  transformNode->SetDescription("Created by ProstateNav module");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  transformNode->ApplyTransform(transform);
  transform->Delete();

  this->MRMLScene->AddNode(transformNode);  

  rstr = transformNode->GetID();
  transformNode->Delete();
  
  return rstr.c_str();

}

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::PerformZFrameCalibration(const char* filename)
{
  std::cerr << "Loading " << filename << std::endl;

  vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes"); 
  if ( m != NULL ) 
    {
    vtkSlicerVolumesLogic* volume_logic = 
      vtkSlicerVolumesGUI::SafeDownCast(m)->GetLogic();
    volume_logic->AddArchetypeVolume(filename, "ZFrameImage", 0x0004);

    vtkMRMLScalarVolumeNode* volumeNode = NULL;   // Event Source MRML node 
    vtkCollection* collection = this->MRMLScene->GetNodesByName("ZFrameImage");
    int nItems = collection->GetNumberOfItems();
    for (int i = 0; i < nItems; i ++)
      {
      vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
      if (strcmp(node->GetNodeTagName(), "Volume") == 0)
        {
        volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
        break;
        }
      }
    if (volumeNode)
      {
      Init(256, 256);
      //vtkMRMLNode* node = this->MRMLScene->GetNodeByID(this->ZFrameTransformNodeID);
      vtkMRMLNode* node = this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode;
      if (node != NULL)
        {
        transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
        ZFrameRegistration(volumeNode, transformNode, 0);
        transformNode->Modified();
        this->GetLogic()->SendZFrame();
        }
      else
        {
        std::cerr << "Couldn't find zframe transform node" << std::endl;
        }
      }
    else
      {
      std::cerr << "Couldn't find ZFrame image in the MRML scene." << std::endl;
      }
    }
}


//----------------------------------------------------------------------------
// Z-frame registration algorithm
//----------------------------------------------------------------------------

#define  RT_CONTROL 1
#define  Z_TRANSFORM_OUT 0
#define  IMAGE_OUT 0
#ifndef M_PI
#define M_PI 3.14159
#endif

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::Init(int xsize, int ysize)
{
  int i,j,m,n;

  // At present, the image dimensions are fixed to 256x256.
  // In the future, this should be flexible.
  // FORCE xsize and ysize for now.
  imgxsize = xsize = 256;
  imgysize = ysize = 256;

  // Define an 11x11 correlation kernel for fiducial detection.
  Real kernel[11][11]={{0,0,0.0,0,0.0,0,0.0,0.0,0.0,0,0},
                       {0,0,0.0,0,0.0,0,0.0,0.0,0.0,0,0},
                       {0,0,0.0,0,0.5,0.5,0.5,0.0,0,0,0},
                       {0,0,0.0,1,1.0,1.0,1.0,1,0.0,0,0},
                       {0,0,0.5,1,1.0,1.0,1.0,1,0.5,0,0},
                       {0,0,0.5,1,1.0,1.0,1.0,1,0.5,0,0},
                       {0,0,0.5,1,1.0,1.0,1.0,1,0.5,0,0},
                       {0,0,0.0,1,1.0,1.0,1.0,1,0.0,0,0},
                       {0,0,0.0,0,0.5,0.5,0.5,0.0,0,0,0},
                       {0,0,0.0,0,0.0,0,0.0,0.0,0.0,0,0},
                       {0,0,0.0,0,0.0,0,0.0,0.0,0.0,0,0}};

  // Create a mask image and initialize elements to zero.
  // The Matrix class is implemented in the newmat library:
  // see: http://www.robertnz.net/
  /*
  this->MaskImage = vtkImageData::New();
  this->MaskImage->SetDimensions(xsize, ysize, 1);
  this->MaskImage->SetExtent(0, xsize-1, 0, ysize-1, 0, 0 );
  this->MaskImage->SetSpacing(1.0, 1.0, 1.0);
  this->MaskImage->SetOrigin(0.0, 0.0, 0.0);
  this->MaskImage->SetNumberOfScalarComponents(1);
  this->MaskImage->SetScalarTypeToFloat();
  this->MaskImage->AllocateScalars();
  float* dest = (float*) this->MaskImage->GetScalarPointer();
  if (dest)
    {
    float* end = dest + xsize*ysize*1;
    float* p = dest;
    while (p < end)
      {
      *p = 0.0;
      p++;
      }
    }
  */
  MaskImage.ReSize(xsize,ysize);
  for(i=0; i<xsize; i++)
    for(j=0; j<ysize; j++)
        MaskImage.element(i,j) = 0;

  // Copy the correlation kernel to the centre of the mask image.
  for(i=((xsize/2)-5),m=0; i<=((xsize/2)+5); i++,m++)
    for(j=((ysize/2)-5),n=0; j<=((ysize/2)+5); j++,n++)
    {
        MaskImage.element(i,j) = kernel[m][n];
    }

  // Correlation will be computed using spatial convolution, and hence 
  // multiplication in the frequency domain. This dramatically accelerates 
  // fiducial detection. Transform mask to frequency domain; this only
  // has to be done once.
  // Before transforming the mask to the spatial frequency domain, need to
  // create an empty imaginary component, since the mask is real-valued.
  zeroimag.ReSize(xsize,ysize);
  for(i=0; i<xsize; i++)
    for(j=0; j<ysize; j++)
        zeroimag.element(i,j) = 0;

  // The Radix-2 FFT algorithm is implemented in the newmat library:
  // see: http://www.robertnz.net/
  FFT2(MaskImage, zeroimag, MFreal, MFimag);

  // Conjugate and normalize the mask.
  MFimag *= -1;
  Real maxabsolute = ComplexMax(MFreal, MFimag);
  MFreal *= (1/maxabsolute);
  MFimag *= (1/maxabsolute);

  // MFreal and MFimag now contain the real and imaginary matrix elements for
  // the frequency domain representation of the correlation mask.
}

/*----------------------------------------------------------------------------*/

/**
 * Method called by the SPL-OpenTracker child node when an event occurs.
 * @param event The event value passed.
 * @param generator The node that generated the event.
 */
int vtkProstateNavCalibrationStep::ZFrameRegistration(vtkMRMLScalarVolumeNode* volumeNode,
                                                      vtkMRMLLinearTransformNode* transformNode,
                                                      int slindex)
{
  int           xsize;
  int           ysize;
  int           zsize;
  //Image         image_attrib;
  int           i,j;
  int           Zcoordinates[7][2];
  float         tZcoordinates[7][2];
  bool          frame_lock;
  float         pixel_size=FORCE_FOV/FORCE_SIZEX;
  float         tmpCoord;
  Column3Vector Zposition;
  Quaternion    Zorientation;

//simond debug frame delay
  static int flip_counter=0;
  static Column3Vector Iposition;
  static Quaternion Iorientation;
  
  // Get the image size attributes from the event.
  /*
  xsize=event.getAttribute(string("xsize"),0);
  ysize=event.getAttribute(string("ysize"),0);
  */
  int dimensions[3];
  vtkImageData* image = volumeNode->GetImageData();
  image->GetDimensions(dimensions);
  xsize = dimensions[0];
  ysize = dimensions[1];
  zsize = dimensions[2];
  
  // Get image position and orientation
  vtkMatrix4x4* rtimgTransform = vtkMatrix4x4::New(); 
  igtl::Matrix4x4 matrix;
  volumeNode->GetIJKToRASMatrix(rtimgTransform);

  float tx = rtimgTransform->GetElement(0, 0);
  float ty = rtimgTransform->GetElement(1, 0);
  float tz = rtimgTransform->GetElement(2, 0);
  float sx = rtimgTransform->GetElement(0, 1);
  float sy = rtimgTransform->GetElement(1, 1);
  float sz = rtimgTransform->GetElement(2, 1);
  float nx = rtimgTransform->GetElement(0, 2);
  float ny = rtimgTransform->GetElement(1, 2);
  float nz = rtimgTransform->GetElement(2, 2);
  float px = rtimgTransform->GetElement(0, 3);
  float py = rtimgTransform->GetElement(1, 3);
  float pz = rtimgTransform->GetElement(2, 3);

  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  // Shift the center
  // NOTE: The center of the image should be shifted due to different
  // definitions of image origin between VTK (Slicer) and OpenIGTLink;
  // OpenIGTLink image has its origin at the center, while VTK image
  // has one at the corner.

  float hfovi = psi * (dimensions[0]-1) / 2.0;
  float hfovj = psj * (dimensions[1]-1) / 2.0;
  float hfovk = psk * (dimensions[2]-1) / 2.0;

  float cx = ntx * hfovi + nsx * hfovj + nnx * hfovk;
  float cy = nty * hfovi + nsy * hfovj + nny * hfovk;
  float cz = ntz * hfovi + nsz * hfovj + nnz * hfovk;

  matrix[0][0] = ntx;
  matrix[1][0] = nty;
  matrix[2][0] = ntz;
  matrix[0][1] = nsx;
  matrix[1][1] = nsy;
  matrix[2][1] = nsz;
  matrix[0][2] = nnx;
  matrix[1][2] = nny;
  matrix[2][2] = nnz;
  matrix[0][3] = px + cx;
  matrix[1][3] = py + cy;
  matrix[2][3] = pz + cz;

  float position[3];
  float quaternion[4];
  igtl::MatrixToQuaternion(matrix, quaternion);
  position[0] = matrix[0][3];
  position[1] = matrix[1][3];
  position[2] = matrix[2][3];

  std::cerr << "=== Image position ===" << std::endl;
  std::cerr << "x = " << position[0] << std::endl; 
  std::cerr << "y = " << position[1] << std::endl;
  std::cerr << "z = " << position[2] << std::endl;

  // Get current position and orientation of the imaging plane.
  // SPL OpenTracker events always contain Position and Orientation attributes.
  Iposition.setX( position[0] );
  Iposition.setY( position[1] );
  Iposition.setZ( position[2] );
  Iorientation.setX( quaternion[0] );
  Iorientation.setY( quaternion[1] );
  Iorientation.setZ( quaternion[2] );
  Iorientation.setW( quaternion[3] );

  //image_attrib=event.getAttribute((Image*)NULL,"image");
 
  // Get a pointer to the image array.
  image->SetScalarTypeToShort();
  image->Modified();
  InputImage=(short*)image->GetScalarPointer();
  if (slindex >= 0 && slindex < zsize)
    {
    InputImage = &InputImage[xsize*ysize*slindex];
    }
  else
    {
    return 0;
    }

  // Transfer image to a Matrix.
  SourceImage.ReSize(xsize,ysize);
  for(i=0; i<xsize; i++)
    for(j=0; j<ysize; j++)
      SourceImage.element(i,j) = InputImage[i*ysize+j];

  // Find the 7 Z-frame fiducial intercept artifacts in the image.
  std::cerr << "ZTrackerTransform - Searching fiducials...\n" << std::endl;
  if(LocateFiducials(SourceImage, xsize, ysize, Zcoordinates, tZcoordinates) == false)
  {
  std::cerr << "ZTrackerTransform::onEventGenerated - Ficudials not detected. No frame lock on this image.\n" << std::endl;
     frame_lock = false;
  }
  else frame_lock = true;

  // Check that the fiducial geometry makes sense.
  std::cerr << "ZTrackerTransform - Checking the fiducial geometries...\n" << std::endl;
  if(CheckFiducialGeometry(Zcoordinates, xsize, ysize) == true)
    frame_lock = true;
  else 
    {
       frame_lock = false;
       std::cerr << "ZTrackerTransform::onEventGenerated - Bad fiducial geometry. No frame lock on this image." << std::endl;
    }

  // Compute the pose of the Z-frame only if we have a lock on the fiducial points.
  if(frame_lock)
  {
  std::cerr << "ZTrackerTransform::onEventGenerated - frame lock." << std::endl;
    
    // Transform pixel coordinates into spatial coordinates.
    // 1) Put the image origin at the centre of the image,
    // 2) Re-align axes according to the IJK convention,
    // 3) Scale by pixel size.
    for(int i=0; i<7; i++)
    {
      // Move origin to centre of image and exchange axes for correct 
      // image frame orientation.
      tmpCoord = tZcoordinates[i][1];
      tZcoordinates[i][1] = (float)(tZcoordinates[i][0]) - (float)(xsize/2);
      tZcoordinates[i][0] = tmpCoord - (float)(ysize/2);

      // Flip the y-axis for IJK coordinates. 
      //tZcoordinates[i][1] *= -1.0;

      // Scale coordinates by pixel size
      tZcoordinates[i][0] *= pixel_size;
      tZcoordinates[i][1] *= pixel_size;
    }

    // Compute relative pose between the Z-frame and the current image.
    if(LocalizeFrame(tZcoordinates, Zposition, Zorientation) == false)
    {
       frame_lock = 0;
       std::cerr << "ZTrackerTransform::onEventGenerated - Could not localize the frame. Skipping this one." << std::endl;
    }
  }

  if(frame_lock)
  {
    // Compute the new image position and orientation that will be centred to
    // the Z-frame.
    Update_Scan_Plane(Iposition, Iorientation, Zposition, Zorientation);
  }
  
  // Construct a new event to pass on to the child node.

#ifdef DEBUG_ZFRAME
  //simond Debug Image Output
  short output_image[FORCE_SIZEX*FORCE_SIZEY];
  if(frame_lock)
  {
    static int counter=7;
    for(i=0; i<counter; i++)
    {
      SourceImage.element((int)(Zcoordinates[i][0]),(int)(Zcoordinates[i][1])) = 0.0;
    }

    for(i=0; i<FORCE_SIZEX; i++)
      for(j=0; j<FORCE_SIZEY; j++)
      {
        output_image[i*FORCE_SIZEX+j] = (short)(SourceImage.element(i,j)*1);
      }
    //if(++counter >7) counter=1;
  }
#endif
//end simond

  position[0] = Iposition.getX();
  position[1] = Iposition.getY();
  position[2] = Iposition.getZ();
  quaternion[0] = Iorientation.getX();
  quaternion[1] = Iorientation.getY();
  quaternion[2] = Iorientation.getZ();
  quaternion[3] = Iorientation.getW();

  igtl::QuaternionToMatrix(quaternion, matrix);
  matrix[0][3] = position[0];
  matrix[1][3] = position[1];
  matrix[2][3] = position[2];

  std::cerr << "Result matrix:" << std::endl;
  igtl::PrintMatrix(matrix);

  vtkMatrix4x4* zMatrix = vtkMatrix4x4::New();
  zMatrix->Identity();
  zMatrix->SetElement(0, 0, -matrix[0][0]);
  zMatrix->SetElement(1, 0, -matrix[1][0]);
  zMatrix->SetElement(2, 0, -matrix[2][0]);
  zMatrix->SetElement(0, 1, matrix[0][1]);
  zMatrix->SetElement(1, 1, matrix[1][1]);
  zMatrix->SetElement(2, 1, matrix[2][1]);
  zMatrix->SetElement(0, 2, -matrix[0][2]);
  zMatrix->SetElement(1, 2, -matrix[1][2]);
  zMatrix->SetElement(2, 2, -matrix[2][2]);
  zMatrix->SetElement(0, 3, matrix[0][3]);
  zMatrix->SetElement(1, 3, matrix[1][3]);
  zMatrix->SetElement(2, 3, matrix[2][3]);

  if (transformNode != NULL)
    {
    vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();
    transformToParent->DeepCopy(zMatrix);
    zMatrix->Delete();
    return 1;
    }
  else
    {
    zMatrix->Delete();
    return 0;
    }

  

}

/*----------------------------------------------------------------------------*/

/**
 * The Z-frame contains seven line fiducials arranged in such a manner that
 * its position and orientation in the MRI scanner can be determined from a
 * single image. This method detects the seven line fiducial intercepts.
 * @param SourceImage An image matrix containing the latest image.
 * @param xsize The width of the image in pixels.
 * @param ysize The height of the image in pixels.
 * @param Zcoordinates[][] The resulting list of seven fiducial coordinates.

*/
bool vtkProstateNavCalibrationStep::LocateFiducials(Matrix &SourceImage, int xsize, 
                  int ysize, int Zcoordinates[7][2], float tZcoordinates[7][2])
{
  int    i,j;
  Real   peakval, offpeak1, offpeak2, offpeak3, offpeak4;

  // Transform the MR image to the frequency domain (k-space).
  FFT2(SourceImage, zeroimag, IFreal, IFimag);

  // Normalize the image.
  Real maxabsolute = ComplexMax(IFreal,IFimag);
  // RISK: maxabsolute may be close to zero.
  if(maxabsolute<MEPSILON)
  {
  std::cerr << "ZTrackerTransform::LocateFiducials - divide by zero." << std::endl;
  } else
    {
      IFreal *= (1/maxabsolute);
      IFimag *= (1/maxabsolute);
    }

  // Pointwise multiply the Image and the Mask in k-space.
  PFreal.ReSize(xsize,ysize);
  PFimag.ReSize(xsize,ysize);
  for(i=0; i<xsize; i++)
    for(j=0; j<ysize; j++)
  {
    PFreal.element(i,j) = IFreal.element(i,j)*MFreal.element(i,j) -
                          IFimag.element(i,j)*MFimag.element(i,j);
    PFimag.element(i,j) = IFreal.element(i,j)*MFimag.element(i,j) +
                          IFimag.element(i,j)*MFreal.element(i,j);   
  }

  // Invert the product of the two k-space images back to spatial domain.
  // Regions of high correlation between the mask the image will appear
  // as sharp peaks in the inverted image.
  PIreal.ReSize(xsize,ysize);
  PIimag.ReSize(xsize,ysize);
  FFT2I(PFreal, PFimag, PIreal, PIimag);

  // FFTSHIFT: exchange diagonally-opposite image quadrants.
  Real swaptemp;
  for(i=0; i<(xsize/2); i++)
    for(j=0; j<ysize/2; j++)
    {
      // Exchange first and fourth quadrants.
      swaptemp = PIreal.element(i,j);
      PIreal.element(i,j) = PIreal.element(i+xsize/2,j+ysize/2);
      PIreal.element(i+xsize/2,j+ysize/2) = swaptemp; 

      // Exchange second and third quadrants.
      swaptemp = PIreal.element(i+xsize/2,j);
      PIreal.element(i+xsize/2,j) = PIreal.element(i,j+ysize/2);
      PIreal.element(i,j+ysize/2) = swaptemp; 
  }

  // Normalize result.
  maxabsolute = RealMax(PIreal);
  // RISK: maxabsolute may be close to zero.
  if(maxabsolute<MEPSILON)
  {
  std::cerr << "ZTrackerTransform::LocateFiducials - divide by zero." << std::endl;
  return(false);
  } else
    {
      PIreal *= (1/maxabsolute);
    }

  // Find the top 7 peak image values.
  int rstart, rstop, cstart, cstop;
  int peakcount=0;
  for(i=0; i<7; i++)
  {
    // Find the next peak value.
    peakval = FindMax(PIreal, Zcoordinates[i][0], Zcoordinates[i][1]); 

    // Define a block neighbourhood around the peak value.
    rstart = Zcoordinates[i][0]-10;
    if(rstart<0) 
    {
      rstart=0;
    }
    rstop = Zcoordinates[i][0]+10;
    if(rstop>=xsize) 
    {
      rstop=xsize-1;
    }
    cstart = Zcoordinates[i][1]-10;
    if(cstart<0) 
    {
      cstart=0;
    }
    cstop = Zcoordinates[i][1]+10;
    if(cstop>=ysize) 
    {
      cstop = ysize-1;
    }

    // Check that this is a local maximum.
    if(peakval<MEPSILON)
    {
    std::cerr << "vtkProstateNavCalibrationStep::OrderFidPoints - peak value is zero." << std::endl;
      return(false);
    }
    else
      {
        offpeak1 = (peakval - PIreal.element(rstart,cstart))/peakval;
        offpeak2 = (peakval - PIreal.element(rstart,cstop))/peakval;
        offpeak3 = (peakval - PIreal.element(rstop,cstart))/peakval;
        offpeak4 = (peakval - PIreal.element(rstop,cstop))/peakval;
        if(offpeak1<0.3 || offpeak2<0.3 || offpeak3<0.3 || offpeak4<0.3)
        {
           // Ignore coordinate if the offpeak value is within 30% of the peak.
           i--;
           std::cerr << "vtkProstateNavCalibrationStep::LocateFiducials - Bad Peak." << std::endl;
           if(++peakcount > 10) return(false);
        }
      }

    // Find the subpixel coordinates of the peak.
    FindSubPixelPeak(&(Zcoordinates[i][0]), &(tZcoordinates[i][0]), 
                     PIreal.element(Zcoordinates[i][0],Zcoordinates[i][1]),
                     PIreal.element(Zcoordinates[i][0]-1,Zcoordinates[i][1]),
                     PIreal.element(Zcoordinates[i][0]+1,Zcoordinates[i][1]),
                     PIreal.element(Zcoordinates[i][0],Zcoordinates[i][1]-1),
                     PIreal.element(Zcoordinates[i][0],Zcoordinates[i][1]+1));
    
    // Eliminate this peak and search for the next.
    for(int m=rstart; m<=rstop; m++)
      for(int n=cstart; n<=cstop; n++)
        PIreal.element(m,n) = 0.0;
  }

  //=== Determine the correct ordering of the detected fiducial points ===
  // Find the centre of the pattern
  float pmid[2];
  FindFidCentre(tZcoordinates, pmid[0], pmid[1]);

  // Find the corner points
  FindFidCorners(tZcoordinates, pmid);

  // Sequentially order all points
  OrderFidPoints(tZcoordinates, pmid[0], pmid[1]);

  // Update Zcoordinates
  for(i=0; i<7; i++)
  {
    Zcoordinates[i][0] = (int)(tZcoordinates[i][0]);
    Zcoordinates[i][1] = (int)(tZcoordinates[i][1]);
  }

  return(true);
}

/*----------------------------------------------------------------------------*/

/**
 * Find the subpixel coordinates of the peak. This implementation approximates
 * the location of the peak by fitting a parabola in each axis. This should be
 * revised to fit a paraboloid in 3D.
 * @param Zcoordinate[] A fiducial coordinate.
 * @param tZcoordinate[] A fiducial coordinate computed to sub-pixel accuracy. 
 */

void vtkProstateNavCalibrationStep::FindSubPixelPeak(int Zcoordinate[2], 
                                         float tZcoordinate[2],
                                         Real Y0, Real Yx1, Real Yx2, Real Yy1, Real Yy2)
{
  float Xshift, Yshift;

  Xshift = (0.5*((float)Yx1 - (float)Yx2)) / 
           ((float)Yx1 + (float)Yx2 - 2.0*(float)Y0);

  Yshift = (0.5*((float)Yy1 - (float)Yy2)) / 
           ((float)Yy1 + (float)Yy2 - 2.0*(float)Y0);

  if(fabs(Xshift)>1.0 || fabs(Yshift)>1.0)
  {
  std::cerr << "vtkProstateNavCalibrationStep::FindSubPixelPeak - subpixel peak out of range." << std::endl;
    tZcoordinate[0] = (float)(Zcoordinate[0]);
    tZcoordinate[1] = (float)(Zcoordinate[1]);
  }
  else
    {   
      tZcoordinate[0] = (float)(Zcoordinate[0]) + Xshift;
      tZcoordinate[1] = (float)(Zcoordinate[1]) + Yshift;
    }
}

/*----------------------------------------------------------------------------*/

/**
 * Check the geometry of the fiducial pattern to be sure that it is valid.
 * @param Zcoordinates[][] The list of seven fiducial coordinates.
 * @param xsize The width of the image in pixels.
 * @param ysize The height of the image in pixels.
 * @return true if the point geometry is ok, else false.
 */
bool vtkProstateNavCalibrationStep::CheckFiducialGeometry(int Zcoordinates[7][2], 
                                              int xsize, int ysize)
{
  Column2Vector  P1, P3, P5, P7;
  Column2Vector  D71, D53, D13, D75;
  int            i;
  float          dotp;

  // First check that the coordinates are in range.
  for(i=0; i<7; i++)
  {
    if(Zcoordinates[i][0]<0 || Zcoordinates[i][0]>=ysize ||
       Zcoordinates[i][1]<0 || Zcoordinates[i][1]>=xsize)
    {
    std::cerr << "vtkProstateNavCalibrationStep::onEventGenerated - fiducial coordinates out of range. No frame lock on this image." << std::endl;
      return(false);
    }
  }

  // Check that corner points form a parallelogram.
  P1.setvalues(Zcoordinates[0][0], Zcoordinates[0][1]);
  P3.setvalues(Zcoordinates[2][0], Zcoordinates[2][1]);
  P5.setvalues(Zcoordinates[4][0], Zcoordinates[4][1]);
  P7.setvalues(Zcoordinates[6][0], Zcoordinates[6][1]);
  D71 = P7 - P1;
  D53 = P5 - P3;
  D13 = P1 - P3;
  D75 = P7 - P5;
  D71.normalize();
  D53.normalize();
  D13.normalize();
  D75.normalize();
  // Check that opposite edges are within 10 degrees of parallel. 
  dotp = D71.getX()*D53.getX() + D71.getY()*D53.getY();
  if(dotp<0) dotp *= -1.0;
  if(dotp < cos(5.0*M_PI/180.0)) return(false);
  dotp = D13.getX()*D75.getX() + D13.getY()*D75.getY();
  if(dotp<0) dotp *= -1.0;
  if(dotp < cos(5.0*M_PI/180.0)) return(false);
   
  return(true);
}


/*----------------------------------------------------------------------------*/

/**
 * Find the centre of the fiducial pattern. A cross-sectional image will
 * intercept each of the Z-frame's seven line fiducials. Once these seven
 * intercepts are detected, this method computes the centre of the region
 * bounded by these points.
 * @param points[][] Image coordinates of the seven fiducial points.
 * @param rmid Row coordinate at centre.
 * @param cmid Column coordinate at centre.
 */
void vtkProstateNavCalibrationStep::FindFidCentre(float points[7][2], float &rmid, float &cmid)
{
  int    i, j;
  float  minrow=0.0, maxrow=0.0, mincol=0.0, maxcol=0.0;

  // Find the bounding rectangle.
  for(i=0; i<7; i++)
  {
    // find minimum row coordinate
    if(points[i][0]<minrow || i==0)
      minrow = points[i][0];

    // find maximum row coordinate
    if(points[i][0]>maxrow || i==0)
      maxrow = points[i][0];

    // find minimum column coordinate
    if(points[i][1]<mincol || i==0)
      mincol = points[i][1];

    // find maximum column coordinate
    if(points[i][1]>maxcol || i==0)
      maxcol = points[i][1];
  }

  // Centre of bounding rectangle.
  rmid = (minrow + maxrow)/2.0;
  cmid = (mincol + maxcol)/2.0;
} 

/*----------------------------------------------------------------------------*/

/**
 * This method identifies the four corner fiducials, based on known geometry 
 * of the Z-frame.
 * @param points[][] The fiducial coordinates detected in the previous step. 
 * @param pmid The centre of the rectangular region bounded by the fiducial 
 *             points.
 */
void vtkProstateNavCalibrationStep::FindFidCorners(float points[7][2], float *pmid)
{
  int    i;
  float  itemp[2];
  float  distances[7], dtemp;
  bool   swapped;

  // Compute distances between each fiducial and the midpoint.
  for(i=0; i<7; i++)
  {
    distances[i] = CoordDistance(pmid, &(points[i][0]));
  }

  // Sort distances in descending order. The four corner points will be 
  // furthest away from the centre and will be sorted to the top of the
  // coordinate list.
  swapped = true;
  // Loop until there are no more exchanges (Bubble Sort)
  while(swapped)
  {
    swapped = false;
    for(i=0; i<6; i++)
        if(distances[i]<distances[i+1])
        {
          // Swap distances.
          dtemp = distances[i];
          distances[i] = distances[i+1];
          distances[i+1] = dtemp;
  
          // Swap corresponding coordinates in the fiducial list.
          itemp[0] = points[i][0];
          itemp[1] = points[i][1];
          points[i][0] = points[i+1][0];
          points[i][1] = points[i+1][1];
          points[i+1][0] = itemp[0];
          points[i+1][1] = itemp[1];
  
          swapped = true;
        }
  }
  
  // Choose the order of the corners, based on their separation distance.
  // First find the closest point to first corner in the list. 
  // This must be an adjacent corner.
  float pdist1 = CoordDistance(&(points[0][0]), &(points[1][0]));
  float pdist2 = CoordDistance(&(points[0][0]), &(points[2][0]));
  if(pdist1>pdist2)
  {
    itemp[0] = points[1][0];
    itemp[1] = points[1][1];
    points[1][0] = points[2][0];
    points[1][1] = points[2][1];
    points[2][0] = itemp[0];
    points[2][1] = itemp[1];
  }
  // Now find closest point (of third or fourth) to second corner in list.
  // This will become the third corner in the sequence.
  pdist1 = CoordDistance(&(points[1][0]), &(points[2][0]));
  pdist2 = CoordDistance(&(points[1][0]), &(points[3][0]));
  if(pdist1>pdist2)
  {
    itemp[0] = points[2][0];
    itemp[1] = points[2][1];
    points[2][0] = points[3][0];
    points[2][1] = points[3][1];
    points[3][0] = itemp[0];
    points[3][1] = itemp[1];
  }
}

/*----------------------------------------------------------------------------*/

/**
 * Find the distance between two image points.
 * TO DO: this should be moved to ZLinAlg.
 * @param p1 First image point coordinates.
 * @param p2 Second image point coordinates.
 * @return Distance between points.
 */
float vtkProstateNavCalibrationStep::CoordDistance(float *p1, float *p2)
{
  float sqdist;
  
  sqdist = (p1[0]-p2[0])*(p1[0]-p2[0]) + (p1[1]-p2[1])*(p1[1]-p2[1]);
  
  // RISK: Argument for SQRT may be negative. Overflow?
  if(sqdist<0)
  {
  std::cerr << "vtkProstateNavCalibrationStep::CoordDistance - \
                              negative SQRT argument.\n" << std::endl;
      return(0);
  } else
    {
      return(sqrt((float)(sqdist)));
    }
}

/*----------------------------------------------------------------------------*/

/**
 * Put the fiducial coordinate point list in sequential order by matching the
 * three remaining points to their neighbouring ordered corner points.
 * TO DO: messy, need to improve readability.
 * @param points[][] The fiducial coordinate point list.
 * @param rmid The centre of the fiducial pattern in the row coordinate.
 * @param cmid The centre of the fiducial pattern in the column coordinate.
 */
void vtkProstateNavCalibrationStep::OrderFidPoints(float points[7][2], float rmid, float cmid)
{
  int    pall[9]={0,-1,1,-1,2,-1,3,-1,0};  // prototype index list for all points
  int    pall2[7];
  int    pother[3]={4,5,6};                // indices of points other than corners
  int    i,j;
  float  points_temp[7][2];
  float  cdist, pdist1, pdist2;
  
  // Find fiducial points that fit between the corners.
  for(i=0; i<7; i+=2)
    for(j=0; j<3; j++)
    {
      if(pother[j]==-1)
      {
        // This point has already been placed. 
        continue;
      }
      cdist = CoordDistance(&(points[pall[i]][0]), &(points[pall[i+2]][0]));
      pdist1 = CoordDistance(&(points[pall[i]][0]), &(points[pother[j]][0]));
      pdist2 = CoordDistance(&(points[pall[i+2]][0]), &(points[pother[j]][0]));

      // RISK: divide by zero.
      if(cdist<MEPSILON)
      {
      std::cerr <<  "vtkProstateNavCalibrationStep::OrderFidPoints - \
                                divide by zero." << std::endl;
        // TO DO: this should be detected in the first sanity check.
      } else
        {
          if(((pdist1+pdist2)/cdist)<1.05)
          {
            pall[i+1] = pother[j]; 
            pother[j] = -1;
            break;
          }
        }
    }
  
  // Re-order the points.
  // Find the -1. The last remaining -1 index marks the two corner points that 
  // do not have a fiducial lying between them. By convention, we start ordering
  // the coordinate points from one of these corners.
  for(i=1; i<9; i++)
      if(pall[i]==-1) break;
  
  // Find the direction to order the points 
  // (traverse points clockwise in the image).
  float d1x=(points[pall[0]][0]-rmid);
  float d1y=(points[pall[0]][1]-cmid);
  float d2x=(points[pall[2]][0]-rmid);
  float d2y=(points[pall[2]][1]-cmid); 
  float nvecz=(d1x*d2y-d2x*d1y);  
  int direction = 0;
  if(nvecz<0)
    direction = -1;
  else
    direction = 1;

  // Do the re-ordering in the clockwise direction.
  for(j=0; j<7; j++)
  {
      i += direction;
      if(i==-1) i=7;
      if(i==9) i=1;
      pall2[j] = pall[i];
  }

  // Create the new ordered point list.
  for(i=0; i<7; i++)
  {
      points_temp[i][0] = points[pall2[i]][0];
      points_temp[i][1] = points[pall2[i]][1];
  }
  for(i=0; i<7; i++)
  {
      points[i][0] = points_temp[i][0];
      points[i][1] = points_temp[i][1];
  }
}

/*----------------------------------------------------------------------------*/

/**
 * Compute the pose of the fiducial frame relative to the image plane, using
 * an adaptation of an algorithm presented by Susil et al.: "A Single image 
 * Registration Method for CT-Guided Interentions", MICCAI 1999.
 * @param Zcoordinates[][] Ordered Z-frame coordinates.
 * @param Zposition Estimated position of Z-frame origin w.r.t. image origin.
 * @param Zorientation Estimated orientation of the Z-frame w.r.t. the image
 *        frame--expressed as a quaternion.
 */
bool vtkProstateNavCalibrationStep::LocalizeFrame(float Zcoordinates[7][2],
                                      Column3Vector &Zposition, 
                                      Quaternion &Zorientation)
{
  Column3Vector       Pz1, Pz2, Pz3;
  Column3Vector       Oz;
  Column3Vector       P2f, P4f, P6f;
  Column3Vector       Cf, Ci, Cfi;
  Column3Vector       Vx, Vy, Vz;
  Quaternion          Qft, Qit;
  float               angle;
  Column3Vector       axis;
  std::ostringstream  outs;
  
  
  //--- Compute diagonal points in the z-frame coordinates -------
  // Frame origin is at lower corner of Side 1,
  // y-axis is vertical, x-axis is horizontal.
  
  //--- SIDE 1
  // Map the three points for this z-fiducial.
  Pz1.setvalues( Zcoordinates[0][0], Zcoordinates[0][1], 0.0 );
  Pz2.setvalues( Zcoordinates[1][0], Zcoordinates[1][1], 0.0 );
  Pz3.setvalues( Zcoordinates[2][0], Zcoordinates[2][1], 0.0 );
  
  // Origin and direction vector of diagonal fiducial.
  Oz.setvalues( -30.0, -30.0, 30.0 );
  Vz.setvalues( 0.0, 1.0, -1.0 );
  
  // Solve for the diagonal intercept in Z-frame coordinates.
  SolveZ(Pz1, Pz2, Pz3, Oz, Vz, P2f);
  
  //--- BASE
  // Map the three points for this z-fiducial.
  Pz1.setvalues( Zcoordinates[2][0], Zcoordinates[2][1], 0.0 );
  Pz2.setvalues( Zcoordinates[3][0], Zcoordinates[3][1], 0.0 );
  Pz3.setvalues( Zcoordinates[4][0], Zcoordinates[4][1], 0.0 );
  
  // Origin and direction vector of diagonal fiducial.
  Oz.setvalues( 30.0, -30.0, 30.0 );
  Vz.setvalues( -1.0, 0.0, -1.0 );
  
  // Solve for the diagonal intercept in Z-frame coordinates.
  SolveZ(Pz1, Pz2, Pz3, Oz, Vz, P4f);
  
  //--- SIDE 2
  // Map the three points for this z-fiducial.
  Pz1.setvalues( Zcoordinates[4][0], Zcoordinates[4][1], 0.0 );
  Pz2.setvalues( Zcoordinates[5][0], Zcoordinates[5][1], 0.0 );
  Pz3.setvalues( Zcoordinates[6][0], Zcoordinates[6][1], 0.0 );
  
  // Origin and direction vector of diagonal fiducial.
  Oz.setvalues( 30.0, 30.0, 30.0 );
  Vz.setvalues( 0.0, -1.0, -1.0 );
  
  // Solve for the diagonal intercept in Z-frame coordinates.
  SolveZ(Pz1, Pz2, Pz3, Oz, Vz, P6f);
  
  
  //--- Compute Transformation Between Image and Frame -----------
  
  // Compute orientation component first.
  // Compute z-frame cross section coordinate frame 
  Vx = P4f - P2f;
  Vy = P6f - P2f;
  Vz = Vx * Vy;
  Vy = Vz * Vx;
  Vx.normalize();
  if(Qft.ComputeFromRotationMatrix(Vx, Vy, Vz) == false)
    return(false);
  
  // Compute image cross-section coordinate frame 
  Pz1.setvalues( Zcoordinates[1][0], Zcoordinates[1][1], 0.0 );
  Pz2.setvalues( Zcoordinates[3][0], Zcoordinates[3][1], 0.0);
  Pz3.setvalues( Zcoordinates[5][0], Zcoordinates[5][1], 0.0);
  
  Vx = Pz2 - Pz1;
  Vy = Pz3 - Pz1;
  Vz = Vx * Vy;
  Vy = Vz * Vx;
  Vx.normalize();
  if(Qit.ComputeFromRotationMatrix(Vx, Vy, Vz) == false)
    return(false);
  
  // Rotation between frame and image: Qif = Qit/Qft
  Zorientation = Qit/Qft;

  // Compute axis-angle.
  angle = 2*acos(Zorientation.getW());
  if(fabs(angle)>15.0)
  {
  std::cerr << "vtkProstateNavCalibrationStep::LocalizeFrame - Rotation angle too large, something is wrong." << std::endl;
     return(false);
  }
  if(angle==0.0)
  {
    axis.setX(1.0);
    axis.setY(0.0);
    axis.setZ(0.0); 
  }
  else axis.setvalues(Zorientation.getX()/
                      sqrt((float)(1-Zorientation.getW()*Zorientation.getW())),
                      Zorientation.getY()/
                      sqrt((float)(1-Zorientation.getW()*Zorientation.getW())),
                      Zorientation.getZ()/
                      sqrt((float)(1-Zorientation.getW()*Zorientation.getW())));
  // RISK: Negative argument for SQRT OR Divide By Zero.
  // TO DO: Overflow unlikely, but check for this condition.
  axis.normalize();
  
  outs.str(""); 
  outs << "Rotation Angle [degrees]: " << angle*180.0/3.14159 << endl;
  std::cerr << outs.str().c_str() << std::endl;
  outs.str(""); 
  outs  << "Rotation Axis: [" << axis.getX() << "," << axis.getY() << "," 
        << axis.getZ() << "]" << endl;
  std::cerr << outs.str().c_str() << std::endl;
  
  // Compute translational component.
  // Centroid of triangle in frame coordinates
  Cf = (P2f + P4f + P6f)/3.0;
  
  // Centroid of frame triangle in image coordinates
  Cfi = Zorientation.RotateVector(Cf);
  
  // Centroid of triangle in image coordinates
  Ci = (Pz1 + Pz2 + Pz3)/3.0;
  
  // Displacement of frame in image coordinates.
  Zposition = Ci - Cfi; 

  if(fabs(Zposition.getZ())>20.0)
  {
  std::cerr << "vtkProstateNavCalibrationStep::LocalizeFrame - Displacement too large, something is wrong." << std::endl;
     return(false);
  }
  
  outs.str(""); 
  outs  << "Displacement [mm]: [" << Zposition.getX() << "," << Zposition.getY() 
        << "," << Zposition.getZ() << "]" << endl;
  std::cerr << outs.str().c_str() << std::endl;

  return(true);
}

/*----------------------------------------------------------------------------*/

/**
 * Find the the point at which the diagonal line fiducial is intercepted, 
 * using the three intercepts for a single set of planar line fiducials 
 * contained in one side of the Z-frame.
 * @param P1  Intercept point of first line fiducial in image.
 * @param P2  Intercept point of second line fiducial in image.
 * @param P3  Intercept point of third line fiducial in image.
 * @param Oz  Origin of this side of the Z-frame, in the Z-frame coordinates.
 * @param Vz  Vector representing the orientation of this side of the Z-frame, 
 *            in the Z-frame coordinates.
 * @param P2f Result: diagonal intercept in physical Z-frame coordinates. 
 */
void vtkProstateNavCalibrationStep::SolveZ(Column3Vector P1, Column3Vector P2, 
                               Column3Vector P3, Column3Vector Oz, 
                               Column3Vector Vz, Column3Vector &P2f)
{
   Column3Vector  Vtmp;
   float          D12, D23, Ld, Lc;

   // Normalize the direction vector of the diagonal fiducial.
   Vz.normalize();

   // Compute intercept.
   Vtmp = P1 - P2;
   D12 = Vtmp.norm();
   Vtmp = P2 - P3;
   D23 = Vtmp.norm();
   Ld = 60.0*sqrt((float)2.0);
   Lc = Ld*D23/(D12+D23);

   // Compute P2 in frame coordinates.
   P2f = Oz + Vz*Lc;
}

/*----------------------------------------------------------------------------*/

/**
 * This method computes the new image plane position and orientation that will
 * align the next image with the centre of the Z-frame.
 * @param pcurrent The current image position that is to be updated.
 * @param ocurrent The current image orientation that is to be updated.
 * @param Zposition The position correction.
 * @param Zorientation The orientation correction
 */
void vtkProstateNavCalibrationStep::Update_Scan_Plane(Column3Vector &pcurrent, 
                                          Quaternion &ocurrent, 
                                          Column3Vector Zposition, 
                                          Quaternion Zorientation)
{
   Quaternion Qcur, Qchg;

   // Update the scan plane position with the calculated offset in Zposition.
   //pcurrent = pcurrent - Zposition;
   double x = pcurrent.getX();
   double y = pcurrent.getY();
   double z = pcurrent.getZ();
   pcurrent.setvalues(x - Zposition.getX(), y - Zposition.getY(), z + Zposition.getZ());

   // Compute the new image orientation.
   //ocurrent = Zorientation * ocurrent;
   ocurrent = Zorientation * ocurrent;
}

/*----------------------------------------------------------------------------*/

/**
 * Find the largest magnitude value in a complex k-space image.
 * @param realmat The matrix of real components.
 * @param imagmat The matrix of imaginary components.
 * @return The magnitude value of the largest k-space element.
 */
Real vtkProstateNavCalibrationStep::ComplexMax(Matrix &realmat, Matrix &imagmat)
{
  Real maxabs=0, valabs, sqmag;
  
  for(int i=0; i<realmat.nrows(); i++)
    for(int j=0; j<realmat.ncols(); j++)
  {
      // Compute squared magnitude of the complex matrix element.
    sqmag = ((realmat.element(i,j)*realmat.element(i,j)) + 
        (imagmat.element(i,j)*imagmat.element(i,j)));

      // RISK: Argument for sqrt cannot be negative. Overflow?
    if(sqmag<0)
    {
    std::cerr << "vtkProstateNavCalibrationStep::ComplexMax - \
                                negative sqrt argument." << std::endl;
    } else
      {
        // SQRT for magnitude of complex matrix element.
        valabs = sqrt((double)(sqmag));
      }

      // Find maximum magnitude.
    if(maxabs < valabs) 
    {
      maxabs = valabs;
    }
  }

  return(maxabs);
}

/*----------------------------------------------------------------------------*/
 
/**
 * Find the maximum value in a matrix.
 * @param realmat A matrix.
 * @return The value of the largest matrix element.
 */
Real vtkProstateNavCalibrationStep::RealMax(Matrix &realmat)
{
  Real maxabs=0, valabs;
  
  
  for(int i=0; i<realmat.nrows(); i++)
    for(int j=0; j<realmat.ncols(); j++)
    {
        // Find the maximum element value.
      if(maxabs < realmat.element(i,j))
      {
        maxabs = realmat.element(i,j);
      } 
    }

  return(maxabs);
}

/*----------------------------------------------------------------------------*/
 
/**
 * Find the maximum value in a matrix, as well as its row and column
 * coordinates in the matrix.
 * @param inmatrix The input matrix.
 * @param row The Resulting row index at which the max value occurs.
 * @param col The Resulting column index at which the max value occurs.
 * @return The value of the largest matrix element.
 */
Real vtkProstateNavCalibrationStep::FindMax(Matrix &inmatrix, int &row, int &col)
{
  Real maxabs=0;
  row = col = 0;
  
  // Avoid 10-pixel margin, due to image artifact.
  for(int i=10; i<inmatrix.nrows()-10; i++)
    for(int j=10; j<inmatrix.ncols()-10; j++)
  {
    if(maxabs < inmatrix.element(i,j))
    {
          // Record the maximum value, as well as the coordinates where it 
          // occurs.
      maxabs = inmatrix.element(i,j); 
      row = i;
      col = j;
    }
  }

  return(maxabs);
}

/*= END METHODS ==============================================================*/


