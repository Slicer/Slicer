// AnnotationModule/Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// AnnotationModule/MRML includes
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>

// AnnotationModule/DisplayableManager includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkMRMLAnnotationROIDisplayableManager.h"

// AnnotationModule/VTKWidgets includes
#include <vtkAnnotationROIRepresentation.h>
#include <vtkAnnotationROIWidget.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkMath.h>
#include <vtkCubeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// std includes
#include <string>
#include <math.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationROIDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationROIDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationROIDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationROIWidgetCallback *New()
  { return new vtkAnnotationROIWidgetCallback; }

  vtkAnnotationROIWidgetCallback(){}


  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
  {
    if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {

      // sanity checks
      if (!this->m_DisplayableManager)
        {
        return;
        }
      if (!this->m_Node)
        {
        return;
        }
      if (!this->m_Widget)
        {
        return;
        }
      // sanity checks end

      // the interaction with the widget ended, now propagate the changes to MRML
      vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(m_Node);
      if (roiNode->GetInteractiveMode() || 
           (!roiNode->GetInteractiveMode() && event == vtkCommand::EndInteractionEvent))
        {
        this->m_DisplayableManager->PropagateWidgetToMRML(this->m_Widget, this->m_Node);
        }
    }
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkMRMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkMRMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationROIDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationROIDisplayableManager::~vtkMRMLAnnotationROIDisplayableManager()
{
  std::map<vtkMRMLAnnotationNode*, vtkActor2D*>::iterator iter;
  for (iter = this->Contour2DActors.begin(); iter != this->Contour2DActors.end(); iter++)
    {
    iter->second->Delete();
    }
  this->Contour2DActors.clear();

  std::map<vtkMRMLAnnotationNode*, vtkPlane*>::iterator iter1;
  for (iter1 = this->Contour2DPlanes.begin(); iter1 != this->Contour2DPlanes.end(); iter1++)
    {
    iter1->second->Delete();
    }
  this->Contour2DPlanes.clear();

  std::map<vtkMRMLAnnotationNode*, vtkCutter*>::iterator iter2;
  for (iter2 = this->Contour2DCutters.begin(); iter2 != this->Contour2DCutters.end(); iter2++)
    {
    iter2->second->Delete();
    }
  this->Contour2DCutters.clear();

  std::map<vtkMRMLAnnotationNode*, vtkCubeSource*>::iterator iter3;
  for (iter3 = this->Contour2DCubes.begin(); iter3 != this->Contour2DCubes.end(); iter3++)
    {
    iter3->second->Delete();
    }
  this->Contour2DCubes.clear();

  std::map<vtkMRMLAnnotationNode*, vtkTransform*>::iterator iter4; 
  for (iter4 = this->Contour2DTransforms.begin(); iter4 != this->Contour2DTransforms.end(); iter4++)
    {
    iter4->second->Delete();
    }
  this->Contour2DTransforms.clear();

  std::map<vtkMRMLAnnotationNode*, vtkTransformPolyDataFilter*>::iterator iter5; 
  for (iter5 = this->Contour2DTransformFilters.begin(); iter5 != this->Contour2DTransformFilters.end(); iter5++)
    {
    iter5->second->Delete();
    }
  this->Contour2DTransformFilters.clear();

}
//---------------------------------------------------------------------------
void vtkMRMLAnnotationROIDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationROIDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("CreateWidget: Could not get ROI node!")
    return 0;
    }


  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::New();

  boxWidget->SetInteractor(this->GetInteractor());
  boxWidget->SetCurrentRenderer(this->GetRenderer());

  boxWidget->SetRotationEnabled(0);

  boxWidget->CreateDefaultRepresentation();

  vtkAnnotationROIRepresentation *boxRepresentation = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  boxRepresentation->SetPlaceFactor(1.0);

  // we place the widget around 0,0,0
  double bounds[6] = {0,0,0,0,0,0};
  bounds[0]=-1;
  bounds[1]=1;
  bounds[2]=-1;
  bounds[3]=1;
  bounds[4]=-1;
  bounds[5]=1;

  boxRepresentation->PlaceWidget(bounds);

  boxRepresentation->NeedToRenderOn();
  //this->RequestRender();
  // end of starting placement

  // now, we move the widget to the position according to the MRML node
  // this is saved as origin and radius
  // we need to get the transformation matrix of the boxRepresentation
  // and modify it according to MRML

  vtkTransform* transform = vtkTransform::New();
  boxRepresentation->GetTransform(transform);

  vtkMatrix4x4* matrix = vtkMatrix4x4::New();

  //std::cout << "Now coming the matrix from rep before modific." << std::endl;
  //transform->PrintSelf(std::cout,vtkIndent(1));

  // we need to save the origin here in the matrix
  //double origin[3];
  double origin[3];
  int originRetVal = roiNode->GetXYZ(origin);
  double radius[3];
  int radiusRetVal = roiNode->GetRadiusXYZ(radius);

  if(!originRetVal || !radiusRetVal)
    {
      vtkErrorMacro("CreateWidget: Failed to initialize a new ROI node!");
      return NULL;
    }

  matrix->SetElement(0,3,origin[0]);
  matrix->SetElement(1,3,origin[1]);
  matrix->SetElement(2,3,origin[2]);

  // we need to save the radius here in the matrix
  //double radius[3];
  matrix->SetElement(0,0,radius[0]*2.0);
  matrix->SetElement(1,1,radius[1]*2.0);
  matrix->SetElement(2,2,radius[2]*2.0);

  //matrix->DeepCopy(newMatrix);

  //std::cout << "Just the matrix after modific." << std::endl;
  //matrix->PrintSelf(std::cout,vtkIndent(1));

  transform->SetMatrix(matrix);

  //std::cout << "Now coming the transform after new matrix but before setting to rep" << std::endl;
  //transform->PrintSelf(std::cout,vtkIndent(1));

  boxRepresentation->SetTransform(transform);

  //vtkTransform* transformAfterSet = vtkTransform::New();
  //boxRepresentation->GetTransform(transformAfterSet);
  //std::cout << "Now coming the transform from rep" << std::endl;
  //transformAfterSet->PrintSelf(std::cout,vtkIndent(1));

  //boxWidget->ProcessEventsOff();

  boxRepresentation->EndWidgetInteraction(origin);
  boxRepresentation->NeedToRenderOn();

  boxWidget->On();

  matrix->Delete();
  transform->Delete();


  if (this->Is2DDisplayableManager())
    {
    this->Update2DCountour(node);
    }

  return boxWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationROIDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!")
    return;
    }

  if (this->Is2DDisplayableManager())
    {
    // cannot manipulate ROI in 2d views
    return;
    }

  vtkAnnotationROIWidgetCallback *myCallback = vtkAnnotationROIWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  node->SaveView();

}

void vtkMRMLAnnotationROIDisplayableManager::UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkMRMLNode *node)
{
  this->Update2DCountour(vtkMRMLAnnotationNode::SafeDownCast(node));
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationROIDisplayableManager::Update2DCountour(vtkMRMLAnnotationNode* node)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  vtkCubeSource *cube = NULL;
  vtkPlane *plane = NULL;
  vtkCutter *cutter = NULL;
  vtkTransformPolyDataFilter *transformFilter = NULL;
  vtkTransform *transform = NULL;
  vtkActor2D *actor = NULL;

  std::map<vtkMRMLAnnotationNode*, vtkActor2D*>::iterator iter = Contour2DActors.find(node);
  if (iter == Contour2DActors.end())
    {
    // create new pipeline
    cube = vtkCubeSource::New();
    plane = vtkPlane::New();

    cutter = vtkCutter::New();
    cutter->SetInput(cube->GetOutput());
    cutter->SetCutFunction(plane);

    transform = vtkTransform::New();
    transformFilter = vtkTransformPolyDataFilter::New();
    transformFilter->SetInput(cutter->GetOutput());
    transformFilter->SetTransform(transform);

    vtkPolyDataMapper2D *mapper = vtkPolyDataMapper2D::New();
    mapper->SetInput(transformFilter->GetOutput());

    actor = vtkActor2D::New();
    actor->SetMapper(mapper);

    this->GetRenderer()->AddActor2D(actor);
   
    this->Contour2DActors[node] = actor;
    this->Contour2DCutters[node] = cutter;
    this->Contour2DPlanes[node] = plane;
    this->Contour2DCubes[node] = cube;
    this->Contour2DTransforms[node] = transform;
    this->Contour2DTransformFilters[node] = transformFilter;

    mapper->Delete();
    }
  else
    {
    actor = iter->second;

    std::map<vtkMRMLAnnotationNode*, vtkPlane*>::iterator iter1 = Contour2DPlanes.find(node);
    if (iter1 == Contour2DPlanes.end())
      {
      vtkErrorMacro("Update2DCountour:plane not found")
      return;
      }
    plane = iter1->second;

    std::map<vtkMRMLAnnotationNode*, vtkCutter*>::iterator iter2 = Contour2DCutters.find(node);
    if (iter2 == Contour2DCutters.end())
      {
      vtkErrorMacro("Update2DCountour:cutter not found")
      return;
      }
    cutter = iter2->second;

    std::map<vtkMRMLAnnotationNode*, vtkCubeSource*>::iterator iter3 = Contour2DCubes.find(node);
    if (iter3 == Contour2DCubes.end())
      {
      vtkErrorMacro("Update2DCountour:cube source not found")
      return;
      }
    cube = iter3->second;


    std::map<vtkMRMLAnnotationNode*, vtkTransform*>::iterator iter4 = Contour2DTransforms.find(node);
    if (iter4 == Contour2DTransforms.end())
      {
      vtkErrorMacro("Update2DCountour: transform not found")
      return;
      }
    transform = iter4->second;

    std::map<vtkMRMLAnnotationNode*, vtkTransformPolyDataFilter*>::iterator iter5 = Contour2DTransformFilters.find(node);
    if (iter5 == Contour2DTransformFilters.end())
      {
      vtkErrorMacro("Update2DCountour:transform filter not found")
      return;
      }
    transformFilter = iter5->second;

    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  double xyz[4]={0,0,0,1};
  roiNode->GetXYZ(xyz);
  cube->SetCenter(xyz);

  double rxyz[4] = {0,0,0,0};
  roiNode->GetRadiusXYZ(rxyz);
  cube->SetXLength(2*rxyz[0]);
  cube->SetYLength(2*rxyz[1]);
  cube->SetZLength(2*rxyz[2]);

  // handle ROI transform to world space
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }

  // update the transform from world to screen space
  // for the extracted cut plane
  vtkSmartPointer<vtkMatrix4x4> rasToXY = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToXY->DeepCopy(sliceNode->GetXYToRAS());
  rasToXY->Invert();

  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  mat->Identity();
  mat->Multiply4x4(rasToXY, transformToWorld, mat);
  rasToXY->DeepCopy(mat);

  transform->SetMatrix(rasToXY);

  //
  // update the plane equation for the current slice cutting plane
  // - extract from the slice matrix
  // - normalize the normal
  transformToWorld->Invert();
  rasToXY->DeepCopy(sliceNode->GetXYToRAS());

  mat->Identity();
  mat->Multiply4x4(transformToWorld, rasToXY, mat);
  rasToXY->DeepCopy(mat);

  double normal[4]={0,0,0,1};
  double origin[4]={0,0,0,1};
  double sum = 0;
  int i;

  for (i=0; i<3; i++)
    {
    normal[i] = rasToXY->GetElement(i, 2);
    origin[i] = rasToXY->GetElement(i, 3);
    }
  for (i=0; i<3; i++)
    {
    sum += normal[i]*normal[i];
    }

  double lenInv = 1./sqrt(sum);
  for (i=0; i<3; i++)
    {
    normal[i] = normal[i]*lenInv;
    }

  plane->SetNormal(normal);
  plane->SetOrigin(origin);

  cutter->Update();
  transformFilter->Update();

  return;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROIDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  if (this->Is2DDisplayableManager())
    {
    std::map<vtkMRMLAnnotationNode*, vtkActor2D*>::iterator iter = this->Contour2DActors.find(vtkMRMLAnnotationNode::SafeDownCast(node));
    if (iter != this->Contour2DActors.end())
      {
      this->GetRenderer()->RemoveActor2D(iter->second);
      //this->Contour2DActors.erase(iter);
      }
    }
  this->Superclass::OnMRMLSceneNodeRemovedEvent(node);
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationROIDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  if (this->Is2DDisplayableManager())
    {
    this->Update2DCountour(node);
    return;
    }


  // cast to the specific widget
  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ROI node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());
  
  if (rep) 
    {
    // update widget from mrml
    double xyz[3];
    double rxyz[3];
    roiNode->GetXYZ(xyz);
    roiNode->GetRadiusXYZ(rxyz);

    double bounds[6];
    for (int i=0; i<3; i++)
      {
      bounds[  i] = xyz[i]-rxyz[i];
      bounds[3+i] = xyz[i]+rxyz[i];
      }
    double b[6];
    b[0] = bounds[0];
    b[1] = bounds[3];
    b[2] = bounds[1];
    b[3] = bounds[4];
    b[4] = bounds[2];
    b[5] = bounds[5];

    rep->PlaceWidget(b);
    }

  this->SetParentTransformToWidget(roiNode, boxWidget);

  // re-render the widget
  rep->NeedToRenderOn();
  boxWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationROIDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{
  if (this->Is2DDisplayableManager())
    {
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAnnotationROIWidget* boxWidget = vtkAnnotationROIWidget::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ROI node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;
  roiNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkAnnotationROIRepresentation * rep = vtkAnnotationROIRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  double extents[3];
  double center[3];
  rep->GetExtents(extents);
  rep->GetCenter(center);

  this->GetWorldToLocalCoordinates(roiNode, center, center);

  roiNode->SetXYZ(center[0], center[1], center[2] );
  roiNode->SetRadiusXYZ(0.5*extents[0], 0.5*extents[1], 0.5*extents[2] );

  // save the current view
  roiNode->SaveView();

  // enable processing of modified events
  roiNode->DisableModifiedEventOff();
  roiNode->Modified();

  roiNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, roiNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationROIDisplayableManager::OnClickInRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }
  if (this->Is2DDisplayableManager())
    {
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(2))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double origin[4] = {0,0,0,0};
    double radius[4] = {0,0,0,0};

    this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],origin);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],radius);

    double rx = sqrt((origin[0]-radius[0])*(origin[0]-radius[0]));
    if (rx==0)
      {
      rx = 100;
      }
    double ry = sqrt((origin[1]-radius[1])*(origin[1]-radius[1]));
    if (ry==0)
      {
      ry = rx;
      }
    double rz = sqrt((origin[2]-radius[2])*(origin[2]-radius[2]));
    if (rz==0)
      {
      rz = rx;
      }


    // done calculating the bounds

    // create the MRML node
    vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();

    roiNode->SetXYZ(origin);
    roiNode->SetRadiusXYZ(rx,ry,rz);

    roiNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationROI"));

    roiNode->Initialize(this->GetMRMLScene());

    roiNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }
    }

  }

void vtkMRMLAnnotationROIDisplayableManager::SetParentTransformToWidget(vtkMRMLAnnotationNode *node, vtkAbstractWidget *widget)
{
  if (!node || !widget)
    {
    return;
    }

  vtkAnnotationROIRepresentation *rep = vtkAnnotationROIRepresentation::SafeDownCast(widget->GetRepresentation());

  // get the nodes's transform node
  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  if (rep != NULL && tnode != NULL && tnode->IsLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    transformToWorld->Identity();
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);

    //vtkSmartPointer<vtkTransform> xform =  vtkSmartPointer<vtkTransform>::New();
    //xform->Identity();
    //xform->SetMatrix(transformToWorld);

    vtkSmartPointer<vtkPropCollection> actors =  vtkSmartPointer<vtkPropCollection>::New();
    rep->GetActors(actors);
    
    for (int i=0; i<actors->GetNumberOfItems(); i++)
      {
      vtkActor *actor = vtkActor::SafeDownCast(actors->GetItemAsObject(i));
      actor->SetUserMatrix(transformToWorld);
      }

    //rep->SetTransform(xform);

    widget->InvokeEvent(vtkCommand::EndInteractionEvent);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationROIDisplayableManager::IsWidgetDisplayable(vtkMRMLSliceNode* vtkNotUsed(sliceNode), 
                                                                 vtkMRMLAnnotationNode* vtkNotUsed(node))
{
  if (this->Is2DDisplayableManager())
    {
    return true;
    }
  else
    {
    return false;
    }
}
