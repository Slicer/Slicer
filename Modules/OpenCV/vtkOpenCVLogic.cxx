/*==========================================================================
 
 Portions (c) Copyright 2009 Atsushi Yamada (Fujimoto Lab, Nagoya Institute of Technology (NIT) All Rights Reserved.
 
 Acknowledgement: K. Chinzei (AIST), Y. Hayashi(Nagoya Univ.), T. Takeuchi (SFC Corp.), J. Tokuda(BWH), N. Hata(BWH), and H. Fujimoto(NIT) 
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).
 
 See README.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Program:   OpenCV
 Module:    $HeadURL: $
 Date:      $Date:07/17/2009 $
 Version:   $Revision: $
 
 ==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkOpenCVLogic.h"

#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"

extern float tx;
extern float ty;
extern float tz;
extern float t0;
extern float sx;
extern float sy;
extern float sz;
extern float s0;
extern float nx;
extern float ny;
extern float nz;
extern float n0;
extern float px;
extern float py;
extern float pz;
extern float p0;

extern float tx2;
extern float ty2;
extern float tz2;
extern float t02;
extern float sx2;
extern float sy2;
extern float sz2;
extern float s02;
extern float nx2;
extern float ny2;
extern float nz2;
extern float n02;
extern float px2;
extern float py2;
extern float pz2;
extern float p02;

extern char *buffer;
extern int rewriteFlag;

extern int textActor1Flag;
extern int textActor2Flag;
extern int textActor3Flag;
extern int textActor4Flag;
extern int textActor5Flag;

//Query image from camera
CvCapture* capture = NULL;
vtkImageImport *importer = vtkImageImport::New();    
vtkImageViewer *viewer = vtkImageViewer::New();

IplImage*    captureImage = NULL;
IplImage*    RGBImage = NULL;
IplImage*    captureImageTmp = NULL;
CvSize        imageSize;
unsigned char* idata = NULL;

vtkImageMapper *captureMapper = vtkImageMapper::New();
vtkActor2D *captureActor = vtkActor2D::New();
vtkRenderer *captureRen = vtkRenderer::New();
vtkRenderWindow *captureRenWin = vtkRenderWindow::New();

// for renderer window capture
vtkRenderWindow *renWinForCapture = vtkRenderWindow::New();
vtkRenderer *renForCapture = vtkRenderer::New();

// OpenCV capture refresh.
void *thread_cvGetImageFromCamera(void *arg);

// thread exit flag
int thread_exit_flag = 0;

// mutex
pthread_mutex_t work_mutex;

/*
if(res != 0) {
    perror("Mutex initialization failed");
    exit(EXIT_FAILURE);
}
*/

// for text overlay
vtkTextActor *textActor = vtkTextActor::New();
vtkTextActor *textActor1 = vtkTextActor::New();
vtkTextActor *textActor2 = vtkTextActor::New();
vtkTextActor *textActor3 = vtkTextActor::New();
vtkTextActor *textActor4 = vtkTextActor::New();
vtkTextActor *textActor5 = vtkTextActor::New();
vtkTextActor *textActor6 = vtkTextActor::New();
vtkActor2D *testActor = vtkActor2D::New();

// status of suction tube
vtkTextActor *suctionActor = vtkTextActor::New();
vtkTextActor *suctionActor1 = vtkTextActor::New();
vtkTextActor *suctionActor2 = vtkTextActor::New();
vtkTextActor *suctionActor3 = vtkTextActor::New();
vtkTextActor *suctionActor4 = vtkTextActor::New();
vtkTextActor *suctionActor5 = vtkTextActor::New();
vtkTextActor *suctionActor6 = vtkTextActor::New();
vtkTextActor *suctionActor7 = vtkTextActor::New();

// status of baloon sensor
vtkTextActor *baloonActor = vtkTextActor::New();
vtkTextActor *baloonActor1 = vtkTextActor::New();
vtkTextActor *baloonActor2 = vtkTextActor::New();
vtkTextActor *baloonActor3 = vtkTextActor::New();
vtkTextActor *baloonActor4 = vtkTextActor::New();
vtkTextActor *baloonActor5 = vtkTextActor::New();
vtkTextActor *baloonActor6 = vtkTextActor::New();
vtkTextActor *baloonActor7 = vtkTextActor::New();
vtkTextActor *baloonActor8 = vtkTextActor::New();

// for text signal 
int textSignalFlag = 0; 

// for poly data (test)
vtkTexture *texture = vtkTexture::New();
vtkTexture *texture2 = vtkTexture::New();
vtkPlaneSource *planeSource = vtkPlaneSource::New();
vtkCubeSource *cubeSource = vtkCubeSource::New();
vtkPolyDataNormals *polyDataNormals = vtkPolyDataNormals::New();
vtkPolyDataMapper *polyDataMapperTexture = vtkPolyDataMapper::New();
vtkActor *actorTexture = vtkActor::New();

#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
vtkPolyDataToImageStencil *polyData2IS = vtkPolyDataToImageStencil::New(); 
vtkImageStencil *IS = vtkImageStencil::New(); 

vtkPNGReader *blendPNGReader =  vtkPNGReader::New();
vtkBMPReader *blendBMPReader =  vtkBMPReader::New();

#include <vtkImageReslice.h>
vtkImageReslice *reslice = vtkImageReslice::New();

// for image blend test
vtkImageMapper *blendMapper = vtkImageMapper::New();
vtkActor2D *blendActor = vtkActor2D::New();
vtkRenderer *blendRen = vtkRenderer::New();
//blendRenWin = vtkRenderWindow::New();
vtkImageBlend *captureBlend = vtkImageBlend::New();
vtkDataSetMapper *blendDataMapper = vtkDataSetMapper::New();

// set up of window capture
vtkWindowToImageFilter *w2if=vtkWindowToImageFilter::New();
vtkPNGWriter *wr=vtkPNGWriter::New();
int writeFlag=0;
int writeFlagFlag=0;

vtkCxxRevisionMacro(vtkOpenCVLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkOpenCVLogic);




//---------------------------------------------------------------------------
vtkOpenCVLogic::vtkOpenCVLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkOpenCVLogic::DataCallback);

}


//---------------------------------------------------------------------------
vtkOpenCVLogic::~vtkOpenCVLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkOpenCVLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkOpenCVLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkOpenCVLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkOpenCVLogic *self = reinterpret_cast<vtkOpenCVLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkOpenCVLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkOpenCVLogic::UpdateAll()
{

}


//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkOpenCVLogic::SetVisibilityOfCaptureData(const char* nodeName, int v)
{
    vtkMRMLModelNode*   locatorModel;

    locatorModel = initCapture(nodeName, 1.0, 0.0, 1.0);

    return locatorModel;
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkOpenCVLogic::SetCloseOfCaptureData(const char* nodeName, int v)
{
    vtkMRMLModelNode*   locatorModel;
    
    thread_exit_flag = 1;
    
    captureMapper->Delete();
    captureActor->Delete();
    captureRen->Delete();
    captureRenWin->Delete();

    cvReleaseImage(&RGBImage);
    cvReleaseImage(&captureImage);
    cvReleaseCapture(&capture);
    
    return locatorModel;
}

// thread function
void *thread_cvGetImageFromCamera(void *arg)
{

    while(1)
    {
        
    
        captureImageTmp = cvQueryFrame( capture );
    
        imageSize = cvGetSize( captureImageTmp );
//        captureImage = cvCloneImage(captureImageTmp);
//        RGBImage = cvCreateImage( imageSize, IPL_DEPTH_8U, 3);
    
        cvFlip(captureImageTmp, captureImage, -1);
    
    
        cvCvtColor( captureImage, RGBImage, CV_BGR2RGB);
    //    cvCvtColor( captureImage, RGBImage, CV_BGR2GRAY);
    
        // rgb image into vtk
        //unsigned char* idata = NULL;
        idata = (unsigned char*) RGBImage->imageData;
    
        //    vtkImageImport *importer = vtkImageImport::New();
        importer->SetWholeExtent(0,imageSize.width-1,0,imageSize.height-1,0,0);
        importer->SetDataExtentToWholeExtent();
        //    importer->SetDataScalarTypeToUnsignedShort();
        importer->SetDataScalarTypeToUnsignedChar();
        importer->SetNumberOfScalarComponents(3);
        //    importer->SetNumberOfScalarComponents(1);
        importer->SetImportVoidPointer(idata);    

        importer->Update();
        captureRenWin->Render();

        
        captureRen->AddActor(textActor);
        captureRen->AddActor(textActor4);    
        captureRen->AddActor(textActor6);
        
        captureRen->AddActor(suctionActor);
        captureRen->AddActor(suctionActor1);
        captureRen->AddActor(suctionActor2);
        captureRen->AddActor(suctionActor3);
        captureRen->AddActor(suctionActor4);
        captureRen->AddActor(suctionActor5);
        captureRen->AddActor(suctionActor6);
        captureRen->AddActor(suctionActor7);
        
        captureRen->AddActor(baloonActor);
        captureRen->AddActor(baloonActor1);
        captureRen->AddActor(baloonActor2);
        captureRen->AddActor(baloonActor3);
        captureRen->AddActor(baloonActor4);
        captureRen->AddActor(baloonActor5);
        captureRen->AddActor(baloonActor6);
        captureRen->AddActor(baloonActor7);
        
        
        
        if(textSignalFlag <= 10)
        {

            captureRen->AddActor(textActor1);
            captureRen->AddActor(textActor2);
            captureRen->AddActor(textActor3);
            captureRen->AddActor(textActor5);    

        }else if(textSignalFlag <= 20){
        
            if(textActor1Flag == 0){
                captureRen->RemoveActor(textActor1);
            }
            
            if(textActor2Flag == 0){
                captureRen->RemoveActor(textActor2);
            }
            
            if(textActor3Flag == 0){
                captureRen->RemoveActor(textActor3);
            }
            
            if(textActor5Flag == 0){
                captureRen->RemoveActor(textActor5);
            }
            
        }else{
            textSignalFlag=0;
        }
        
        textSignalFlag++;
        //}            
        
    
        if(thread_exit_flag == 1)
        {
        //    pthread_exit ("close the second window.");
        }

    
    }

    
}

void vtkOpenCVLogic::cvGetImageFromCamera(void)
{
    
        captureImageTmp = cvQueryFrame( capture );

        cvFlip(captureImageTmp, captureImage, -1);
        
        
        cvCvtColor( captureImage, RGBImage, CV_BGR2RGB);
        //    cvCvtColor( captureImage, RGBImage, CV_BGR2GRAY);
        
        // rgb image into vtk
        //unsigned char* idata = NULL;
        idata = (unsigned char*) RGBImage->imageData;

    
        //    vtkImageImport *importer = vtkImageImport::New();
        importer->SetWholeExtent(0,imageSize.width-1,0,imageSize.height-1,0,0);
        importer->SetDataExtentToWholeExtent();
        //    importer->SetDataScalarTypeToUnsignedShort();
        importer->SetDataScalarTypeToUnsignedChar();
        importer->SetNumberOfScalarComponents(3);
        //    importer->SetNumberOfScalarComponents(1);
        importer->SetImportVoidPointer(idata);    

        importer->Update();

        captureRenWin->Render();
    
}


void vtkOpenCVLogic::cvCaptureAndView(void)
{
    
    if(NULL==(capture = cvCaptureFromCAM(0)))
    {
        printf("cannot find a camera");
    //    return -1;
    }
    
    viewer->SetInputConnection(importer->GetOutputPort());
    //    viewer->SetZSlice(45);
    viewer->SetZSlice(0);
    viewer->SetColorWindow(512);//2000);
    viewer->SetColorLevel(256);
    //viewer->Render();

}


vtkMRMLModelNode* vtkOpenCVLogic::initCapture(const char* nodeName, double r, double g, double b)
{

    // for threading
    int res;
    pthread_t a_thread;
    
    vtkMRMLModelNode           *locatorModel;
    vtkMRMLModelDisplayNode    *locatorDisp;
    
    
    locatorModel = vtkMRMLModelNode::New();
    locatorDisp = vtkMRMLModelDisplayNode::New();
    
    GetMRMLScene()->SaveStateForUndo();
    GetMRMLScene()->AddNode(locatorDisp);
    GetMRMLScene()->AddNode(locatorModel);  
    
    locatorDisp->SetScene(this->GetMRMLScene());
    
    locatorModel->SetName(nodeName);
    locatorModel->SetScene(this->GetMRMLScene());
    locatorModel->SetAndObserveDisplayNodeID(locatorDisp->GetID());
    locatorModel->SetHideFromEditors(0);

    textActor4->SetInput("Status [DEMO]");
    textActor4->GetTextProperty()->SetFontSize(12);
    textActor4->SetPosition(10,170);
    textActor4->GetTextProperty()->BoldOn();

    textActor6->SetInput("Camera Connection: ON");
    textActor6->GetTextProperty()->SetFontSize(12);
    textActor6->SetPosition(10,150);
    
    textActor2->SetInput("Robot Connection: OFF");
    textActor2->GetTextProperty()->SetFontSize(12);
    textActor2->SetPosition(10,130);

    textActor3->SetInput("Network Connection: OFF");
    textActor3->GetTextProperty()->SetFontSize(12);
    textActor3->SetPosition(10,110);
    
    textActor1->SetInput("Baloon Sensor: OFF");
    textActor1->GetTextProperty()->SetFontSize(12);
    textActor1->SetPosition(10,90);

    textActor5->SetInput("Suction Tube: OFF");
    textActor5->GetTextProperty()->SetFontSize(12);
    textActor5->SetPosition(10,70); 
    
    textActor->SetInput("Robot Console ver. 0.1-Alpha Based on Slicer3.4");
    textActor->GetTextProperty()->BoldOn();
    textActor->GetTextProperty()->ItalicOn();
    textActor->SetPosition(10,0);

    textActor->GetTextProperty()->SetFontSize(14);
    textActor->GetTextProperty()->SetOpacity(0.5);
    
    // display status about suction tube
    suctionActor->SetInput("Suction Tube Status");
    suctionActor->GetTextProperty()->SetFontSize(12);
    suctionActor->SetPosition(200,170);
    suctionActor->GetTextProperty()->BoldOn();

    suctionActor1->SetInput("Regulator / Suction Pressure");
    suctionActor1->GetTextProperty()->SetFontSize(12);
    suctionActor1->SetPosition(200,150);

    suctionActor2->SetInput("Suction Tube Switch");
    suctionActor2->GetTextProperty()->SetFontSize(12);
    suctionActor2->SetPosition(200,130);
    
    suctionActor3->SetInput("Scalpel cut");
    suctionActor3->GetTextProperty()->SetFontSize(12);
    suctionActor3->SetPosition(200,110);
    
    suctionActor4->SetInput("Scalpel coag");
    suctionActor4->GetTextProperty()->SetFontSize(12);
    suctionActor4->SetPosition(200,90);
    
    suctionActor5->SetInput("Scalpel On/Off");
    suctionActor5->GetTextProperty()->SetFontSize(12);
    suctionActor5->SetPosition(200,70);

    suctionActor6->SetInput("Scalpel Time");
    suctionActor6->GetTextProperty()->SetFontSize(12);
    suctionActor6->SetPosition(200,50);

    suctionActor7->SetInput("System Status");
    suctionActor7->GetTextProperty()->SetFontSize(12);
    suctionActor7->SetPosition(200,30);

    // display status about baloon sensor
    baloonActor->SetInput("Baloon Sensor Status");
    baloonActor->GetTextProperty()->SetFontSize(12);
    baloonActor->SetPosition(600,170);
    baloonActor->GetTextProperty()->BoldOn();

    baloonActor1->SetInput("Status");
    baloonActor1->GetTextProperty()->SetFontSize(12);
    baloonActor1->SetPosition(600,150);
    
    baloonActor2->SetInput("Pressure");
    baloonActor2->GetTextProperty()->SetFontSize(12);
    baloonActor2->SetPosition(600,130);
    
    baloonActor3->SetInput("Volume");
    baloonActor3->GetTextProperty()->SetFontSize(12);
    baloonActor3->SetPosition(600,110);
    
    baloonActor4->SetInput("Diff_Pressure");
    baloonActor4->GetTextProperty()->SetFontSize(12);
    baloonActor4->SetPosition(600,90);
    
    cubeSource->SetCenter(0.0,-200.0,0.0);    

    polyDataNormals->SetInputConnection( cubeSource->GetOutputPort() );
    polyDataMapperTexture->SetInputConnection( polyDataNormals->GetOutputPort() );

    actorTexture->SetMapper(polyDataMapperTexture);
    
    blendBMPReader->SetFileName("/Users/ayamada/Desktop/videoOpenIGTLink/bmpData/1.bmp");
    blendBMPReader->SetDataSpacing(1.0,1.0,1.0);
    reslice->SetInput(blendBMPReader->GetOutput());

    reslice->SetOutputSpacing(0.5, 0.5, 0.5);

    texture2->SetInputConnection(reslice->GetOutputPort());
    actorTexture->SetTexture(texture2);

    actorTexture->GetProperty()->SetOpacity(0.2);    

    
    // for window capture
//    renForCapture->AddActor(actorTexture);
//    renWinForCapture->AddRenderer(renForCapture);
    
//    w2if->SetInput(renWinForCapture);
//    wr->SetInputConnection(w2if->GetOutputPort());
    //    wr->SetFileName("Capture.png");
    //    wr->Write();
    //    w2if->Delete();
    //    wr->Delete();    
    
    
//    polyData2IS->SetInputConnection(polyDataNormals->GetOutputPort());
    
    // image blend test
    captureBlend->SetInputConnection(importer->GetOutputPort());
//    captureBlend->AddInputConnection(wr->GetOutputPort());
//    captureBlend->AddInputConnection(blendBMPReader->GetOutputPort());
    captureBlend->SetBlendMode(1);
    captureBlend->SetOpacity(1,0.4);

    blendMapper->SetInputConnection(captureBlend->GetOutputPort());
    blendMapper->SetZSlice(0);
    blendMapper->SetColorWindow(512);
    blendMapper->SetColorLevel(256);
    
    blendActor->SetMapper(blendMapper);
    blendActor->GetProperty()->SetOpacity(0.7);

    
    // capture setup
    if(NULL==(capture = cvCaptureFromCAM(0)))
    {
        printf("cannot find a camera");
        //    return -1;
    }
    
    
    captureMapper->SetInputConnection( importer->GetOutputPort() );

    captureMapper->SetZSlice(0);
    captureMapper->SetColorWindow(512);
    captureMapper->SetColorLevel(256);
    
    captureActor->SetMapper( captureMapper );

    
    captureRen->SetBackground( 0, 0, 0 );
    
    captureRen->AddActor( captureActor );
    
    captureRenWin->AddRenderer( captureRen );
    captureRenWin->SetSize(640,480);
    
    
    captureRenWin->SetPosition(100,100);
    
    captureImageTmp = cvQueryFrame( capture );
    
    imageSize = cvGetSize( captureImageTmp );
    captureImage = cvCreateImage(imageSize, IPL_DEPTH_8U,3);    
    RGBImage = cvCreateImage( imageSize, IPL_DEPTH_8U, 3);    
    
    // initial data
    cvGetImageFromCamera();
    importer->Update();
    
    char message[] ="thread test";
    
    // start thread
    res = pthread_create(&a_thread, NULL, thread_cvGetImageFromCamera, (void *)message);
    if(res != 0) {
        perror("Thread creation faild");
    //    exit(EXIT_FAILURE);
    }
//    res = pthread_join(a_thread, &thread_result);
    
    locatorDisp->Delete();    
    return locatorModel;    
    
}



