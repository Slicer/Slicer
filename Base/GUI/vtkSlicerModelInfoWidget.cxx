#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkTriangleFilter.h"
#include "vtkMassProperties.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include "vtkSlicerModelInfoWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithLabel.h"

#include <itksys/SystemTools.hxx> 

#include <vtksys/stl/string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelInfoWidget );
vtkCxxRevisionMacro ( vtkSlicerModelInfoWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerModelInfoWidget::vtkSlicerModelInfoWidget ( )
{
  this->ModelNode = NULL;
  this->ModelSelectorWidget = NULL;
  
  this->AreaEntry = NULL;
  this->VolumeEntry = NULL;
  
  this->NumPointsEntry = NULL;
  this->NumCellsEntry = NULL;
  
  this->NumPointScalarsEntry = NULL;
  this->NumCellScalarsEntry = NULL;
  
  this->FileNameEntry = NULL;

  this->Triangles = vtkTriangleFilter::New();
  this->MassProps = vtkMassProperties::New();
  this->MassProps->SetInput( this->Triangles->GetOutput() );

  this->UpdatingFromMRML = 0;
}


//---------------------------------------------------------------------------
vtkSlicerModelInfoWidget::~vtkSlicerModelInfoWidget ( )
{
  if (this->ModelSelectorWidget)
    {
    this->ModelSelectorWidget->SetParent(NULL);
    this->ModelSelectorWidget->Delete();
    }

  if (this->VolumeEntry)
    {
    this->VolumeEntry->SetParent(NULL);
    this->VolumeEntry->Delete();
    }
  if (this->NumCellsEntry)
    {
    this->NumCellsEntry->SetParent(NULL);
    this->NumCellsEntry->Delete();
    }
  if (this->NumPointsEntry)
    {
    this->NumPointsEntry->SetParent(NULL);
    this->NumPointsEntry->Delete();
    }
  if (this->AreaEntry)
    {
    this->AreaEntry->SetParent(NULL);
    this->AreaEntry->Delete();
    }
  if (this->NumPointScalarsEntry)
    {
    this->NumPointScalarsEntry->SetParent(NULL);
    this->NumPointScalarsEntry->Delete();
    }
  if (this->NumCellScalarsEntry)
    {
    this->NumCellScalarsEntry->SetParent(NULL);
    this->NumCellScalarsEntry->Delete();
    }
  if (this->FileNameEntry)
    {
    this->FileNameEntry->SetParent(NULL);
    this->FileNameEntry->Delete();
    }

  this->SetMRMLScene ( NULL );  
  if (this->ModelNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->ModelNode, NULL);
    }

  this->Triangles->Delete();
  this->MassProps->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerModelInfoWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

void vtkSlicerModelInfoWidget::SetModelNode ( vtkMRMLModelNode *modelNode )
{ 
  // Select this model node
 
  if (this->ModelSelectorWidget)
    {
    this->ModelSelectorWidget->SetSelected(modelNode); 
    }
  // observe node modified events are not being observed
  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue( vtkCommand::ModifiedEvent);
  events->InsertNextValue( vtkMRMLModelNode::DisplayModifiedEvent);
  //  events->InsertNextValue( vtkMRMLModelNode::PolyDataModifiedEvent);

  vtkSetAndObserveMRMLNodeEventsMacro ( this->ModelNode, modelNode, events );
  events->Delete();
    
  // 
  // Set the member variables and do a first process
  //
  if ( modelNode != NULL)
    {
    this->ProcessMRMLEvents(modelNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
vtkMRMLModelNode * vtkSlicerModelInfoWidget::GetModelNode ()
{
  vtkMRMLModelNode *model;
  if (this->ModelSelectorWidget)
    {
    model = 
        vtkMRMLModelNode::SafeDownCast(this->ModelSelectorWidget->GetSelected());
    }
  else
    {
    model = this->ModelNode;
    }
   return model;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode * vtkSlicerModelInfoWidget::GetModelStorageNode ()
{ 
   vtkMRMLStorageNode *storage = NULL;
   vtkMRMLModelNode *model = this->GetModelNode();
   if (model != NULL)
      {
      storage = model->GetStorageNode();
      }
   return storage;
}

//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void * vtkNotUsed(callData) )
{

  //
  // process model selector events
  //
  if (this->ModelSelectorWidget)
    {
    vtkSlicerNodeSelectorWidget *modSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

    if (modSelector == this->ModelSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
      {
      vtkMRMLModelNode *model = 
        vtkMRMLModelNode::SafeDownCast(this->ModelSelectorWidget->GetSelected());

      if (model != NULL)
        {
        this->SetModelNode(model);
        this->UpdateWidgetFromMRML();
        }
      }
    return;
    }

}



//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void * vtkNotUsed(callData) )
{
  if (vtkMRMLModelNode::SafeDownCast(caller) != NULL && 
      vtkMRMLModelNode::SafeDownCast(caller) == this->GetModelNode() &&
     event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::UpdateWidgetFromMRML ()
{
  this->UpdatingFromMRML = 1;

  vtkMRMLModelNode *modelNode = this->GetModelNode();
  if (modelNode != NULL)
    {
    vtkPolyData *poly = modelNode->GetPolyData();
    if (poly)
      {
      this->Triangles->SetInput(poly);
      this->MassProps->Update();
      this->AreaEntry->GetWidget()->SetValueAsDouble(this->MassProps->GetSurfaceArea());
      this->VolumeEntry->GetWidget()->SetValueAsDouble(this->MassProps->GetVolume());

      this->NumCellsEntry->GetWidget()->SetValueAsInt(poly->GetNumberOfCells());
      this->NumPointsEntry->GetWidget()->SetValueAsInt(poly->GetNumberOfPoints());
      this->NumPointScalarsEntry->GetWidget()->SetValueAsInt(poly->GetPointData()->GetNumberOfComponents());
      this->NumCellScalarsEntry->GetWidget()->SetValueAsInt(poly->GetCellData()->GetNumberOfComponents());

      }
    }
  else
    {
    this->AreaEntry->GetWidget()->SetValueAsDouble(0.);
    this->VolumeEntry->GetWidget()->SetValueAsDouble(0.);

    this->NumCellsEntry->GetWidget()->SetValueAsInt(0);
    this->NumPointsEntry->GetWidget()->SetValueAsInt(0);
    this->NumPointScalarsEntry->GetWidget()->SetValueAsInt(0);
    this->NumCellScalarsEntry->GetWidget()->SetValueAsInt(0);
    }

  vtkMRMLStorageNode *storageNode = this->GetModelStorageNode();
  if (storageNode != NULL && storageNode->GetFileName() != NULL) 
    {
    itksys_stl::string dir =  
          itksys::SystemTools::GetParentDirectory(storageNode->GetFileName());   
    if (dir.size() > 0 && dir[dir.size()-1] != '/')
      {
      dir = dir + vtksys_stl::string("/");
      }
    itksys_stl::string name = itksys::SystemTools::GetFilenameName(storageNode->GetFileName());
    dir = dir+name;
    this->FileNameEntry->GetWidget()->SetValue(dir.c_str());
    }
  else
    {
    this->FileNameEntry->GetWidget()->SetValue("");
    }

  this->UpdatingFromMRML = 0;
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::AddWidgetObservers ( ) 
{
  if (this->ModelSelectorWidget)
    {
    if (this->MRMLScene != NULL)
      {
      this->ModelSelectorWidget->SetMRMLScene(this->MRMLScene);
      }
    this->ModelSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::RemoveWidgetObservers ( ) 
{
  if (this->ModelSelectorWidget)
    {
    this->ModelSelectorWidget->SetMRMLScene(NULL);
    this->ModelSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelInfoWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );


  this->ModelSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ModelSelectorWidget->SetParent ( frame );
  this->ModelSelectorWidget->Create ( );
  this->ModelSelectorWidget->SetNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->ModelSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ModelSelectorWidget->SetBorderWidth(2);
  // this->ModelSelectorWidget->SetReliefToGroove();
  this->ModelSelectorWidget->SetPadX(2);
  this->ModelSelectorWidget->SetPadY(2);
  this->ModelSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ModelSelectorWidget->SetLabelText( "Model Select: ");
  this->ModelSelectorWidget->SetNoneEnabled(1);
  this->ModelSelectorWidget->SetSelected(NULL);
  this->ModelSelectorWidget->SetBalloonHelpString("select a model from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ModelSelectorWidget->GetWidgetName());


  this->AreaEntry = vtkKWEntryWithLabel::New();
  this->AreaEntry->SetParent(frame);
  this->AreaEntry->Create();
  this->AreaEntry->SetLabelText("Surface Area(mm^2):");
  this->AreaEntry->GetWidget()->SetValueAsDouble(0);
  this->AreaEntry->SetLabelWidth(18);
  this->AreaEntry->SetWidth(48);
  this->AreaEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->AreaEntry->GetWidgetName());


  this->VolumeEntry = vtkKWEntryWithLabel::New();
  this->VolumeEntry->SetParent(frame);
  this->VolumeEntry->Create();
  this->VolumeEntry->SetLabelText("Volume(mm^3):");
  this->VolumeEntry->GetWidget()->SetValueAsDouble(0);
  this->VolumeEntry->SetLabelWidth(18);
  this->VolumeEntry->SetWidth(48);
  this->VolumeEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                 this->VolumeEntry->GetWidgetName ( ));

  this->NumPointsEntry = vtkKWEntryWithLabel::New();
  this->NumPointsEntry->SetParent(frame);
  this->NumPointsEntry->Create();
  this->NumPointsEntry->SetLabelText("Number of Points:");
  this->NumPointsEntry->GetWidget()->SetValueAsInt(0);
  this->NumPointsEntry->SetLabelWidth(18);
  this->NumPointsEntry->SetWidth(48);
  this->NumPointsEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                 this->NumPointsEntry->GetWidgetName ( ) );

  this->NumCellsEntry = vtkKWEntryWithLabel::New();
  this->NumCellsEntry->SetParent(frame);
  this->NumCellsEntry->Create();
  this->NumCellsEntry->SetLabelText("Number of Cells:");
  this->NumCellsEntry->GetWidget()->SetValueAsInt(0);
  this->NumCellsEntry->SetLabelWidth(18);
  this->NumCellsEntry->SetWidth(48);
  this->NumCellsEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                 this->NumCellsEntry->GetWidgetName ( ) );
  
  this->NumPointScalarsEntry = vtkKWEntryWithLabel::New();
  this->NumPointScalarsEntry->SetParent(frame);
  this->NumPointScalarsEntry->Create();
  this->NumPointScalarsEntry->SetLabelText("Number of Point Scalars:");
  this->NumPointScalarsEntry->GetWidget()->SetValueAsInt(0);
  this->NumPointScalarsEntry->SetLabelWidth(18);
  this->NumPointScalarsEntry->SetWidth(48);
  this->NumPointScalarsEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->NumPointScalarsEntry->GetWidgetName());


  this->NumCellScalarsEntry = vtkKWEntryWithLabel::New();
  this->NumCellScalarsEntry->SetParent(frame);
  this->NumCellScalarsEntry->Create();
  this->NumCellScalarsEntry->SetLabelText("Number of Cell Scalars:");
  this->NumCellScalarsEntry->GetWidget()->SetValueAsInt(0);
  this->NumCellScalarsEntry->SetLabelWidth(18);
  this->NumCellScalarsEntry->SetWidth(48);
  this->NumCellScalarsEntry->GetWidget()->SetStateToDisabled();
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->NumCellScalarsEntry->GetWidgetName());


  this->FileNameEntry = vtkKWEntryWithLabel::New();
  this->FileNameEntry->SetParent(frame);
  this->FileNameEntry->Create();
  this->FileNameEntry->SetLabelText("File Name:");
  this->FileNameEntry->GetWidget()->SetValue("");
  this->FileNameEntry->SetWidth(48);
  this->FileNameEntry->SetLabelWidth(18);
  this->FileNameEntry->GetWidget()->SetStateToDisabled();  
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->FileNameEntry->GetWidgetName());


  this->AddWidgetObservers();
  frame->Delete();

}
