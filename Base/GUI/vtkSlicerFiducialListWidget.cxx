#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerFiducialListWidget.h"

#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

#include "vtkActor.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkTransform.h"
#include "vtkPolyData.h"
#include "vtkLookupTable.h"
#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkMapper.h"
#include "vtkVectorText.h"
#include "vtkRenderer.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkSlicerGlyphSource2D.h"

#include "vtkMRMLFiducialListNode.h"


#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"

#include "vtkCollection.h"

#include "vtkSphereSource.h"

#include "vtkTransformPolyDataFilter.h"
#include "vtkGlyph3D.h"
#include "vtkPointWidget.h"

// for pick events
#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerViewerInteractorStyle.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"

class vtkPointWidgetCallback : public vtkCommand
{
public:
  static vtkPointWidgetCallback *New()
  { return new vtkPointWidgetCallback; }
  virtual void Execute(vtkObject *caller, unsigned long event, void*)
  {
    vtkPointWidget *pointWidget = reinterpret_cast<vtkPointWidget*>(caller);
    if (pointWidget)
      {
      double x[3];
      pointWidget->GetPosition(x);
      // now update the fiducial
      if (this->FiducialList)
        {
        this->FiducialList->SetNthFiducialXYZ(this->FiducialIndex, x[0], x[1], x[2]);
        }
      }
  }
  vtkPointWidgetCallback():FiducialList(0) {}
  vtkMRMLFiducialListNode *FiducialList;
  int FiducialIndex;
};

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFiducialListWidget );
vtkCxxRevisionMacro ( vtkSlicerFiducialListWidget, "$Revision: $");

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget::vtkSlicerFiducialListWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::Constructor");
  
  this->MainViewer = NULL;
  this->ProcessingMRMLEvent = 0;
  this->RenderPending = 0;
  
  this->DiamondTransformMap.clear();

//  this->Glyph3DList = vtkCollection::New();
  
  // Create the 3d diamond glyphs
  vtkPoints * diamondGlyphPoints = vtkPoints::New();
  diamondGlyphPoints->SetNumberOfPoints(6);
  diamondGlyphPoints->InsertPoint(0, 1, 0, 0);
  diamondGlyphPoints->InsertPoint(1, 0, 1, 0);
  diamondGlyphPoints->InsertPoint(2, 0, 0, 1);
  diamondGlyphPoints->InsertPoint(3, -1, 0, 0);
  diamondGlyphPoints->InsertPoint(4, 0, -1, 0);
  diamondGlyphPoints->InsertPoint(5, 0, 0, -1);

  vtkCellArray * diamondGlyphPolys = vtkCellArray::New();
  diamondGlyphPolys->InsertNextCell( 4 );
  diamondGlyphPolys->InsertCellPoint(0);
  diamondGlyphPolys->InsertCellPoint(1);
  diamondGlyphPolys->InsertCellPoint(3);
  diamondGlyphPolys->InsertCellPoint(4);
  
  diamondGlyphPolys->InsertNextCell(4);
  diamondGlyphPolys->InsertCellPoint(1);
  diamondGlyphPolys->InsertCellPoint(2);
  diamondGlyphPolys->InsertCellPoint(4);
  diamondGlyphPolys->InsertCellPoint(5);

  diamondGlyphPolys->InsertNextCell(4);
  diamondGlyphPolys->InsertCellPoint(2);
  diamondGlyphPolys->InsertCellPoint(0);
  diamondGlyphPolys->InsertCellPoint(5);
  diamondGlyphPolys->InsertCellPoint(3);

  vtkCellArray * diamondGlyphLines = vtkCellArray::New(); 
          
  diamondGlyphLines->InsertNextCell(2);
  diamondGlyphLines->InsertCellPoint(0);
  diamondGlyphLines->InsertCellPoint(3);

  diamondGlyphLines->InsertNextCell(2);
  diamondGlyphLines->InsertCellPoint(1);
  diamondGlyphLines->InsertCellPoint(4);

  diamondGlyphLines->InsertNextCell(2);                                         
  diamondGlyphLines->InsertCellPoint(2);
  diamondGlyphLines->InsertCellPoint(5);

  this->DiamondGlyphPolyData = vtkPolyData::New();
  this->DiamondGlyphPolyData->SetPoints(diamondGlyphPoints);
  diamondGlyphPoints->Delete();
  this->DiamondGlyphPolyData->SetPolys(diamondGlyphPolys);
  this->DiamondGlyphPolyData->SetLines(diamondGlyphLines);
  diamondGlyphPolys->Delete();
  diamondGlyphLines->Delete();

  this->SphereSource = vtkSphereSource::New();
  this->SphereSource->SetRadius(0.3);
  this->SphereSource->SetPhiResolution(10);
  this->SphereSource->SetThetaResolution(10);

  // for picking
  this->ViewerWidget = NULL;
  this->InteractorStyle = NULL;
  
//  this->DebugOn();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveMRMLObservers()
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveMRMLObservers\n");
  this->RemoveFiducialObservers();
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget::~vtkSlicerFiducialListWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::Destructor\n");

  // let go of the pointer to the main viewer
  this->SetMainViewer(NULL);

  this->RemoveMRMLObservers();

  this->DiamondGlyphPolyData->Delete();
  this->DiamondGlyphPolyData = NULL;
  this->SphereSource->Delete();
  this->SphereSource = NULL;

  vtkDebugMacro("\tDeleting " << this->DisplayedFiducials.size() << " fiducial actors...");
  std::map< std::string, vtkActor *>::iterator actorIter;
  for(actorIter = this->DisplayedFiducials.begin();
      actorIter != this->DisplayedFiducials.end();
      actorIter++) 
    {
    if (actorIter->second != NULL)
      {
      if (vtkFollower::SafeDownCast(actorIter->second))
        {
        vtkFollower::SafeDownCast(actorIter->second)->SetCamera(NULL);
        }
      actorIter->second->Delete();
      }
    }
  this->DisplayedFiducials.clear();

  std::map< std::string, vtkFollower *>::iterator fIter;
  for(fIter = this->DisplayedTextFiducials.begin();
      fIter != this->DisplayedTextFiducials.end();
      fIter++) 
    {
    if (fIter->second != NULL)
      {
      fIter->second->SetCamera(NULL);
      fIter->second->SetMapper(NULL);
      fIter->second->Delete();
      }
    }

  std::map< std::string, vtkPointWidget*>::iterator pointIter;
  for (pointIter = this->DisplayedPointWidgets.begin();
       pointIter != this->DisplayedPointWidgets.end();
       pointIter++)
    {
      if (pointIter->second != NULL)
    {
        vtkDebugMacro("Deleting displayed point widget at id " << pointIter->first.c_str());
        pointIter->second->RemoveObservers(vtkCommand::EnableEvent);
          pointIter->second->RemoveObservers(vtkCommand::StartInteractionEvent);
          pointIter->second->RemoveObservers(vtkCommand::InteractionEvent);
          pointIter->second->EnabledOff();
          pointIter->second->SetInteractor(NULL);
      pointIter->second->Delete();
    }
    }

  std::map< std::string, vtkTransform * >::iterator transformIter;
  for (transformIter=this->DiamondTransformMap.begin();
       transformIter != this->DiamondTransformMap.end();
       transformIter++)
    {      
    if (transformIter->second != NULL)
      {
      transformIter->second->Delete();
      }
    }
  this->DiamondTransformMap.clear();

  std::map< std::string, vtkPoints * >::iterator gpIter;
  for (gpIter=  this->GlyphPointsMap.begin();
       gpIter != this->GlyphPointsMap.end();
       gpIter++)
    {
    if (gpIter->second != NULL)
      {
      gpIter->second->Delete();
      }
    }
  this->GlyphPointsMap.clear();
    
  std::map< std::string, vtkFloatArray * >::iterator gsIter;
  for (gsIter = this->GlyphScalarsMap.begin();
       gsIter != this->GlyphScalarsMap.end();
       gsIter++)
    {
    if (gsIter->second != NULL)
      {
      gsIter->second->Delete();
      }
    }
  this->GlyphScalarsMap.clear();

  std::map< std::string, vtkPolyData * >::iterator pdIter;
  for (pdIter = this->GlyphPolyDataMap.begin();
       pdIter != this->GlyphPolyDataMap.end();
       pdIter++)
    {
    if (pdIter->second != NULL)
      {
      pdIter->second->Delete();
      }
    }
  
  for (transformIter=this->TextTransformMap.begin();
       transformIter != this->TextTransformMap.end();
       transformIter++) 
    {
    if (transformIter->second != NULL)
      {
      transformIter->second->Delete();
      }
    }
  this->TextTransformMap.clear();
  
  for (transformIter=this->SymbolTransformMap.begin();
       transformIter != this->SymbolTransformMap.end();
       transformIter++)
    {
    if (transformIter->second != NULL)
      {
      transformIter->second->Delete();
      }
    }
  this->SymbolTransformMap.clear();

  std::map< std::string, vtkTransformPolyDataFilter * >::iterator tfIter;
  for (tfIter = this->TransformFilterMap.begin();
       tfIter != this->TransformFilterMap.end();
       tfIter++)
    {
    if (tfIter->second != NULL)
      {
      tfIter->second->Delete();
      }
    }
  this->TransformFilterMap.clear();

  std::map< std::string, vtkMapper * >::iterator gmIter;
  for (gmIter = this->GlyphMapperMap.begin();
       gmIter != this->GlyphMapperMap.end();
       gmIter++)
    {
    if (gmIter->second != NULL)
      {
      gmIter->second->Delete();
      }
    }
  this->GlyphMapperMap.clear();
  
  /*
    if (this->Glyph3DList)
    {
    this->Glyph3DList->RemoveAllItems();
    this->Glyph3DList->Delete();
    }
  */
  std::map< std::string, vtkGlyph3D * >::iterator g3dIter;
  for (g3dIter = this->Glyph3DMap.begin();
       g3dIter != this->Glyph3DMap.end();
       g3dIter++)
    {
    if (g3dIter->second != NULL)
      {
      g3dIter->second->Delete();
      }
    }
  this->Glyph3DMap.clear();

  this->Use3DSymbolsMap.clear();
  
  this->SetViewerWidget(NULL);
  this->SetInteractorStyle(NULL);
}
//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerFiducialListWidget: " << this->GetClassName ( ) << "\n";
    
    vtkIndent nextIndent;
    nextIndent = indent.GetNextIndent();
    if (this->GetMainViewer() != NULL)
      {
      os << indent << "Main Viewer:\n";
      this->GetMainViewer()->PrintSelf(os, nextIndent);
      }
    
    std::map< std::string, vtkActor *>::iterator iter;
    for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
      {
      os << indent << "Actor " << iter->first << "\n";
      if (iter->second != NULL)
        {
        iter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << indent << "Symbol vtkActor is null\n";
        }
      }

    std::map< std::string, vtkFollower *>::iterator titer;
    for(titer=this->DisplayedTextFiducials.begin(); titer != this->DisplayedTextFiducials.end(); titer++) 
      {
      os << indent << "Text Actor " << titer->first << "\n";
      if (titer->second != NULL)
        {
        titer->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << indent << "Text vtkActor is null\n";
        }
      }

    std::map< std::string, vtkPointWidget *>::iterator pointIter;
    for(pointIter=this->DisplayedPointWidgets.begin(); pointIter != this->DisplayedPointWidgets.end(); pointIter++) 
      {
      os << indent << "Point Widget " << pointIter->first << "\n";
      if (pointIter->second != NULL)
        {
        pointIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << indent << "Point widget is null\n";
        }
      }


    os << indent << "Maps:\n";
    os << indent << "DiamondTransformMap: size = " << DiamondTransformMap.size() << "\n";
    std::map< std::string, vtkTransform * >::iterator transformIter;
    for (transformIter=this->DiamondTransformMap.begin();
         transformIter != this->DiamondTransformMap.end();
         transformIter++)
      {      
      if (transformIter->second != NULL)
        {
        transformIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }

    os << indent << "GlyphPointsMap: size = " << this->GlyphPointsMap.size() << "\n";
    std::map< std::string, vtkPoints * >::iterator gpIter;
    for (gpIter=  this->GlyphPointsMap.begin();
         gpIter != this->GlyphPointsMap.end();
         gpIter++)
      {
      if (gpIter->second != NULL)
        {
        gpIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }

    os << indent << "GlyphScalarsMap: size = " << this->GlyphScalarsMap.size() << "\n";
    std::map< std::string, vtkFloatArray * >::iterator gsIter;
    for (gsIter = this->GlyphScalarsMap.begin();
         gsIter != this->GlyphScalarsMap.end();
         gsIter++)
      {
      if (gsIter->second != NULL)
        {
        gsIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }
    
    os << indent << "GlyphPolyDataMap: size = " << this->GlyphPolyDataMap.size() << "\n";
    std::map< std::string, vtkPolyData * >::iterator pdIter;
    for (pdIter = this->GlyphPolyDataMap.begin();
         pdIter != this->GlyphPolyDataMap.end();
         pdIter++)
      {
      if (pdIter->second != NULL)
        {
        pdIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }
    
    os << indent << "TextTransformMap: size = " << this->TextTransformMap.size() << "\n";
    for (transformIter=this->TextTransformMap.begin();
         transformIter != this->TextTransformMap.end();
         transformIter++) 
      {
      if (transformIter->second != NULL)
        {
        transformIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }
    
    os << indent << "SymbolTransformMap: size = " << this->SymbolTransformMap.size() << "\n";
    for (transformIter=this->SymbolTransformMap.begin();
         transformIter != this->SymbolTransformMap.end();
         transformIter++)
      {
      if (transformIter->second != NULL)
        {
        transformIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }      
      }
    
    os << indent << "TransformFilterMap: size = " << this->TransformFilterMap.size() << "\n";
    std::map< std::string, vtkTransformPolyDataFilter * >::iterator tfIter;
    for (tfIter = this->TransformFilterMap.begin();
         tfIter != this->TransformFilterMap.end();
         tfIter++)
      {
      if (tfIter->second != NULL)
        {
        tfIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }
    /*
    os << indent << "Glyph3DMap: size = " << this->Glyph3DMap.size() << "\n";
    for (i = 0; i < this->Glyph3DMap.size(); i++)
      {
      if (this->Glyph3DMap[i] != NULL)
        {
        this->Glyph3DMap[i]->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }
      }
    os << indent << "Glyph3DList: size = " << this->Glyph3DList->GetNumberOfItems() << "\n";

    int i;
    for (i = 0; i < this->Glyph3DList->GetNumberOfItems(); i++)
      {
      ((vtkGlyph3D*)this->Glyph3DList->GetItemAsObject(i))->PrintSelf(os,nextIndent);
      }
    */
    os << indent << "Glyph3DMap: size = " << this->Glyph3DMap.size() << "\n";
    std::map< std::string, vtkGlyph3D * >::iterator g3dIter;
    for (g3dIter = this->Glyph3DMap.begin();
         g3dIter != this->Glyph3DMap.end();
         g3dIter++)
      {
      if (g3dIter->second != NULL)
        {
        g3dIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }  
      }
    
    os << indent << "GlyphMapperMap: size = " << this->GlyphMapperMap.size() << "\n";
    std::map< std::string, vtkMapper * >::iterator gmIter;
    for (gmIter = this->GlyphMapperMap.begin();
         gmIter != this->GlyphMapperMap.end();
         gmIter++)
      {
      if (gmIter->second != NULL)
        {
        gmIter->second->PrintSelf(os, nextIndent);
        }
      else
        {
        os << nextIndent << "NULL\n";
        }      
      }

    os << indent << "Use3DSymbolsMap: size = " << this->Use3DSymbolsMap.size() << "\n";    
    std::map< std::string, bool>::iterator boolIter;
    for (boolIter = this->Use3DSymbolsMap.begin();
         boolIter != this->Use3DSymbolsMap.end();
         boolIter++)
      {
      os << nextIndent << boolIter->first.c_str() << " = " << boolIter->second << "\n";
      }
    
    os << indent << "DisplayedPointWidgets: size = " << this->DisplayedPointWidgets.size() << "\n";
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::ProcessWidgetEvents: event = " << event);
  // check for pick events
  if (event == vtkSlicerViewerInteractorStyle::PickEvent &&
      vtkSlicerViewerInteractorStyle::SafeDownCast(caller) != NULL &&
      callData != NULL)
    {
    vtkDebugMacro("FiducialListWidget: Pick event!\n");
    // do the pick
    int x = ((int *)callData)[0];
    int y = ((int *)callData)[1];
    if (this->GetViewerWidget()->Pick(x,y) != 0)
      {
      // check for a valid RAS point
      double *rasPoint = this->GetViewerWidget()->GetPickedRAS();
      if (rasPoint != NULL &&
          vtkSlicerFiducialsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Fiducials")) != NULL )
        {
        vtkSlicerFiducialsLogic *fidLogic  = vtkSlicerFiducialsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Fiducials"))->GetLogic();
        int modelIndex = fidLogic->AddFiducialSelected(rasPoint[0], rasPoint[1], rasPoint[2], 1);
        // swallow the event
        vtkDebugMacro("Fiducial List Widget dealt with the Pick, added fiducial at index " << modelIndex << ", setting my gui call back command abort flag so that the interactor style will stop passing the event along, event = " << event);
        if (this->GUICallbackCommand != NULL)
          {
            
          this->GUICallbackCommand->SetAbortFlag(1);
          }
        else
          {
          vtkErrorMacro("Unable to get the gui call back command that calls process widget events, event = " << event << " is not swallowed here");
          }
        }
      }
    }
} 

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::ProcessMRMLEvents: processing = " << this->ProcessingMRMLEvent << ", event = " << event);
  
  if (this->ProcessingMRMLEvent != 0 )
    {
    vtkDebugMacro("Returning because already processing an event, " << this->ProcessingMRMLEvent);
    return;
    }

  this->ProcessingMRMLEvent = event;

  // if it's a general fid display or point modified event, or it's a modified
  // event on a fid list, update
  
  if (event == vtkMRMLFiducialListNode::DisplayModifiedEvent ||
      event == vtkMRMLFiducialListNode::FiducialModifiedEvent ||
      (vtkMRMLFiducialListNode::SafeDownCast(caller) != NULL && event == vtkCommand::ModifiedEvent) ||
      (vtkMRMLScene::SafeDownCast(caller) != NULL && 
      (event == vtkMRMLScene::NodeAddedEvent && vtkMRMLFiducialListNode::SafeDownCast((vtkObjectBase *)callData) != NULL ||
      event == vtkMRMLScene::NodeRemovedEvent && vtkMRMLFiducialListNode::SafeDownCast((vtkObjectBase *)callData) != NULL ||
      event == vtkMRMLScene::SceneCloseEvent)) ) 
      //||
      //event == vtkMRMLScene::NewSceneEvent )) )

    {
    // could have finer grain control by calling remove fid props and then
    // update fids from mrml if necessary
    vtkDebugMacro("ProcessMRMLEvents: got a relevant event " << event << ", calling update from mrml");  
    this->UpdateFromMRML();
    }  
     
  // if the list transfrom was updated...
  if (event == vtkMRMLTransformableNode::TransformModifiedEvent &&
      (vtkMRMLFiducialListNode::SafeDownCast(caller) != NULL))
    {
    vtkDebugMacro("Got transform modified event, calling update from mrml");
    this->UpdateFromMRML();
    } 
  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::CreateWidget ( )
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::CreateWidget...\n");
  
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateFromMRML()
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::UpdateFromMRML: calling RemoveFiducialProps and then UpdateFiducialsFromMRML");
  this->RemoveFiducialProps ( );
  this->UpdateFiducialsFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::AddList(vtkMRMLFiducialListNode *flist)
{
  if (flist == NULL)
    {
    return;
    }
  
  vtkDebugMacro("AddList: starting...");

  const char* fid = flist->GetID();
  
  float textScale = flist->GetTextScale();
  float symbolScale = flist->GetSymbolScale();
  int glyphType = flist->GetGlyphType();

  // are all the current vectors the same size?
    
  // where in RAS space will the fiducials be displayed?
  vtkPoints * glyphPoints = vtkPoints::New();
  glyphPoints->Initialize();
  this->GlyphPointsMap[fid] = glyphPoints;
  // the map doesn't remember the pointer
  //glyphPoints->Delete();
  
  // the scalar array is used to determine the colour of the fiducial,
  // selected or not
  vtkFloatArray * glyphScalars = vtkFloatArray::New();
  this->GlyphScalarsMap[fid] = glyphScalars;
  //glyphScalars->Delete();

  vtkDebugMacro("...added the new scalars...");
  
  // points and scalars are encapsulated in the poly data
  vtkPolyData * glyphPolyData = vtkPolyData::New();
  this->GlyphPolyDataMap[fid] = glyphPolyData;
//  glyphPolyData->Delete();

  vtkDebugMacro("...added the new glyph poly data...");
  
  vtkDebugMacro("...using n = " << fid << ", poly data size = " << this->GlyphPolyDataMap.size() << ", glyph points vector size = " <<  this->GlyphPointsMap.size() << ", glyph scalars vector size = " << this->GlyphScalarsMap.size() );
  
  this->GlyphPolyDataMap[fid]->SetPoints(this->GlyphPointsMap[fid]);
  this->GlyphPolyDataMap[fid]->GetPointData()->SetScalars(this->GlyphScalarsMap[fid]);

  vtkDebugMacro("...set the points, and the scalars...");
  
  // the default size for the text
  vtkTransform *textTransform = vtkTransform::New();
  textTransform->AddObserver(vtkCommand::WarningEvent, this->MRMLCallbackCommand );
  int textPush = 10;
  textTransform->Translate(0, 0, textPush);
  textTransform->GetMatrix()->SetElement(0, 1, 0.333);
  textTransform->Scale(textScale, textScale, 1);
  this->TextTransformMap[fid] = textTransform;
//  textTransform->Delete();
  
  vtkDebugMacro("...added the new text transform...");

  // default size for symbols
  vtkTransform *symbolTransform = vtkTransform::New();
  symbolTransform->AddObserver(vtkCommand::WarningEvent, this->MRMLCallbackCommand );
  symbolTransform->Scale(symbolScale, symbolScale, symbolScale);
  this->SymbolTransformMap[fid] = symbolTransform;
//  symbolTransform->Delete();

  vtkDebugMacro("...added the new symbol transform..");
  
  // set up the shape of the glyph
  vtkTransformPolyDataFilter * transformFilter = vtkTransformPolyDataFilter::New();
  // use the shape built in the constructor
  if (glyphType == vtkMRMLFiducialListNode::Diamond3D)
    {
    transformFilter->SetInput(this->DiamondGlyphPolyData);
    }
  else
    {
//    vtkDebugMacro("Using the sphere source\n");
    transformFilter->SetInput(this->SphereSource->GetOutput());
    }
  transformFilter->SetTransform(this->SymbolTransformMap[fid]);
  this->TransformFilterMap[fid] = transformFilter;
//  transformFilter->Delete();

  vtkDebugMacro("...added the new transform filter...");
  
  // now set up the glyph
  vtkGlyph3D *glyph3D = vtkGlyph3D::New();
  glyph3D->SetSource(this->TransformFilterMap[fid]->GetOutput());
  glyph3D->SetInput(this->GlyphPolyDataMap[fid]);
  glyph3D->SetScaleFactor(1.0);
  glyph3D->ClampingOn();
  glyph3D->ScalingOff();
  glyph3D->SetRange(0, 1);
  this->Glyph3DMap[fid] = glyph3D;
  //this->Glyph3DList->vtkCollection::AddItem(glyph3D);
  //glyph3D->Delete();

  vtkDebugMacro("...added the new glyph...");
  
  // now set up the mapper
  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(this->Glyph3DMap[fid]->GetOutput());
  //vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper->GetLookupTable())%;
  vtkLookupTable::SafeDownCast(mapper->GetLookupTable())->SetNumberOfTableValues(2);
  // set the selected/unselected colours
  vtkLookupTable::SafeDownCast(mapper->GetLookupTable())->SetTableValue(0, 1, 0, 0, 1.0);
  vtkLookupTable::SafeDownCast(mapper->GetLookupTable())->SetTableValue(1, 0, 0, 1, 1.0);
  
  this->GlyphMapperMap[fid] = mapper;
//  lut->Delete();
//  mapper->Delete();

  vtkDebugMacro("...added the new mapper at fid " << fid);
  
  // set up the list's transform?
  
  /*
  
    this->DiamondGlyphPolyDataMap->SetPoints(glyphPoints);
    vtkFloatArray * glyphScalars = vtkFloatArray::New();
    this->DiamondGlyphPolyDataMap->GetPointData()->SetScalars(glyphScalars);
    GlyphPoints[listNumber]->SetNumberOfPoints(0);
    GlyphScalars[listNumber]->SetNumberOfTuples(0);
  */
  vtkDebugMacro("AddPoints Done...");
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveList(vtkMRMLFiducialListNode * flist)
{
  if (flist != NULL)
    {
    // iterate through the list, removing it's fiducials from the display
    for (int f = 0; f < flist->GetNumberOfFiducials(); f++)
      {
      vtkDebugMacro("RemoveList: " << flist->GetID() << " removing fiducial point #" << f);
      this->RemoveFiducial(flist->GetNthFiducialID(f));
      }
    std::string id = std::string(flist->GetID());

    // The rest of these are only valid for the 3d glyphs
    if (this->DiamondTransformMap[id] != NULL)
      {
      this->DiamondTransformMap[id]->Delete();
      this->DiamondTransformMap.erase(id);
      }

    if (this->GlyphPointsMap[id] != NULL)
      {
      this->GlyphPointsMap[id]->Delete();
      this->GlyphPointsMap.erase(id);
      }

    if (this->GlyphScalarsMap[id] != NULL)
      {
      this->GlyphScalarsMap[id]->Delete();
      this->GlyphScalarsMap.erase(id);
      }
    if (this->GlyphPolyDataMap[id] != NULL)
      {
      this->GlyphPolyDataMap[id]->Delete();
      this->GlyphPolyDataMap.erase(id);
      }
    if (this->TextTransformMap[id] != NULL)
      {
      this->TextTransformMap[id]->Delete();
      this->TextTransformMap.erase(id);
      }
    if (this->SymbolTransformMap[id] != NULL)
      {
      this->SymbolTransformMap[id]->Delete();
      this->SymbolTransformMap.erase(id);
      }
    if (this->TransformFilterMap[id] != NULL)
      {
      this->TransformFilterMap[id]->Delete();
      this->TransformFilterMap.erase(id);
      }
    if (this->Glyph3DMap[id] != NULL)
      {
      this->Glyph3DMap[id]->Delete();
      this->Glyph3DMap.erase(id);
      }
    if (this->GlyphMapperMap [id] != NULL)
      {
      this->GlyphMapperMap[id]->Delete();
      this->GlyphMapperMap.erase(id);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveFiducial(const char *id)
{
  if (id == NULL)
    {
    return;
    }
  std::string stringID = std::string(id);

  std::map< std::string, vtkActor *>::iterator iter;
  iter = this->DisplayedFiducials.find(stringID);
  if (iter != this->DisplayedFiducials.end())
    {
    this->DisplayedFiducials[stringID]->Delete();
    this->DisplayedFiducials.erase(iter);
    }
  
   std::map< std::string, vtkFollower *>::iterator titer;
   titer = this->DisplayedTextFiducials.find(stringID);
   if (titer != this->DisplayedTextFiducials.end())
     {
     this->DisplayedTextFiducials[stringID]->Delete();
     this->DisplayedTextFiducials.erase(stringID);
     }

   std::map< std::string, vtkPointWidget *>::iterator pointIter;
   pointIter = this->DisplayedPointWidgets.find(stringID);
   if (pointIter != this->DisplayedPointWidgets.end())
     {
     vtkDebugMacro("RemoveFiducial: Deleting point widget at " << stringID.c_str());
         
          this->DisplayedPointWidgets[stringID]->RemoveObservers(vtkCommand::EnableEvent);
          this->DisplayedPointWidgets[stringID]->RemoveObservers(vtkCommand::StartInteractionEvent);
          this->DisplayedPointWidgets[stringID]->RemoveObservers(vtkCommand::InteractionEvent);
          this->DisplayedPointWidgets[stringID]->EnabledOff();
          this->DisplayedPointWidgets[stringID]->SetInteractor(NULL);
     this->DisplayedPointWidgets[stringID]->Delete();
     this->DisplayedPointWidgets.erase(stringID);
     }
    
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RequestRender()
{
    if (this->GetRenderPending())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after idle \"%s Render\"", this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::Render()
{
  this->MainViewer->Render();
  this->SetRenderPending(0);
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateFiducialsFromMRML()
{
  
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (scene == NULL)
    {
    vtkErrorMacro("...the scene is null... returning");
    return;
    }

  vtkDebugMacro("UpdateFiducialsFromMRML: Starting to update the viewer's actors, glyphs for the fid lists.");
  
  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    if (flist->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
      }
    if (flist->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
      }
    // observe display node
    if (flist->HasObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {     
      flist->AddObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
      }
    // fiducial point modified?
    if (flist->HasObserver ( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand );
      }

    // set up the points at which the glyphs will be shown
    double* selectedColor =  flist->GetSelectedColor();
    double* unselectedColor = flist->GetColor();

    std::string id = std::string(flist->GetID());

    // check to see if used a different symbol type the last time through
    // For now this isn't an issue, but watch for memory leaks.
    bool changeSymbolType = false;
    std::map< std::string, bool>::iterator iter;
    iter = this->Use3DSymbolsMap.find(id);
    if (iter != this->Use3DSymbolsMap.end())
      {
      // have rendered before
      if ((iter->second == true ? 1 : 0) != flist->GlyphTypeIs3D())
        {
        changeSymbolType = true;
        vtkDebugMacro("Changing symbol type between 2d and 3d!!!");
        }
      }
    if (flist->GlyphTypeIs3D())
      {
      this->Use3DSymbolsMap[id] = true;
      }
    else
      {
      this->Use3DSymbolsMap[id] = false;
      }

    // a flag set when an actor is found in the DisplayFiducials map
    int actorExists = 0;
    // a flag set when a point widget is found in the DisplayedPointWidgets map
    int pointWidgetExists = 0;

   
    
    if (this->Use3DSymbolsMap[id])
      {
      // do we already have the structures for this list?
      std::map< std::string, vtkPoints * >::iterator gpIter;
      gpIter = this->GlyphPointsMap.find(id);
      if (gpIter == this->GlyphPointsMap.end())
        {
        // this id isn't used as a key yet, so add the data structures need for
        // a new list
        AddList(flist);
        }
      else
        {
        // clear out the points list
        this->GlyphPointsMap[id]->SetNumberOfPoints(0);
        this->GlyphScalarsMap[id]->SetNumberOfTuples(0);
        }    
      
      // make sure that we've got the right glyph for the 3d case
      if (flist->GetGlyphType() == vtkMRMLFiducialListNode::Diamond3D)
        {
        if ( this->TransformFilterMap[id]->GetInput() != this->DiamondGlyphPolyData)
          {
          this->TransformFilterMap[id]->SetInput(this->DiamondGlyphPolyData);
          }
        }
      else if (flist->GetGlyphType() == vtkMRMLFiducialListNode::Sphere3D)
        {
        if (this->TransformFilterMap[id]->GetInput() != this->SphereSource->GetOutput())
          {
          this->TransformFilterMap[id]->SetInput(this->SphereSource->GetOutput());
          }
        }
      // set up the selected/unselected colours for the list
      if (this->GlyphMapperMap[id] != NULL &&
            this->GlyphMapperMap[id]->GetLookupTable() != NULL)
        {
        if (unselectedColor != NULL)
          {
          vtkLookupTable::SafeDownCast(this->GlyphMapperMap[id]->GetLookupTable())->SetTableValue(0,
                                                                                                  unselectedColor[0],
                                                                                                  unselectedColor[1],
                                                                                                  unselectedColor[2],
                                                                                                  1.0);
          } 
        if (selectedColor != NULL)
          {
          vtkLookupTable::SafeDownCast(this->GlyphMapperMap[id]->GetLookupTable())->SetTableValue(1,
                                                                                                  selectedColor[0],
                                                                                                  selectedColor[1],
                                                                                                  selectedColor[2],
                                                                                                  1.0);
          }
        }
      else
        {
        vtkErrorMacro("ERROR: unable to get a lookup table for the glyph mapper at fid list id " << id.c_str());
        } 

      this->GlyphScalarsMap[id]->SetNumberOfTuples(flist->GetNumberOfFiducials());

      vtkFollower * actor = NULL;
      actor = vtkFollower::SafeDownCast(GetFiducialActorByID(id.c_str()));
      if (actor != NULL)
        {
        actorExists = 1;
        }

      vtkGlyph3D * glyph3d = NULL;
      glyph3d = (vtkGlyph3D*)this->Glyph3DMap[id]; // ->GetItemAsObject(listNumber);
      if (glyph3d != NULL)
        {
        if (actorExists)
          {
          // actor is in the list, get it and the mapper
          glyph3d->SetOutput(actor->GetMapper()->GetInput());
          }
        else
          {
          // no actor, allocate vars and set up the pipeline
          actor = vtkFollower::New();              
          actor->SetMapper ( this->GlyphMapperMap[id] );
          this->MainViewer->AddViewProp ( actor );
          }                        
        // reset the fid list glyph actor's colours
        vtkLookupTable::SafeDownCast(actor->GetMapper()->GetLookupTable())->SetTableValue(0, unselectedColor[0],
                                                                                          unselectedColor[1],
                                                                                          unselectedColor[2],
                                                                                          1.0);
        vtkLookupTable::SafeDownCast(actor->GetMapper()->GetLookupTable())->SetTableValue(1,
                                                                                          selectedColor[0],
                                                                                          selectedColor[1],
                                                                                          selectedColor[2],
                                                                                          1.0);
       
        } // end of glyph3d != NULL

      // do the updates for each point, point position, scalar map, text actor
      // text

      // first get the list's transform node
      vtkMRMLTransformNode* tnode = flist->GetParentTransformNode();
      vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
      transformToWorld->Identity();
      if (tnode != NULL && tnode->IsLinear())
        {
        vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
        lnode->GetMatrixTransformToWorld(transformToWorld);
        }
      for (int f=0; f<flist->GetNumberOfFiducials(); f++)
        {
        // get this point
        float *xyz = flist->GetNthFiducialXYZ(f);
        // convert by the parent transform
        float xyzw[4];
        xyzw[0] = xyz[0];
        xyzw[1] = xyz[1];
        xyzw[2] = xyz[2];
        xyzw[3] = 1.0;
        float worldxyz[4], *worldp = &worldxyz[0];        
        transformToWorld->MultiplyPoint(xyzw, worldp);

        // add this point to the list of points
        vtkDebugMacro("Inserting point for id " << id.c_str() << " " << worldxyz[0] << " " << worldxyz[1] << " " << worldxyz[2]);
        this->GlyphPointsMap[id]->InsertNextPoint(worldxyz);
        //vtkWarningMacro("3D: added the next point to the glyph points map, " << f << " = " << flist->GetNthFiducialXYZ(f) << ", glyph points map now has " << this->GlyphPointsMap[id]->GetNumberOfPoints() << " points." << endl;

        // update the scalar map for the point selected state
        if (flist->GetNthFiducialSelected(f))
          {
          //vtkWarningMacro("\tfid " << f << " is Selected, setting scalar tuple " << f << " to 1\n";
          this->GlyphScalarsMap[id]->SetTuple1(f, 1.0);
          }
        else
          {
          //vtkWarningMacro("\tfid " << f << " is unselected, setting tuple " << f << " to 0\n";
          this->GlyphScalarsMap[id]->SetTuple1(f, 0.0);
          }
        //vtkWarningMacro("\tafter setting the tuple it's = " << this->GlyphScalarsMap[id]->GetTuple1(f) << endl;
        
        this->UpdateTextActor(flist, f);
    
        }
      transformToWorld->Delete();
      transformToWorld = NULL;
      // now update the actor that controls the full list
      this->SetFiducialDisplayProperty(flist, 0, actor, NULL);
      
      // set the symbol scale      
      if (this->SymbolTransformMap[id] != NULL)        
        {
        this->SymbolTransformMap[id]->Identity();
        float symbolScale = flist->GetSymbolScale();
        this->SymbolTransformMap[id]->Scale(symbolScale, symbolScale, symbolScale);
        } else { vtkErrorMacro("ERROR: unable to get the transform for the list " << id.c_str() << "  symbol\n"); }
      
      if (!actorExists)
        {
        vtkDebugMacro("Adding an actor for the fiducial list into the disp fids list at id " << id.c_str());
        this->DisplayedFiducials[id] = actor;
        }
      // reset the poly data
      this->GlyphPolyDataMap[id]->SetPoints(this->GlyphPointsMap[id]);
      this->GlyphPolyDataMap[id]->GetPointData()->SetScalars(this->GlyphScalarsMap[id]);
      } // end of 3d symbols
    else
      {
      // have 2d glyphs, use one actor for each point in the list
      for (int f=0; f<flist->GetNumberOfFiducials(); f++)
        {
        vtkFollower * actor = NULL;
        actor = vtkFollower::SafeDownCast(GetFiducialActorByID(flist->GetNthFiducialID(f)));

        if (actor != NULL)
          {
          actorExists = 1;
          }
        
        vtkPolyDataMapper *mapper = NULL;
        vtkSlicerGlyphSource2D *glyph2d = vtkSlicerGlyphSource2D::New();
        if (actorExists)
          {
          // actor is in the list, get it and the mapper
          glyph2d->SetOutput(actor->GetMapper()->GetInput());              
          }
        else
          {
          // no actor, allocate vars and set up the pipeline
          mapper = vtkPolyDataMapper::New ();
          mapper->SetInput ( glyph2d->GetOutput() );
          actor = vtkFollower::New ( );
          actor->SetCamera(this->MainViewer->GetRenderer()->GetActiveCamera());
          actor->SetMapper ( mapper );
          mapper->Delete();
          this->MainViewer->AddViewProp ( actor );
          }
        
        if (glyph2d != NULL)
          {
          glyph2d->SetGlyphTypeAsString(flist->GetGlyphTypeAsString());
          if (flist->GetNthFiducialSelected(f))
            {
            glyph2d->SetColor(flist->GetSelectedColor());
            }
          else
            {
            glyph2d->SetColor(flist->GetColor());
            }
          }
        this->UpdateTextActor(flist, f);
        // now update the actor
        this->SetFiducialDisplayProperty(flist, f, actor, NULL);
        
        // save the actor and clean up, if necessary
        if (!actorExists)
          {
          vtkDebugMacro("Adding an actor to the displayed fiducials list at id " << flist->GetNthFiducialID(f));
          this->DisplayedFiducials[flist->GetNthFiducialID(f)] = actor;   
          }
        // only call delete if made them new, they didn't exist before
        if (glyph2d != NULL)
          {
          glyph2d->SetOutput(NULL);
          glyph2d->Delete();
          glyph2d = NULL;
          }
        } // end of for each fid point on list
      } // end of 2d symbols

    // check for point widgets
    for (int f=0; f<flist->GetNumberOfFiducials(); f++)
      {
      std::string fid = flist->GetNthFiducialID(f);
      float *pos = flist->GetNthFiducialXYZ(f);
    double x[3];
    x[0] = pos[0];
    x[1] = pos[1];
    x[2] = pos[2];
    vtkPointWidget * pointWidget = NULL;
    pointWidget = vtkPointWidget::SafeDownCast(GetPointWidgetByID(fid.c_str()));
    if (pointWidget != NULL)
      {
      pointWidgetExists = 1;
      }
    if (pointWidgetExists)
      {
      vtkDebugMacro("point widget exists " << fid.c_str() << ", update...");
      }
    else
      {
      // no point widget, allocate vars 
      pointWidget = vtkPointWidget::New();
      //      pointWidget->GetSelectedProperty()->SetColor(selectedColor);
      //      pointWidget->GetProperty()->SetColor(unselectedColor);
      
      //pointWidget->SetPriority(100);

      // trigger the renderer so that picking the point widgets will work
      // TODO: figure out the best place to put this, should only need to be called once
      int rwSizeX = this->MainViewer->GetRenderWindow()->GetSize()[0];
      int rwSizeY = this->MainViewer->GetRenderWindow()->GetSize()[1];
      vtkDebugMacro("New fid widget: Updating interactor size to " << rwSizeX << " , " << rwSizeY);
      this->MainViewer->GetRenderWindow()->GetInteractor()->UpdateSize(rwSizeX,rwSizeY);

      pointWidget->SetInteractor(this->MainViewer->GetRenderWindowInteractor());
      pointWidget->EnabledOn();
      pointWidget->AllOff();
      vtkPointWidgetCallback *myCallback = vtkPointWidgetCallback::New();
      myCallback->FiducialIndex = f;
      myCallback->FiducialList = flist;
      pointWidget->AddObserver(vtkCommand::EnableEvent, myCallback);
      pointWidget->AddObserver(vtkCommand::StartInteractionEvent, myCallback);
      pointWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
      // clean up callback to avoid leaks
      myCallback->Delete();
      pointWidget->PlaceWidget(x[0]-1, x[0]+1, x[1]-1, x[1]+1, x[2]-1, x[2]+1);
      pointWidget->TranslationModeOn();
      pointWidget->SetPosition(x);
      vtkDebugMacro("Putting new fiducial " << fid.c_str() << " in place: " << x[0] << "," << x[1] << "," << x[2]);
      this->DisplayedPointWidgets[fid] = pointWidget;
      }
    this->UpdatePointWidget(flist, f);
      }
    // let go of the pointer
    flist = NULL;
    } // end of while loop for each vtkMRMLFiducialListNode
    
  // Render
  if (this->MainViewer != NULL)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdateTextActor(vtkMRMLFiducialListNode *flist, int f)
{
  if (flist == NULL || f < 0)
    {
    return;
    }
  // handle text
  // check to see if this fiducial follower has actors in the
  // DisplayedTextFiducials map
  int textActorExists = 0;
  std::map< std::string, vtkFollower *>::iterator titer;

  titer = this->DisplayedTextFiducials.find(flist->GetNthFiducialID(f));
  if (titer != this->DisplayedTextFiducials.end())
    {
    textActorExists = 1;
    }
      
  vtkVectorText *vtext = vtkVectorText::New();
  vtkPolyDataMapper *textMapper;
  vtkFollower *textActor;
  if (textActorExists)
    {
    // get it out of the map
    textActor = titer->second;
    vtext->SetOutput(titer->second->GetMapper()->GetInput());
    }
  else
    {
    textMapper = vtkPolyDataMapper::New ();
    textMapper->SetInput ( vtext->GetOutput() );
    
    textActor = vtkFollower::New();
    textActor->SetCamera(this->MainViewer->GetRenderer()->GetActiveCamera());
    textActor->SetMapper(textMapper);
    
    this->MainViewer->AddViewProp ( textActor );
    }
  vtext->SetText(flist->GetNthFiducialLabelText(f));

  // set the display properties on just the text actor
  this->SetFiducialDisplayProperty(flist, f, NULL, textActor);

  if (!textActorExists)
    {
    this->DisplayedTextFiducials[flist->GetNthFiducialID(f)] = textActor;
    // only delete them if made them new
    if (textMapper != NULL)
      {
      textMapper->Delete();
      textMapper = NULL;
      }
    if (vtext != NULL)
      {
      vtext->Delete();
      vtext = NULL;
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::UpdatePointWidget(vtkMRMLFiducialListNode *flist, int f)
{
  if (flist == NULL || f < 0)
    {
    return;
    }

  std::map< std::string, vtkPointWidget *>::iterator pointIter;

  std::string fidID = flist->GetNthFiducialID(f);
  pointIter = this->DisplayedPointWidgets.find(fidID);
  if (pointIter != this->DisplayedPointWidgets.end())
    {
    float *xyz = flist->GetNthFiducialXYZ(f);
    if (xyz)
      {
      double pos[3];
      pos[0] = xyz[0]; pos[1] = xyz[1]; pos[2] = xyz[2];
      vtkDebugMacro("UpdatePointWidget: setting position for fid " << fidID.c_str() << " to " << pos[0] << ", " << pos[1] << ", " << pos[2]);
      vtkDebugMacro("UpdatePointWidget: point widget has observer? " << (pointIter->second->HasObserver(vtkCommand::InteractionEvent) ? "yes" : "no"));
      pointIter->second->SetInteractor(this->MainViewer->GetRenderWindowInteractor());
      // don't need to place it when updating it, just set position
      pointIter->second->SetPosition(pos);
      pointIter->second->EnabledOn();      
      }
    if (flist->GetVisibility() == 0 ||
        flist->GetNthFiducialVisibility(f) == 0)
      {
      // Point is not visible, disabling point widget
      pointIter->second->EnabledOff();
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveFiducialProps()
{
  // clear out any glyph actors
  if (this->DisplayedFiducials.size() == 0 &&
      this->DisplayedTextFiducials.size() == 0 && 
      this->DisplayedPointWidgets.size() == 0)
    {
    return;
    }

  vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveFiducialProps: number of displayed fiducials = " << this->DisplayedFiducials.size());
  std::map< std::string, vtkActor *>::iterator iter;
  for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
    {
    if (iter->second != NULL)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      // if the camera is set, let go of it
      if (vtkFollower::SafeDownCast(iter->second))
        {
        vtkFollower::SafeDownCast(iter->second)->SetCamera(NULL);
        }
      iter->second->Delete();
      }
    }
  this->DisplayedFiducials.clear();

  // text actors
  std::map< std::string, vtkFollower *>::iterator titer;
  for(titer=this->DisplayedTextFiducials.begin(); titer != this->DisplayedTextFiducials.end(); titer++) 
    {
    if (titer->second != NULL)
      {
      this->MainViewer->RemoveViewProp(titer->second);
      titer->second->SetCamera(NULL);
      titer->second->Delete();
      }
    }
  this->DisplayedTextFiducials.clear();

  // point widgets
  // TODO: this causes a crash, but without it, not getting the right number
  // of point widgets
  /*
  std::map< std::string, vtkPointWidget *>::iterator pointIter;
  for(pointIter=this->DisplayedPointWidgets.begin(); pointIter != this->DisplayedPointWidgets.end(); pointIter++) 
    {
    if (pointIter->second != NULL)
      {
      vtkWarningMacro("RemoveFiducialProps: Deleting point widget at " << pointIter->first.c_str());
      
      // pointIter->second->RemoveObservers(vtkCommand::EnableEvent);
      //pointIter->second->RemoveObservers(vtkCommand::StartInteractionEvent);
      //pointIter->second->RemoveObservers(vtkCommand::InteractionEvent);
      pointIter->second->EnabledOff();
      pointIter->second->SetInteractor(NULL);
//      pointIter->second->Delete();
      }
    //this->DisplayedPointWidgets.erase(pointIter->first);
    }
  //this->DisplayedPointWidgets.clear();
  */
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::RemoveFiducialObservers()
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveFiducialObservers\n");
    if (this->GetMRMLScene() == NULL)
    {
    vtkDebugMacro("vtkSlicerFiducialListWidget::RemoveFiducialObservers: no scene, returning...");
        return;
    }
    // remove the observers on all the fiducial lists
    vtkMRMLFiducialListNode *flist;
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
    for (int n=0; n<nnodes; n++)
      {
      flist = vtkMRMLFiducialListNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
      vtkDebugMacro("Removing observers on fiducial list " << flist->GetID());
      if (flist->HasObserver (vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 1)
        {
        flist->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
        }
      if (flist->HasObserver( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 1)
        {
        flist->RemoveObservers ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
        }
      if (flist->HasObserver( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 1)
        {
        flist->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
        }
      if (flist->HasObserver(vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand ) == 1)
        {
        flist->RemoveObservers ( vtkMRMLFiducialListNode::FiducialModifiedEvent, this->MRMLCallbackCommand );
        }
      }
    flist = NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetFiducialDisplayProperty(vtkMRMLFiducialListNode *flist, 
                                                       int n,
                                                       vtkActor *actor, vtkFollower *textActor)
{
  vtkDebugMacro("vtkSlicerFiducialListWidget::SetFiducialDisplayProperty: n = " << n << ", actor is " << (actor == NULL ? "null" : "not null"));
  float *xyz = flist->GetNthFiducialXYZ(n);
  if (xyz == NULL)
    {
    return;
    }
  float xyzw[4];
  xyzw[0] = xyz[0];
  xyzw[1] = xyz[1];
  xyzw[2] = xyz[2];
  xyzw[3] = 1.0;
  float worldxyz[4], *worldp = &worldxyz[0];
  int selected = flist->GetNthFiducialSelected(n);

  vtkMRMLTransformNode* tnode = flist->GetParentTransformNode();
  vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
  vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsLinear())
    {
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }
  transformToWorld->MultiplyPoint(xyzw, worldp);

  if (actor != NULL)
    {
    if (!this->Use3DSymbolsMap[flist->GetID()])
      {
      // don't set the position if it's a 3d list, it was done already at the
      // point level
      actor->SetPosition(worldxyz[0], worldxyz[1], worldxyz[2]);
      actor->SetScale(flist->GetSymbolScale());
      }
    // set the visibility, if the list is invisble, use 0, but if the list is
    // visible, use the individual fid's visibility flag
    if (flist->GetVisibility() == 0)
      {
      actor->SetVisibility(0);
      }
    else
      {
      actor->SetVisibility(flist->GetNthFiducialVisibility(n));
      }
    }

  if (textActor != NULL)
    {
    textActor->SetPosition(worldxyz[0], worldxyz[1], worldxyz[2]);
    textActor->SetScale(flist->GetTextScale());
    // visib from after tnode code
    // if the list is invisble, use 0, but if the list is
    // visible, use the individual fid's visibility flag
    if (flist->GetVisibility() == 0)
      {
      textActor->SetVisibility(0);
      }
    else
      {
      textActor->SetVisibility(flist->GetNthFiducialVisibility(n));
      }
    }

  transformToWorld->Delete();
  
  
  // don't update the actor's selected if it's 3d
  if (selected)
    {
    if (actor && !this->Use3DSymbolsMap[flist->GetID()])
      {
      actor->GetProperty()->SetColor(flist->GetSelectedColor());
      }
    if (textActor)
      {
      textActor->GetProperty()->SetColor(flist->GetSelectedColor());
      }
    }
  else
    {
    if (actor && !this->Use3DSymbolsMap[flist->GetID()])
      {
      actor->GetProperty()->SetColor(flist->GetColor());
      }
    if (textActor)
      {
      textActor->GetProperty()->SetColor(flist->GetColor());
      }
    }

  if (actor)
    {
    actor->GetProperty()->SetOpacity(flist->GetOpacity());
    actor->GetProperty()->SetAmbient(flist->GetAmbient());
    actor->GetProperty()->SetDiffuse(flist->GetDiffuse());
    actor->GetProperty()->SetSpecular(flist->GetSpecular());
    actor->GetProperty()->SetSpecularPower(flist->GetPower());
    actor->SetTexture(NULL);
    }
  if (textActor)
    {
    textActor->GetProperty()->SetOpacity(flist->GetOpacity());
    textActor->GetProperty()->SetAmbient(flist->GetAmbient());
    textActor->GetProperty()->SetDiffuse(flist->GetDiffuse());
    textActor->GetProperty()->SetSpecular(flist->GetSpecular());
    textActor->GetProperty()->SetSpecularPower(flist->GetPower());
    }

  vtkDebugMacro("vtkSlicerFiducialListWidget::SetFiducialDisplayProperty: done setting for " << n << "\n");

}

//---------------------------------------------------------------------------
std::string
vtkSlicerFiducialListWidget::GetFiducialNodeID (const char *actorid, int &index)
{
  // take the index off the actor id to get the fiducial node's id
  std::string actorString = actorid;
  std::stringstream ss;
  std::string sid;
  ss << actorid;
  ss >> sid;
  ss >> index;
  return sid;
}

//---------------------------------------------------------------------------
vtkActor *
vtkSlicerFiducialListWidget::GetFiducialActorByID (const char *id)
{  
  vtkDebugMacro("vtkSlicerFiducialListWidget::GetFiducialActorByID: id = " << id);
  if ( !id )      
    {
    return (NULL);
    }
  std::string sid = id;

  std::map< std::string, vtkActor *>::iterator iter;
  // search for matching string (can't use find, since it would look for 
  // matching pointer not matching content)
  for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
    {
    if ( iter->first.compare(sid) == 0 )
      {
      return (iter->second);
      }
    }
  return (NULL);
}

//---------------------------------------------------------------------------
vtkPointWidget *
vtkSlicerFiducialListWidget::GetPointWidgetByID (const char *id)
{  
  vtkDebugMacro("vtkSlicerFiducialListWidget::GetPointWidgetByID: id = " << id);
  if ( !id )      
    {
    return (NULL);
    }
  std::string sid = id;

  std::map< std::string, vtkPointWidget *>::iterator iter;
  // search for matching string (can't use find, since it would look for 
  // matching pointer not matching content)
  for(iter=this->DisplayedPointWidgets.begin(); iter != this->DisplayedPointWidgets.end(); iter++) 
    {
    if ( iter->first.compare(sid) == 0 )
      {
          vtkDebugMacro("Found point widget for id " << id );
      return (iter->second);
      }
    }
  return (NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  this->ViewerWidget = viewerWidget;
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialListWidget::SetInteractorStyle( vtkSlicerViewerInteractorStyle *interactorStyle )
{
  // note: currently the GUICallbackCommand calls ProcessWidgetEvents
  // remove observers
  if (this->InteractorStyle != NULL &&
      this->InteractorStyle->HasObserver(vtkSlicerViewerInteractorStyle::PickEvent, this->GUICallbackCommand) == 1)
    {
    this->InteractorStyle->RemoveObservers(vtkSlicerViewerInteractorStyle::PickEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  
  this->InteractorStyle = interactorStyle;

  // add observers
  if (this->InteractorStyle)
    {
    vtkDebugMacro("vtkSlicerFiducialWidget: Adding observer on interactor style");
    this->InteractorStyle->AddObserver(vtkSlicerViewerInteractorStyle::PickEvent, (vtkCommand *)this->GUICallbackCommand);
    }
}
