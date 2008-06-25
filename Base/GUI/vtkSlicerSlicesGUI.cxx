#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerSlicesGUI.h"
//#include "vtkSlicerSliceGUICollection.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include <map>

// Private implementaton of an std::map
class SliceGUIMap : public std::map<std::string, vtkSmartPointer<vtkSlicerSliceGUI> > {};


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesGUI);
vtkCxxRevisionMacro(vtkSlicerSlicesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesGUI::vtkSlicerSlicesGUI (  )
{
  this->InternalSliceGUIMap = new SliceGUIMap;
}


//---------------------------------------------------------------------------
vtkSlicerSlicesGUI::~vtkSlicerSlicesGUI ( )
{
  if (this->InternalSliceGUIMap)
    {
    delete this->InternalSliceGUIMap;
    }
}

void vtkSlicerSlicesGUI::AddSliceGUI(char *layoutName, vtkSlicerSliceGUI *pSliceGUI)
{
  std::string sMRMLNodeLayoutName = layoutName;
  (*this->InternalSliceGUIMap)[sMRMLNodeLayoutName] = pSliceGUI;
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::AddAndObserveSliceGUI ( char *layoutName, vtkSlicerSliceGUI *pSliceGUI )
{
  this->AddSliceGUI ( layoutName, pSliceGUI );
  pSliceGUI->AddGUIObservers ( );
}

vtkSlicerSliceGUI* vtkSlicerSlicesGUI::GetSliceGUI(char *layoutName)
{
  if (this->InternalSliceGUIMap)
    {
    SliceGUIMap::const_iterator gend = (*this->InternalSliceGUIMap).end();
    SliceGUIMap::const_iterator git =       (*this->InternalSliceGUIMap).find(layoutName);
    
    if ( git != gend)
      return (vtkSlicerSliceGUI::SafeDownCast((*git).second));
    else
      return NULL;
    }
  else
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::AddGUIObservers ( )
{
  // TODO: add observers for all widgets in slicesGUIs UIpanel.
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::RemoveGUIObservers ( )
{
        SliceGUIMap::const_iterator git;
        for (git = this->InternalSliceGUIMap->begin();
                git != this->InternalSliceGUIMap->end(); ++git)
        {
                vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast((*git).second);
                g->RemoveGUIObservers();
        }
}

void vtkSlicerSlicesGUI::RemoveGUIMapObservers()
{
        SliceGUIMap::const_iterator git;
        for (git = this->InternalSliceGUIMap->begin();
                git != this->InternalSliceGUIMap->end(); ++git)
        {
                vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast((*git).second);
                g->RemoveGUIObservers();
        }
}

vtkSlicerSliceGUI* vtkSlicerSlicesGUI::GetFirstSliceGUI ()
{
        if (this->InternalSliceGUIMap)
        {
                SliceGUIMap::const_iterator git;
                git = this->InternalSliceGUIMap->begin();
                return (vtkSlicerSliceGUI::SafeDownCast((*git).second));
        }
        else
                return NULL;
}

char* vtkSlicerSlicesGUI::GetFirstSliceGUILayoutName()
{
        if (this->InternalSliceGUIMap)
        {
                SliceGUIMap::const_iterator git;
                git = this->InternalSliceGUIMap->begin();
                return ((char *)(*git).first.c_str());
        }
        else
                return NULL;
}

int vtkSlicerSlicesGUI::GetNumberOfSliceGUI()
{ 
        if (this->InternalSliceGUIMap)
                return (this->InternalSliceGUIMap->size());
        else
                return -1;
}

vtkSlicerSliceGUI* vtkSlicerSlicesGUI::GetNextSliceGUI(char *layoutName)
{
        if (this->InternalSliceGUIMap)
        {
                SliceGUIMap::const_iterator gend = (*this->InternalSliceGUIMap).end();
                SliceGUIMap::const_iterator git =       (*this->InternalSliceGUIMap).find(layoutName);
                git++;

                if ( git != gend)
                        return (vtkSlicerSliceGUI::SafeDownCast((*git).second));
                else
                        return NULL;
        }
        else
                return NULL;
}

char* vtkSlicerSlicesGUI::GetNextSliceGUILayoutName(char *layoutName)
{
        if (this->InternalSliceGUIMap)
        {
                SliceGUIMap::const_iterator gend = (*this->InternalSliceGUIMap).end();
                SliceGUIMap::const_iterator git =       (*this->InternalSliceGUIMap).find(layoutName);
                git++;

                if ( git != gend)
                        return ((char *)(*git).first.c_str());
                else
                        return NULL;
        }
        else
                return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event, void *callData)
    {
        // Fill in
    }


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
}
 

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}
 

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::BuildGUI (  )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Slices", "Slices", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Slices Module manages the display of the Slice Viewers.";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Slices" );
    this->BuildHelpAndAboutFrame ( page, help, about );

    // ---
    // DISPLAY FRAME            
    vtkSlicerModuleCollapsibleFrame *sliceDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    sliceDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Slices" ) );
    sliceDisplayFrame->Create ( );
    sliceDisplayFrame->SetLabelText ("Slice information");
    sliceDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  sliceDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Slices")->GetWidgetName());

    sliceDisplayFrame->Delete();
}


//----------------------------------------------------------------------------
void vtkSlicerSlicesGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "SlicerSlicesGUI:" << this->GetClassName ( ) << "\n";
  os << indent << "SliceGUIMap: " << this->InternalSliceGUIMap << endl;
}
