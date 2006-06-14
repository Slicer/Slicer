#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkObjectFactory.h"
#include "vtkKWInternationalization.h"
#include "vtkKWWidget.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerApplicationSettingsInterface );
vtkCxxRevisionMacro(vtkSlicerApplicationSettingsInterface, "$Revision: 1.0 $");

vtkSlicerApplicationSettingsInterface::vtkSlicerApplicationSettingsInterface()
{
  this->ModuleSettingsFrame = 0;
  this->ModulePathEntry = 0;
}


vtkSlicerApplicationSettingsInterface::~vtkSlicerApplicationSettingsInterface()
{
  if (this->ModuleSettingsFrame)
    {
    this->ModuleSettingsFrame->Delete();
    this->ModuleSettingsFrame = 0;
    }

  if (this->ModulePathEntry)
    {
    this->ModulePathEntry->Delete();
    this->ModulePathEntry = 0;
    }
}


// ---------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::Create()
{
  if (this->IsCreated())
    {
    vtkErrorMacro("The panel is already created.");
    return;
    }

  // Create the superclass instance (and set the application)

  this->Superclass::Create();

  ostrstream tk_cmd;
  vtkKWWidget *page;
  vtkKWFrame *frame;

  // --------------------------------------------------------------
  // Add a "Preferences" page

  this->AddPage(this->GetName());
  page = this->GetPageWidget(this->GetName());

  // --------------------------------------------------------------
  // Interface settings : main frame

  if (!this->ModuleSettingsFrame)
    {
    this->ModuleSettingsFrame = vtkKWFrameWithLabel::New();
    }

  this->ModuleSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->ModuleSettingsFrame->Create();
  this->ModuleSettingsFrame->SetLabelText(
    ks_("Application Settings|Module Settings"));
    
  tk_cmd << "pack " << this->ModuleSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->ModuleSettingsFrame->GetFrame();

  // --------------------------------------------------------------
  // Module settings : Module Path

  if (!this->ModulePathEntry)
    {
    this->ModulePathEntry = vtkKWEntryWithLabel::New();
    }

  this->ModulePathEntry->SetParent(frame);
  this->ModulePathEntry->Create();
  this->ModulePathEntry->SetLabelText(
    ks_("Module Settings|Module Path"));
  this->ModulePathEntry->GetWidget()->SetCommand(this, "ModulePathCallback");
  this->ModulePathEntry->SetBalloonHelpString(
    k_("Search path for modules."));

  tk_cmd << "pack " << this->ModulePathEntry->GetWidgetName()
         << "  -side top -anchor w -expand no -fill x" << endl;


  // --------------------------------------------------------------
  // Pack 

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);

  // Update

  this->Update();
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ModulePathCallback(char *path)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetModulePath(path);
    }
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::Update()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Pull values from the application object and put them in the
    // settings interface widgets
    if (this->ModulePathEntry)
      {
      this->ModulePathEntry->GetWidget()->SetValue(app->GetModulePath());
      }
    }
}
