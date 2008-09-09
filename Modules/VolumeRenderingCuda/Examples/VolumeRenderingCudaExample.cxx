#include "vtkActor.h"
#include "vtkKWApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWWindow.h"
#include "vtkRenderWindow.h"
#include "vtkKWSimpleAnimationWidget.h"


#include "vtkKWVolumePropertyWidget.h"
#include "vtkKWRange.h"
#include "vtkCallbackCommand.h"
#include "vtkKWEvent.h"
#include "vtkKWCheckButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCudaVolumeMapper.h"
#include "vtkImageReader.h"
#include <sstream>
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"

#include "vtkImageWriter.h"
#include "vtkCudaImageDataFilter.h"
#include "vtkCudaImageData.h"
#include "vtkImageData.h"

#include "vtkImageClip.h"

#include "vtkInteractorStyleTrackballCamera.h"

#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkVolumeTextureMapper2D.h"
#include "vtkVolumeTextureMapper3D.h"


vtkKWApplication *app;
vtkKWRenderWidget* renderWidget;
vtkKWRange* ThresholdRange;
vtkKWScale* SteppingSizeScale;
vtkKWScale* ScaleFactorScale;
vtkKWVolumePropertyWidget* VolumePropertyWidget;
vtkVolumeMapper* VolumeMapper;
vtkVolume* Volume;
vtkKWCheckButton* cb_Animate;
vtkKWCheckButton* cb_Animate2;
vtkKWMenuButton*  mb_Model;
vtkKWMenuButton*  mb_Mapper;

int frameNumber = 0;
std::vector<vtkImageReader*> readers;
std::vector<vtkImageClip*> clippers;
bool renderScheduled = false;

void Clear()
{
    while (!readers.empty())
    {
        readers.back()->Delete();
        readers.pop_back();
    }
    VolumeMapper->SetInput((vtkImageData*)NULL);
}

void LoadHead()
{
    Clear();
    readers.push_back(vtkImageReader::New());
    readers[0]->SetDataScalarTypeToUnsignedChar();
    readers[0]->SetNumberOfScalarComponents(1);
    readers[0]->SetDataExtent(0, 255, 0, 255, 0, 93);
    readers[0]->SetDataSpacing(1.0, 1.0, 1.0);
    readers[0]->SetFileDimensionality(3);
    readers[0]->SetFileName("D:\\fullhead94.raw");
    readers[0]->Update();
}

void LoadBunny()
{
    Clear();
    readers.clear();
    readers.push_back(vtkImageReader::New());
    readers[0]->SetDataByteOrderToBigEndian();
    readers[0]->SetDataScalarTypeToShort();
    readers[0]->SetNumberOfScalarComponents(1);
    readers[0]->SetDataExtent(0, 511, 0, 511, 0, 359);
    readers[0]->SetDataSpacing(1.0, 1.0, 1.0);
    readers[0]->SetFileDimensionality(3);
    readers[0]->SetFileName("D:\\Volumes\\Bunny\\Bunny.raw");
    readers[0]->Update();
}


void LoadHeart()
{
    Clear();
    readers.push_back(vtkImageReader::New());
    readers[0]->SetDataScalarTypeToUnsignedChar();
    readers[0]->SetNumberOfScalarComponents(1);
    readers[0]->SetDataExtent(0, 255, 0, 255, 0, 255);
    readers[0]->SetDataSpacing(1.0, 1.0, 1.0);
    readers[0]->SetFileDimensionality(3);
    readers[0]->SetFileName("D:\\heart256.raw");
    readers[0]->Update();
}

void LoadHeartSeries()
{
    Clear();
    //    Reading in the Data using a ImageReader
    for (unsigned int i = 0; i < 5; i++ ) 
    {
        readers.push_back(vtkImageReader::New());

        readers[i]->SetDataScalarTypeToUnsignedChar();
        readers[i]->SetNumberOfScalarComponents(1);
        readers[i]->SetDataExtent(0, 255,
            0, 255, 
            0, 255);
        readers[i]->SetFileDimensionality(3);
        readers[i]->SetDataSpacing(1.0, 1.0, 1.0);

        std::stringstream s;
        s << "D:\\heart256-" << i+1 << ".raw";

        readers[i]->SetFileName(s.str().c_str());
        readers[i]->Update();
    }
}

void LoadLung()
{
    Clear();
    readers.push_back(vtkImageReader::New());

    readers[0]->SetDataScalarTypeToShort();
    readers[0]->SetNumberOfScalarComponents(1);
    readers[0]->SetDataExtent(0, 127, 0, 127, 0, 29);
    readers[0]->SetFileDimensionality(3);
    readers[0]->SetDataSpacing(3.125f, 3.125f, 5.000f);

    std::stringstream s;
    s << "D:\\Volumes\\RawLung\\92\\lung.raw";

    readers[0]->SetFileName(s.str().c_str());
    readers[0]->Update();
}

void LoadLungSeries()
{
    Clear();
    int j = 92;
    for (unsigned int i = 0; i < 11; i ++)
    {
        readers.push_back(vtkImageReader::New());
        readers[i]->SetDataScalarTypeToShort();
        readers[i]->SetNumberOfScalarComponents(1);
        readers[i]->SetDataExtent(0, 127, 0, 127, 0, 29);
        readers[i]->SetFileDimensionality(3);
        readers[i]->SetDataSpacing(3.125f, 3.125f, 5.000f);

        std::stringstream s;
        s << "D:\\Volumes\\RawLung\\" << j++ << "\\lung.raw";

        readers[i]->SetFileName(s.str().c_str());
        readers[i]->Update();
    }
}

void LoadUltrasoundHeartSeries()
{
    Clear();
    int j = 92;
    for (unsigned int i = 0; i < 50; i ++)
    {
        readers.push_back(vtkImageReader::New());
        readers[i]->SetDataScalarTypeToUnsignedChar();
        readers[i]->SetNumberOfScalarComponents(1);
        readers[i]->SetDataExtent(0, 79, 0, 79, 0, 159);
        readers[i]->SetFileDimensionality(3);
        readers[i]->SetDataSpacing(1.0f, 1.0f, 0.74f);
        readers[i]->SetHeaderSize(i * 80 * 80 * 160 * sizeof(unsigned char));

        std::stringstream s;
        s << "D:\\Volumes\\4DUltrasound\\3DDCM002.raw";

        readers[i]->SetFileName(s.str().c_str());
        readers[i]->Update();
    }
}

void LoadProstate()
{
    Clear();
    readers.push_back(vtkImageReader::New());

    readers[0]->SetDataScalarTypeToShort();
    readers[0]->SetNumberOfScalarComponents(1);
    readers[0]->SetDataExtent(0, 255, 0, 255, 0, 53);
    readers[0]->SetDataSpacing(1, 0.65, 3.5);
    readers[0]->SetFileDimensionality(3);
    readers[0]->SetFileName("D:\\prostate.raw");
    readers[0]->Update();
}

void SetClipRatio(int val)
{
    for (unsigned int i = 0; i < clippers.size(); i++)
    {
        clippers[i]->SetOutputWholeExtent(
            (readers[i]->GetOutput()->GetExtent()[1] - readers[i]->GetOutput()->GetExtent()[0]) / 2 -1 - val,
            (readers[i]->GetOutput()->GetExtent()[1] - readers[i]->GetOutput()->GetExtent()[0]) / 2 + val,
            (readers[i]->GetOutput()->GetExtent()[3] - readers[i]->GetOutput()->GetExtent()[2]) / 2 - 1 - val,
            (readers[i]->GetOutput()->GetExtent()[3] - readers[i]->GetOutput()->GetExtent()[2]) / 2 + val,
            (readers[i]->GetOutput()->GetExtent()[5] - readers[i]->GetOutput()->GetExtent()[4]) / 2 - 1 - val,
            (readers[i]->GetOutput()->GetExtent()[5] - readers[i]->GetOutput()->GetExtent()[4]) / 2 + val);
        clippers[i]->ClipDataOn();

        clippers[i]->Update();
    }
}

void ChangeModel(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
    cb_Animate->SetSelectedState(0);
    if (! strcmp(mb_Model->GetValue(), "Head"))
        LoadHead();
    else if (!strcmp(mb_Model->GetValue(), "Heart"))
        LoadHeartSeries();
    else if (!strcmp(mb_Model->GetValue(), "Lung"))
        LoadLungSeries();
    else if (!strcmp(mb_Model->GetValue(), "Bunny"))
        LoadBunny();
    else if (!strcmp(mb_Model->GetValue(), "Prostate"))
        LoadProstate();
    else if (!strcmp(mb_Model->GetValue(), "Ultrasound Heart"))
        LoadUltrasoundHeartSeries();
    else
        Clear();



    unsigned int i;
    for (i = 0; i < clippers.size(); i++)
        clippers[i]->Delete();
    clippers.clear();
    for (i = 0; i < readers.size(); i++)
    {
        clippers.push_back(vtkImageClip::New());
        clippers[i]->SetInput(readers[i]->GetOutput());
        clippers[i]->SetOutputWholeExtent(readers[i]->GetOutput()->GetExtent());
        clippers[i]->ClipDataOn();
        clippers[i]->Update();
        VolumeMapper->Update();
    }
    SetClipRatio(50);

    if (!readers.empty())
        VolumeMapper->SetInput(clippers[0]->GetOutput());

    app->Script("after idle %s Render", renderWidget->GetTclName());
}

void SetMapper(vtkVolumeMapper* mapper)
{
    VolumeMapper->Delete();
    VolumeMapper = mapper;
    if (!readers.empty())
        VolumeMapper->SetInput(clippers[0]->GetOutput());
    Volume->SetMapper(VolumeMapper);
}


void ChangeMapper(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
    if (!strcmp(mb_Mapper->GetValue(), "CUDA"))
        SetMapper(vtkCudaVolumeMapper::New());
    else if (!strcmp(mb_Mapper->GetValue(), "Texture_2D"))
        SetMapper(vtkVolumeTextureMapper2D::New());
    else if (!strcmp(mb_Mapper->GetValue(), "Texture_3D"))
        SetMapper(vtkVolumeTextureMapper3D::New());
    else if (!strcmp(mb_Mapper->GetValue(), "Software_Ray_Caster"))
        SetMapper(vtkFixedPointVolumeRayCastMapper::New());
//    app->Script("after idle %s Render", renderWidget->GetTclName());
}



#include "vtkTimerLog.h"
const int sampleMax = 29;
const int clipStart = 7;
const int clipStop = 127;
int clipRatio = clipStart;
vtkTimerLog* logger = vtkTimerLog::New();
std::vector<double> vals;
ofstream f_means;
ofstream f_deviations;
ofstream f_volSizes;

void UpdateRenderer(vtkObject *caller, unsigned long eid, void *clientData, void *callData)
{
    if (caller == cb_Animate)
    {
        renderWidget->GetRenderer()->GetActiveCamera()->SetPosition(500, 0, 500);
        vals.clear();
        clipRatio = clipStart;
        SetClipRatio(clipRatio);
        if (cb_Animate->GetSelectedState() == 1)
        {
            app->Script("after idle %s Render", renderWidget->GetTclName());
            std::stringstream name;
            name << "D:\\measurements\\" << mb_Mapper->GetValue() << "_mean" << ((cb_Animate2->GetSelectedState() == 1)? "_anim" : "") << ".txt" ;
            f_means.open(name.str().c_str());
            std::stringstream devName;
            devName << "D:\\measurements\\" << mb_Mapper->GetValue() << "_deviation" << ((cb_Animate2->GetSelectedState() == 1)? "_anim" : "") << ".txt" ;
            f_deviations.open(devName.str().c_str());
            f_volSizes.open("D:\\measurements\\VolSizes.txt");
        }
        else if (f_volSizes != NULL) 
        {
            f_means.close(); 
            f_deviations.close();;
            f_means.close();
        }

    }

    //if (caller == ThresholdRange)
    //    VolumeMapper->SetThreshold(ThresholdRange->GetRange());
    //else if (caller == SteppingSizeScale)
    //    VolumeMapper->SetSampleDistance(SteppingSizeScale->GetValue());
    if (caller == ScaleFactorScale)
    {
        vtkCudaVolumeMapper* mapper = vtkCudaVolumeMapper::SafeDownCast(VolumeMapper);
        if (mapper != NULL) mapper->SetRenderOutputScaleFactor(ScaleFactorScale->GetValue());
    }
    //renderWidget->SetRenderModeToInteractive();
    //    VolumeMapper->SetInput(clippers[0]->GetOutput());
    //app->Script("after 10 %s Render", renderWidget->GetTclName());
    //    renderWidget->Modified();
    //renderWidget->GetRenderer()->Render();
}



void StartRender(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
    //cout << renderWidget->GetRenderer()->GetSize()[0] << "x" << renderWidget->GetRenderer()->GetSize()[1] << endl; 
    if (cb_Animate->GetSelectedState() == 1)
    {
        renderWidget->GetRenderer()->GetActiveCamera()->SetPosition(500+ sin((float)vals.size()/5.0) * 100,
            500, 
            500 +cos((float)vals.size()/5.0) * 100);
    }
    if (++frameNumber == readers.size())
        frameNumber = 0;

    logger->StartTimer();
    if (cb_Animate2->GetSelectedState() == 1)
         VolumeMapper->SetInput(clippers[frameNumber]->GetOutput());


    renderScheduled = false;
//    cout << "begin \n";
}

void Animate(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
    logger->StopTimer();
//    cout << "Last RenderTime: " << logger->GetElapsedTime()  << endl;
    //if (cb_Animate2->GetSelectedState() == 1)
    //{
    //    if (!renderScheduled)
    //  //      app->Script("after 1000 %s Render", renderWidget->GetTclName());
    //    renderScheduled = true;
    //}
    if (cb_Animate->GetSelectedState() == 1)
    {

        vals.push_back(logger->GetElapsedTime());
        unsigned int i;
        double mean = 0.0;
        for (i = 0; i < vals.size(); i++)
            mean += vals[i];
        mean /= vals.size();
        double deviation = 0.0;
        for (i = 0; i < vals.size(); i++)
            deviation += (vals[i] - mean) * (vals[i] - mean);
        deviation = sqrt(deviation / vals.size());


        cout << vals.size() << "::" << clipRatio << ": Last RenderTime: " << logger->GetElapsedTime() 
            << " Mean RenderTime: " << mean << " Deviation: " << deviation << endl;
        if (vals.size() > sampleMax)
        {
            if (f_volSizes != NULL)
            {
                f_volSizes << clipRatio << std::endl;
                f_deviations << deviation << std::endl;
                f_means << mean << std::endl;
            }
            vals.clear();
            clipRatio++; 
            SetClipRatio(clipRatio);
        }
        if (clipRatio < clipStop )
            app->Script("after idle %s Render", renderWidget->GetTclName());
        else
        {
            if (f_volSizes != NULL) 
            {
                f_volSizes.close();
                f_deviations.close();
                f_means.close();
            //    cb_Animate->SetSelectedState(0);
            }
            if (cb_Animate2->GetSelectedState() == 0)
            {
                cb_Animate2->SetSelectedState(1);
                UpdateRenderer(cb_Animate, 0, NULL, NULL);
            }
            else
            {
                cb_Animate2->SetSelectedState(0);
                int val = mb_Mapper->GetMenu()->GetIndexOfItem(mb_Mapper->GetValue());
                if (++val < 4)
                {
                    mb_Mapper->GetMenu()->SelectItem(val);
                    mb_Mapper->SetValue(mb_Mapper->GetMenu()->GetItemLabel(val));
                    ChangeMapper(mb_Mapper, 0, NULL, NULL);
                    UpdateRenderer(cb_Animate, 0, NULL, NULL);
                }
                else
                {
                    cb_Animate->SetSelectedState(0);
                }
            }


        }
        //if (renderScheduled == false)
        //{    
        //    renderScheduled = true;
        //    renderWidget->SetRenderModeToInteractive();
        //    cerr << *renderWidget;
        //    //  app->Script("after 100 %s Render", renderWidget->GetTclName());
        //}
    }
}


int my_main(int argc, char *argv[])
{
    // Initialize Tcl

    Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
    if (!interp)
    {
        cerr << "Error: InitializeTcl failed" << endl ;
        return 1;
    }

    // Process some command-line arguments
    // The --test option here is used to run this example as a non-interactive 
    // test for software quality purposes. You can ignore it.

    int option_test = 0;
    vtksys::CommandLineArguments args;
    args.Initialize(argc, argv);
    args.AddArgument(
        "--test", vtksys::CommandLineArguments::NO_ARGUMENT, &option_test, "");
    args.Parse();

    app = vtkKWApplication::New();
    app->SetName("CUDA/VTK Volume Rendering Viewer");
    if (option_test)
    {
        app->SetRegistryLevel(0);
        app->PromptBeforeExitOff();
    }
    app->RestoreApplicationSettingsFromRegistry();

    // Set a help link. Can be a remote link (URL), or a local file

    app->SetHelpDialogStartingPage("http://www.kwwidgets.org");

    // Add a window
    // Set 'SupportHelp' to automatically add a menu entry for the help link

    vtkKWWindow *win = vtkKWWindow::New();
    win->SupportHelpOn();
    app->AddWindow(win);
    win->Create();
    win->SecondaryPanelVisibilityOff();

    // Add a render widget, attach it to the view frame, and pack

    renderWidget = vtkKWRenderWidget::New();

    renderWidget->SetBackgroundColor(200, 200, 200);
    renderWidget->SetParent(win->GetViewFrame());
    renderWidget->Create();

    app->Script("pack %s -expand y -fill both -anchor c -expand y", 
        renderWidget->GetWidgetName());

    // Create the mapper and actor
    Volume = vtkVolume::New();
    VolumeMapper = vtkCudaVolumeMapper::New();
    Volume->SetMapper(VolumeMapper);

    LoadHead();
    clippers.push_back(vtkImageClip::New());
    clippers[0]->SetInput(readers[0]->GetOutput());
    clippers[0]->Update();
    if (!readers.empty())
        VolumeMapper->SetInput(clippers[0]->GetOutput());
    SetClipRatio(200);

    vtkVolumeProperty* prop = vtkVolumeProperty::New();
    Volume->SetProperty(prop);
    renderWidget->GetRenderer()->AddVolume(Volume);


    /// GUI EVENT
    vtkCallbackCommand* GUICallbackCommand = vtkCallbackCommand::New ( );
    GUICallbackCommand->SetCallback( UpdateRenderer);

    vtkCallbackCommand* StartCallbackCommand = vtkCallbackCommand::New ( );
    StartCallbackCommand->SetCallback( StartRender );

    vtkCallbackCommand* AnimCallbackCommand = vtkCallbackCommand::New ( );
    AnimCallbackCommand->SetCallback( Animate);
    vtkCallbackCommand* ModelCallbackCommand = vtkCallbackCommand::New();
    ModelCallbackCommand->SetCallback( ChangeModel );

    vtkCallbackCommand* MapperCallbackCommand = vtkCallbackCommand::New();
    MapperCallbackCommand->SetCallback( ChangeMapper );

    /// SETUP THE GUI
    mb_Model = vtkKWMenuButton::New();
    mb_Model->SetParent(win->GetMainPanelFrame());
    mb_Model->Create();
    mb_Model->GetMenu()->AddRadioButton("Heart");
    mb_Model->GetMenu()->AddRadioButton("Head");
    mb_Model->GetMenu()->AddRadioButton("Lung");
    mb_Model->GetMenu()->AddRadioButton("Bunny");
    mb_Model->GetMenu()->AddRadioButton("Prostate");
    mb_Model->GetMenu()->AddRadioButton("Ultrasound Heart");
    
    mb_Model->GetMenu()->AddRadioButton("Empty");
    mb_Model->SetValue("Head");
    mb_Model->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)ModelCallbackCommand);

    app->Script("pack %s -side top -anchor nw -expand n -fill x -pady 2",
        mb_Model->GetWidgetName()); 


    mb_Mapper = vtkKWMenuButton::New();
    mb_Mapper->SetParent(win->GetMainPanelFrame());
    mb_Mapper->Create();
    mb_Mapper->GetMenu()->AddRadioButton("Software_Ray_Caster");
    mb_Mapper->GetMenu()->AddRadioButton("Texture_2D");
    mb_Mapper->GetMenu()->AddRadioButton("Texture_3D");
    mb_Mapper->GetMenu()->AddRadioButton("CUDA");
    mb_Mapper->SetValue("CUDA");
    mb_Mapper->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)MapperCallbackCommand);

    app->Script("pack %s -side top -anchor nw -expand n -fill x -pady 2",
        mb_Mapper->GetWidgetName()); 


    //Volume Property
    VolumePropertyWidget = vtkKWVolumePropertyWidget::New();
    VolumePropertyWidget->SetParent(win->GetMainPanelFrame());
    VolumePropertyWidget->Create();
    app->Script( "pack %s -side top -anchor nw -expand n -fill x -pady 2",
        VolumePropertyWidget->GetWidgetName()); 
    VolumePropertyWidget->AddObserver(vtkKWEvent::VolumePropertyChangedEvent, (vtkCommand*)GUICallbackCommand);

    VolumePropertyWidget->SetVolumeProperty(prop);
    prop->Delete();

    ThresholdRange = vtkKWRange::New();
    ThresholdRange->SetParent(win->GetMainPanelFrame());
    ThresholdRange->Create();
    ThresholdRange->SetWholeRange(0, 255);
    ThresholdRange->SetRange(90, 255);
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        ThresholdRange->GetWidgetName()); 
    ThresholdRange->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand*)GUICallbackCommand);

    SteppingSizeScale = vtkKWScale::New();
    SteppingSizeScale->SetParent(win->GetMainPanelFrame());
    SteppingSizeScale->Create();
    SteppingSizeScale->SetRange(0.1f, 3.0f);
    SteppingSizeScale->SetResolution(.1f);
    SteppingSizeScale->SetValue(1.0f);
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        SteppingSizeScale->GetWidgetName()); 
    SteppingSizeScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand*)GUICallbackCommand);


    ScaleFactorScale = vtkKWScale::New();
    ScaleFactorScale->SetParent(win->GetMainPanelFrame());
    ScaleFactorScale->Create();
    ScaleFactorScale->SetRange(1.0f, 10.0f);
    ScaleFactorScale->SetResolution(.5f);
    ScaleFactorScale->SetValue(1.0f);
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        ScaleFactorScale->GetWidgetName()); 
    ScaleFactorScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand*)GUICallbackCommand);


    cb_Animate = vtkKWCheckButton::New();
    cb_Animate->SetParent(win->GetMainPanelFrame());
    cb_Animate->Create();
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        cb_Animate->GetWidgetName());
    cb_Animate->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)GUICallbackCommand);

    cb_Animate2 = vtkKWCheckButton::New();
    cb_Animate2->SetParent(win->GetMainPanelFrame());
    cb_Animate2->Create();
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        cb_Animate2->GetWidgetName());
    cb_Animate2->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)GUICallbackCommand);


    renderWidget->GetRenderer()->GetActiveCamera()->SetPosition(500, 500, 500);
    renderWidget->GetRenderer()->GetActiveCamera()->SetClippingRange(100, 1000);
    renderWidget->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();

    vtkInteractorStyle* interactorStyle = vtkInteractorStyleTrackballCamera::New();
    renderWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
    renderWidget->SetRenderModeToInteractive();

    renderWidget->GetRenderWindow()->AddObserver(vtkCommand::StartEvent,  (vtkCommand*)StartCallbackCommand);
    renderWidget->GetRenderWindow()->AddObserver(vtkCommand::EndEvent,  (vtkCommand*)AnimCallbackCommand);
    int ret = 0;
    win->Display();
    if (!option_test)
    {
        app->Start(argc, argv);
        ret = app->GetExitStatus();
    }
    renderWidget->GetRenderer()->RemoveVolume(Volume);
    win->Close();

    interactorStyle->Delete();
    // Deallocate and exit
    SteppingSizeScale->Delete();
    ScaleFactorScale->Delete();
    MapperCallbackCommand->Delete();
    ModelCallbackCommand->Delete();
    AnimCallbackCommand->Delete();
    GUICallbackCommand->Delete();
    //    filter->Delete();
    mb_Model->Delete();
    mb_Mapper->Delete();

    Volume->Delete();
    Clear();
    for (unsigned int i = 0; i < clippers.size(); i++)
        clippers[i]->Delete();
    VolumeMapper->Delete();

    VolumePropertyWidget->Delete();
    cb_Animate->Delete();
    ThresholdRange->Delete();
    renderWidget->Delete();
    win->Delete();
    app->Delete();

    return ret;
}

/*#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
int argc;
char **argv;
vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments(
lpCmdLine, &argc, &argv);
int ret = my_main(argc, argv);
for (int i = 0; i < argc; i++) { delete [] argv[i]; }
delete [] argv;
return ret;
}
#else */
int main(int argc, char *argv[])
{
    return my_main(argc, argv);
}
//#endif
