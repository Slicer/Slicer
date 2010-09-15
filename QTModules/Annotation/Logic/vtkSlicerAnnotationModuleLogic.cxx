// QTGUI includes
// Note: The logic shouldn't deal with UI Dialog
#include <qSlicerApplication.h>
#include <qSlicerIOManager.h>

// Annotation includes
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// Annotation/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationSplineNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTransformNode.h> // for ruler
#include <vtkMRMLLinearTransformNode.h> // for ruler
#include <vtkMRMLBSplineTransformNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkProperty.h>
#include <vtkPolygonalSurfacePointPlacer.h>
// Note: The following VTK includes should be in DisplayableManager
#include <vtkLineRepresentation.h>
#include <vtkLineWidget2.h> // for ruler
#include <vtkPointHandleRepresentation3D.h>
#include <vtkSphereHandleRepresentation.h>
#include <vtkAngleWidget.h>
#include <vtkAngleRepresentation3D.h>
#include <vtkSplineWidget.h>
#include <vtkSplineRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSeedRepresentation.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerAnnotationModuleLogic, "$Revision: 1.9.12.1 $")
;
vtkStandardNewMacro(vtkSlicerAnnotationModuleLogic)
;

//-----------------------------------------------------------------------------
// General Functions 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class vtkSlicerAnnotationModuleLogicPrivate
{
public:
  vtkSlicerAnnotationModuleLogicPrivate();
  ~vtkSlicerAnnotationModuleLogicPrivate();
  //char* FiducialListID;
  char* AngleNodeID;
  int numControlPoints;
  int maxControlPoints;
  int Updating3DFiducial;
  int Updating3DRuler;
  int Updating3DAngle;
  //BTX
  // Description:
  // encapsulated 3d widgets for each ruler node
  //std::map<std::string, vtkMeasurementsAngleWidgetClass *> AngleWidgets;
  // Not using it really just as dummy place holder
  //std::map<std::string, vtkSlicerFiducialListWidget *> FiducialWidgets;
  //ETX

};

//-----------------------------------------------------------------------------
// vtkSlicerAnnotationModuleLogicPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogicPrivate::vtkSlicerAnnotationModuleLogicPrivate()
{
  this->AngleNodeID = 0;
  this->Updating3DFiducial = 0;
  this->Updating3DRuler = 0;
  this->Updating3DAngle = 0;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogicPrivate::~vtkSlicerAnnotationModuleLogicPrivate()
{
  /*
   // 3d widgets
   std::map<std::string, vtkMeasurementsAngleWidgetClass *>::iterator iter2;
   for (iter2 = this->AngleWidgets.begin();
   iter2 != this->AngleWidgets.end();
   iter2++)
   {
   iter2->second->Delete();
   }
   this->AngleWidgets.clear();
   */
}

//-----------------------------------------------------------------------------
// vtkSlicerAnnotationModuleLogic methods

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::vtkSlicerAnnotationModuleLogic()
  : d_ptr(new vtkSlicerAnnotationModuleLogicPrivate)
{
  this->m_Widget = 0;

  this->m_LastAddedAnnotationNode = 0;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{
  this->m_Widget = 0;
  this->m_LastAddedAnnotationNode = 0;
}

void vtkSlicerAnnotationModuleLogic::SetAndObserveWidget(qSlicerAnnotationModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  this->m_Widget = widget;
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}
;

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{

  vtkDebugMacro("ProcessMRMLEvents: Event "<< event);

  vtkMRMLAnnotationNode* annotationNode =
      reinterpret_cast<vtkMRMLAnnotationNode*> (callData);
  if (!annotationNode)
    {
    return;
    }

  switch (event)
    {
    case vtkMRMLScene::NodeAddedEvent:
      this->OnMRMLSceneNodeAddedEvent(annotationNode);
      break;
    case vtkCommand::ModifiedEvent:
      this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
      break;

    }

  /*
   // Check RulerNode
   {
   vtkMRMLAnnotationRulerNode *callerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(caller);
   if (callerNode != 0)
   {
   if (event == vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent)
   {
   this->TestReceivedMessage = 1;
   //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
   // need to be sure that the modifeid event isn't coming from the widget moving
   //this->Update3DRuler(callerNode);
   }
   if (event == vtkCommand::ModifiedEvent)
   {
   this->TestReceivedMessage = 1;
   //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
   // need to be sure that the modifeid event isn't coming from the widget moving
   //this->Update3DRuler(callerNode);
   }
   if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
   {
   vtkDebugMacro("ProcessLogicEvents: got a transform modified event");
   //this->Update3DRuler(callerNode);
   }
   // check for a node added event
   if (event == vtkMRMLScene::NodeAddedEvent)
   {
   vtkDebugMacro("ProcessMRMLEvents: got a node added event on scene");
   //check to see if it was a angle node
   vtkMRMLAnnotationAngleNode *addNode = reinterpret_cast<vtkMRMLAnnotationAngleNode*>(callData);
   if (addNode != 0 && addNode->IsA("vtkMRMLAnnotationAngleNode"))
   {
   vtkDebugMacro("Got a node added event with a angle node " << addNode->GetID());
   return;
   }
   }
   }
   else
   {
   vtkMRMLAnnotationTextDisplayNode *callerNode = vtkMRMLAnnotationTextDisplayNode::SafeDownCast(caller);
   if (callerNode != 0)
   {
   if (event == vtkCommand::ModifiedEvent)
   {
   this->TestReceivedMessage = 1;
   //vtkWarningMacro("ProcessLogicEvents: got a control point modified event");
   // need to be sure that the modifeid event isn't coming from the widget moving
   //this->Update3DRuler(callerNode);
   }
   }
   }
   }

   // Check for fiducials

   {
   vtkMRMLFiducialListNode* callerNode = vtkMRMLFiducialListNode::SafeDownCast(caller);
   if (callerNode != 0)
   {
   if ( event == vtkMRMLScene::NodeAddedEvent )
   {
   return;
   }
   if ( event == vtkMRMLFiducialListNode::FiducialModifiedEvent )
   {
   return;
   }

   if ( event == vtkMRMLScene::NodeRemovedEvent )
   {
   return;
   }
   }
   }*/

}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  this->AddNodeCompleted(annotationNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent " << node->GetID());

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  const char * measurementValue = this->GetAnnotationMeasurement(node->GetID(),false);
  const char* textValue = this->GetAnnotationText(node->GetID());

  this->m_Widget->updateAnnotationInTableByID(annotationNode->GetID(), measurementValue, textValue);

  /*vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
   if (!annotationNode)
   {
   return;
   }

   if (annotationNode->IsA("vtkMRMLAnnotationTextNode")) {
   this->AddNodeCompleted(annotationNode);
   }
   if (annotationNode->IsA("vtkMRMLAnnotationAngleNode")) {
   this->AddNodeCompleted(annotationNode);
   }*/
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SaveMRMLScene()
{
  /*
   vtkSlicerApplicationGUI *appGUI = GetApplicationGUI();
   if ( !appGUI )
   {
   return;
   }
   appGUI->ProcessSaveSceneAsCommand();
   */
  qSlicerApplication::application()->ioManager()->openSaveDataDialog();
}

//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicerAnnotationModuleLogic::GetViewerWidget()
{
  //return this->GetApplicationGUI()->GetActiveViewerWidget();
  return 0;
}
/*
 //---------------------------------------------------------------------------
 vtkSlicerApplicationGUI* vtkSlicerAnnotationModuleLogic::GetApplicationGUI()
 {
 vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (vtkSlicerApplication::GetInstance());
 if ( !app )
 {
 std::cerr << "GetApplicationGUI: got 0 SlicerApplication"  << std::endl;
 return 0;
 }
 vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
 if ( !appGUI )
 {
 std::cerr << "GetApplicationGUI: got 0 SlicerApplicationGUI"  << std::endl;
 return 0;
 }
 return appGUI;
 
 }
 */

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RemoveAnnotationByID(const char* id)
{
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ModifyPropertiesAndWidget(vtkMRMLNode* node, int type, void* data)
{
  if (!node)
    {
    return;
    }
  if (vtkMRMLAnnotationLinesNode::SafeDownCast(node) == 0)
    {
    this->SetAnnotationControlPointsProperties(
        vtkMRMLAnnotationControlPointsNode::SafeDownCast(node), type, data);
    }
  else
    {
    this->SetAnnotationLinesProperties(
        vtkMRMLAnnotationLinesNode::SafeDownCast(node), type, data);
    }

  //this->GetViewerWidget()->Render();
}

//-----------------------------------------------------------------------------
/*vtkImageData* vtkSlicerAnnotationModuleLogic::SaveScreenShot()
 {
 return 0;

 vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (vtkSlicerApplication::GetInstance());
 if ( !app )
 {
 std::cerr << "selectLayout: got 0 SlicerApplication"  << std::endl;
 return 0;
 }

 vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
 if ( !appGUI )
 {
 std::cerr << "selectLayout: got 0 SlicerApplicationGUI"  << std::endl;
 return 0;
 }

 vtkSlicerViewerWidget* viewerWidget = appGUI->GetActiveViewerWidget();
 vtkKWRenderWidget* mainViewer = viewerWidget->GetMainViewer();
 vtkRenderWindow* win = mainViewer->GetRenderWindow();

 vtkWindowToImageFilter* renderLargeImage = vtkWindowToImageFilter::New();
 renderLargeImage->SetInput( win );
 renderLargeImage->SetMagnification( 1 );

 mainViewer->Render();
 renderLargeImage->Update();

 return renderLargeImage->GetOutput();

 }*/


//-----------------------------------------------------------------------------
// MRML Related Set/Get Functions     
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationLinesProperties(vtkMRMLAnnotationLinesNode* node, int type, void* data)
{
  if (!node)
    {
    return;
    }
  this->SetAnnotationControlPointsProperties(node, type, data);

  // Line Display Properties
  if (node->GetAnnotationLineDisplayNode() == 0)
    {
    }
  node->CreateAnnotationLineDisplayNode();
  this->SetAnnotationLineDisplayProperties(
      node->GetAnnotationLineDisplayNode(), type, data);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationProperties(vtkMRMLAnnotationNode* node, int type, void* data)
{
  if (!node)
    {
    return;
    }

  switch (type)
    {
    case TEXT:
      {
      node->SetText(0, (char*) data, 0, 1);
      break;
      }
    default:
      {
      if (node->GetAnnotationTextDisplayNode() == 0)
        {
        }
      node->CreateAnnotationTextDisplayNode();
      this->SetAnnotationTextDisplayProperties(
          node->GetAnnotationTextDisplayNode(), type, data);
      break;
      }
    }

}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::SetAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode* node, int type, void* data)
{
  if ((type < TEXT_COLOR) || (type > TEXT_SCALE))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case TEXT_SCALE:
      {
      node->SetTextScale(*(double*) data);
      return 1;
      }
    default:
      {
      return this->SetAnnotationDisplayProperties(node, type, data);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode* node, int type, void* data)
{
  if (!node)
    {
    return;
    }

  this->SetAnnotationProperties(node, type, data);
  if (node->GetAnnotationPointDisplayNode() == 0)
    {
    }
  node->CreateAnnotationPointDisplayNode();
  this->SetAnnotationPointDisplayProperties(
      node->GetAnnotationPointDisplayNode(), type, data);
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::SetAnnotationPointDisplayProperties(vtkMRMLAnnotationPointDisplayNode* node, int type, void* data)
{
  if ((type < POINT_COLOR) || (type > POINT_SIZE))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case POINT_SIZE:
      {
      //ToDo: set Point Size
      return 1;
      }
    default:
      return this->SetAnnotationDisplayProperties(node, type, data);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::SetAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode* node, int type, void* data)
{
  if ((type < LINE_COLOR) || (type > LINE_WIDTH))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case LINE_WIDTH:
      {
      node->SetLineThickness(*(double*) data);
      return 1;
      }
    default:
      return this->SetAnnotationDisplayProperties(node, type, data);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::SetAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode* node, int type, void* data)
{
  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case TEXT_OPACITY:
    case POINT_OPACITY:
    case LINE_OPACITY:
      node->SetOpacity(*(double*) data);
      return 1;
    case TEXT_AMBIENT:
    case POINT_AMBIENT:
    case LINE_AMBIENT:
      node->SetAmbient(*(double*) data);
      return 1;
    case TEXT_DIFFUSE:
    case POINT_DIFFUSE:
    case LINE_DIFFUSE:
      node->SetDiffuse(*(double*) data);
      return 1;
    case TEXT_SPECULAR:
    case POINT_SPECULAR:
    case LINE_SPECULAR:
      node->SetSpecular(*(double*) data);
      return 1;
    case TEXT_COLOR:
    case LINE_COLOR:
    case POINT_COLOR:
      {
      double* color = (double*) data;
      node->SetColor(color[0], color[1], color[2]);
      return 1;
      }
    case TEXT_SELECTED_COLOR:
    case POINT_SELECTED_COLOR:
    case LINE_SELECTED_COLOR:
      {
      double* color = (double*) data;
      node->SetSelectedColor(color[0], color[1], color[2]);
      return 1;
      }
    default:
      {
      std::cerr << "Property Type not found!" << endl;
      return 0;
      }
    }

}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetNumberOfControlPoints(vtkMRMLNode* mrmlnode)
{
  vtkMRMLAnnotationControlPointsNode* node =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);
  if (!node)
    {
    return -1;
    }
  return node->GetNumberOfControlPoints();
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetNumberOfControlPointsByID(const char * id)
{
  vtkMRMLNode * node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    return -1;
    }

  vtkMRMLAnnotationControlPointsNode* cpNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!cpNode)
    {
    return -1;
    }

  return cpNode->GetNumberOfControlPoints();
}

//-----------------------------------------------------------------------------
const char* vtkSlicerAnnotationModuleLogic::GetIconName(vtkMRMLNode* node, bool isEdit)
{
  if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditPoint.png";
      }
    else
      {
      return ":/Icons/AnnotationPoint.png";
      }
    }
  else if (node->IsA("vtkMRMLFiducialListNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditPoint.png";
      }
    else
      {
      return ":/Icons/AnnotationPoint.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditDistance.png";
      }
    else
      {
      return ":/Icons/AnnotationDistance.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditAngle.png";
      }
    else
      {
      return ":/Icons/AnnotationAngle.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditNote.png";
      }
    else
      {
      return ":/Icons/AnnotationNote.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditText.png";
      }
    else
      {
      return ":/Icons/AnnotationText.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditROI.png";
      }
    else
      {
      return ":/Icons/AnnotationROI.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditBidimensional.png";
      }
    else
      {
      return ":/Icons/AnnotationBidimensional.png";
      }
    }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
    {
    if (isEdit)
      {
      return ":/Icons/AnnotationEditSpline.png";
      }
    else
      {
      return ":/Icons/AnnotationSpline.png";
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetAnnotationTextProperty(vtkMRMLNode* node)
{
  vtkMRMLAnnotationNode *aNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!aNode)
    {
    return "";
    }
  return aNode->GetText(0);
}

const char* vtkSlicerAnnotationModuleLogic::GetAnnotationTextFormatProperty(vtkMRMLNode* node)
{
  if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    return " ";
    }
  if (node->IsA("vtkMRMLFiducialListNode"))
    {
    return " ";
    }
  else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
    return vtkMRMLAnnotationRulerNode::SafeDownCast(node)->GetDistanceAnnotationFormat();
    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
    return vtkMRMLAnnotationAngleNode::SafeDownCast(node)->GetLabelFormat();
    }
  else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {
    return " ";
    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
    {
    return " ";
    }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
    {
    return vtkMRMLAnnotationROINode::SafeDownCast(node)->GetROIAnnotationFormat();
    }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {
    return vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetAnnotationFormat();
    }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
    {
    return vtkMRMLAnnotationSplineNode::SafeDownCast(node)->GetDistanceAnnotationFormat();
    }

  return 0;
}

//-----------------------------------------------------------------------------
// Return the measurement valu of a annotation node
//-----------------------------------------------------------------------------
/*
std::vector<double> vtkSlicerAnnotationModuleLogic::GetAnnotationMeasurement(vtkMRMLNode* node)
{

}*/

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, vtkIdType coordId)
{
  vtkMRMLAnnotationControlPointsNode* node =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);

  if (!node || (coordId >= node->GetNumberOfControlPoints()))
    {
    return 0;
    }

  return node->GetControlPointCoordinates(coordId);
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationControlPointsCoordinateByID(const char * id, vtkIdType coordId)
{
  vtkMRMLNode * node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    return 0;
    }

  vtkMRMLAnnotationControlPointsNode* cpNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);

  if (!cpNode || (coordId >= cpNode->GetNumberOfControlPoints()))
    {
    return 0;
    }

  return cpNode->GetControlPointCoordinates(coordId);
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::SetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, double* pos, vtkIdType coordId)
{
  vtkMRMLAnnotationControlPointsNode* node =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlnode);

  if (!node || (coordId >= node->GetNumberOfControlPoints()))
    {
    return 0;
    }

  node->SetControlPoint(coordId, pos, 0, 1);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationLinesPropertiesDouble(vtkMRMLNode* node, int type, double &result)
{
  if (!node)
    {
    return 0;
    }
  vtkMRMLAnnotationLinesNode* lnode = vtkMRMLAnnotationLinesNode::SafeDownCast(
      node);
  if (!lnode)
    {
    return 0;
    }

  if (this->GetAnnotationControlPointsPropertiesDouble(
      (vtkMRMLAnnotationControlPointsNode*) lnode, type, result))
    {
    return 1;
    }

  lnode->CreateAnnotationLineDisplayNode();
  if (this->GetAnnotationLineDisplayPropertiesDouble(
      lnode->GetAnnotationLineDisplayNode(), type, result))
    {
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationControlPointsPropertiesDouble(vtkMRMLAnnotationControlPointsNode* node, int type, double &result)
{
  if (!node)
    {
    return 0;
    }

  if (this->GetAnnotationPropertiesDouble(node, type, result))
    {
    return 1;
    }

  node->CreateAnnotationPointDisplayNode();
  if (this->GetAnnotationPointDisplayPropertiesDouble(
      node->GetAnnotationPointDisplayNode(), type, result))
    {
    return 1;
    }
  return 0;

}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationLineDisplayPropertiesDouble(vtkMRMLAnnotationLineDisplayNode* node, int type, double& result)
{
  if ((type < LINE_COLOR) || (type > LINE_WIDTH))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case LINE_WIDTH:
      {
      result = node->GetLineThickness();
      return 1;
      }
    default:
      return this->GetAnnotationDisplayPropertiesDouble(node, type, result);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationDisplayPropertiesDouble(vtkMRMLAnnotationDisplayNode* node, int type, double& result)
{
  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case POINT_OPACITY:
    case LINE_OPACITY:
    case TEXT_OPACITY:
      result = node->GetOpacity();
      return 1;
    case POINT_AMBIENT:
    case LINE_AMBIENT:
    case TEXT_AMBIENT:
      result = node->GetAmbient();
      return 1;
    case POINT_DIFFUSE:
    case LINE_DIFFUSE:
    case TEXT_DIFFUSE:
      result = node->GetDiffuse();
      return 1;
    case POINT_SPECULAR:
    case LINE_SPECULAR:
    case TEXT_SPECULAR:
      result = node->GetSpecular();
      return 1;
    default:
      {
      std::cerr << "Property Type not found!" << endl;
      return 0;
      }
    }
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationPropertiesDouble(vtkMRMLAnnotationNode* node, int type, double& result)
{
  if (!node)
    {
    return 0;
    }

  node->CreateAnnotationTextDisplayNode();
  if (this->GetAnnotationTextDisplayPropertiesDouble(
      node->GetAnnotationTextDisplayNode(), type, result))
    {
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationTextDisplayPropertiesDouble(vtkMRMLAnnotationTextDisplayNode* node, int type, double& result)
{
  if ((type < TEXT_COLOR) || (type > TEXT_SCALE))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case TEXT_SCALE:
      result = node->GetTextScale();
      return 1;
    default:
      return this->GetAnnotationDisplayPropertiesDouble(node, type, result);
    }
}

//-----------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationPointDisplayPropertiesDouble(vtkMRMLAnnotationPointDisplayNode* node, int type, double& result)
{
  if ((type < POINT_COLOR) || (type > POINT_SIZE))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case POINT_SIZE:
      {
      //ToDo: get Point Size
      return 1;
      }
    default:
      return this->GetAnnotationDisplayPropertiesDouble(node, type, result);
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationLinesPropertiesColor(vtkMRMLNode* mrmlnode, int type)
{
  vtkMRMLAnnotationLinesNode* lnode = vtkMRMLAnnotationLinesNode::SafeDownCast(
      mrmlnode);
  if (!lnode)
    {
    return 0;
    }

  switch (type)
    {
    case LINE_COLOR:
    case LINE_SELECTED_COLOR:
      {
      lnode->CreateAnnotationLineDisplayNode();
      return this->GetAnnotationLineDisplayPropertiesColor(
          lnode->GetAnnotationLineDisplayNode(), type);
      }
    default:
      {
      return this->GetAnnotationControlPointsPropertiesColor(
          (vtkMRMLAnnotationControlPointsNode*) lnode, type);
      }
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationLineDisplayPropertiesColor(vtkMRMLAnnotationLineDisplayNode* node, int type)
{
  if ((type != LINE_COLOR) && (type != LINE_SELECTED_COLOR))
    {
    return 0;
    }

  if (!node)
    {
    return 0;
    }
  return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationControlPointsPropertiesColor(vtkMRMLAnnotationControlPointsNode* node, int type)
{
  if (!node)
    {
    return 0;
    }

  switch (type)
    {
    case POINT_COLOR:
    case POINT_SELECTED_COLOR:
      {
      node->CreateAnnotationPointDisplayNode();
      return this->GetAnnotationPointDisplayPropertiesColor(
          node->GetAnnotationPointDisplayNode(), type);
      }
    default:
      {
      return this->GetAnnotationPropertiesColor(node, type);
      }
    }
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationPropertiesColor(vtkMRMLAnnotationNode* node, int type)
{
  if (!node)
    {
    return 0;
    }
  node->CreateAnnotationTextDisplayNode();
  return this->GetAnnotationTextDisplayPropertiesColor(
      node->GetAnnotationTextDisplayNode(), type);
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationTextDisplayPropertiesColor(vtkMRMLAnnotationTextDisplayNode* node, int type)
{
  if ((type != TEXT_COLOR) && (type != TEXT_SELECTED_COLOR))
    {
    return 0;
    }
  if (!node)
    {
    return 0;
    }
  return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationPointDisplayPropertiesColor(vtkMRMLAnnotationPointDisplayNode* node, int type)
{
  if ((type != POINT_COLOR) && (type != POINT_SELECTED_COLOR))
    {
    return 0;
    }
  if (!node)
    {
    return 0;
    }
  return this->GetAnnotationDisplayPropertiesColor(node, type);
}

//-----------------------------------------------------------------------------
double* vtkSlicerAnnotationModuleLogic::GetAnnotationDisplayPropertiesColor(vtkMRMLAnnotationDisplayNode* node, int type)
{
  if (!node)
    {
    return 0;
    }
  switch (type)
    {
    case TEXT_COLOR:
    case POINT_COLOR:
    case LINE_COLOR:
      {
      return node->GetColor();
      }
    case TEXT_SELECTED_COLOR:
    case POINT_SELECTED_COLOR:
    case LINE_SELECTED_COLOR:
      {
      return node->GetSelectedColor();
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
// Add Annotation Node
//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddAnnotationNode(const char * nodeDescriptor)
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("AddAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActiveAnnotationID(nodeDescriptor);

  this->StartPlaceMode();

}

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StartPlaceMode()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
  events->Delete();

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (!interactionNode)
    {
    vtkErrorMacro ( "StartPlaceMode: No interaction node in the scene." );
    return;
    }

  interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
  interactionNode->SetPlaceModePersistence(1);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::Place)
    {

    vtkErrorMacro("StartPlaceMode: Could not set place mode!");
    return;

    }
}

//---------------------------------------------------------------------------
// called after a new annotation node was added, now add it to the table in the GUI
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddNodeCompleted(vtkMRMLAnnotationNode * node)
{

  if (!node)
    {
    return;
    }

  if (!this->m_Widget)
    {
    return;
    }

  this->m_Widget->addNodeToTable(node->GetID());
  this->m_LastAddedAnnotationNode = node;

}

//---------------------------------------------------------------------------
// Exit the place mode
void vtkSlicerAnnotationModuleLogic::StopPlaceMode()
{

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode == NULL)
    {
    vtkErrorMacro ( "StopPlaceMode: No interaction node in the scene." );
    return;
    }

  interactionNode->SetPlaceModePersistence(0);
  interactionNode->SetCurrentInteractionMode(
      vtkMRMLInteractionNode::ViewTransform);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::ViewTransform)
    {

    vtkErrorMacro("AddTextNode: Could not set transform mode!");

    }

}

//---------------------------------------------------------------------------
// Cancel the current placement or remove the last placed node
void vtkSlicerAnnotationModuleLogic::CancelCurrentOrRemoveLastAddedAnnotationNode()
{

  if (!this->m_LastAddedAnnotationNode)
    {
    return;
    }

  if (this->m_LastAddedAnnotationNode->IsA("vtkMRMLAnnotationTextNode"))
    {
    // for text annotations, just remove the last node
    this->GetMRMLScene()->RemoveNode(this->m_LastAddedAnnotationNode);
    this->m_LastAddedAnnotationNode = 0;
    }

}

//---------------------------------------------------------------------------
//
//
// Annotation Properties as an interface to MRML
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Return the name of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationName(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the annotationMRML node.")
    return 0;
    }

  return annotationNode->GetName();
}

//---------------------------------------------------------------------------
// Return the text of an annotation MRML Node
//---------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetAnnotationText(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the annotationMRML node.")
    return 0;
    }

  return annotationNode->GetText(0);

}

//---------------------------------------------------------------------------
// Set the text of an annotation MRML Node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationText(const char* id, const char * newtext)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the annotationMRML node.")
    return;
    }

  if (!newtext)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the text.")
    return;
    }

  std::cout << "text" << std::endl;

  annotationNode->SetText(0,newtext,1,1);

}

//---------------------------------------------------------------------------
// Get the textScale of a MRML Annotation node
//---------------------------------------------------------------------------
double vtkSlicerAnnotationModuleLogic::GetAnnotationTextScale(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetTextScale();

}

//---------------------------------------------------------------------------
// Set the textScale of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextScale(const char* id, double textScale)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->SetTextScale(textScale);

}

//---------------------------------------------------------------------------
// Get the selected text color of a MRML Annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationTextSelectedColor(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetSelectedColor();

}

//---------------------------------------------------------------------------
// Set the selected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextSelectedColor(const char* id, double * color)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetSelectedColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the unselected text color of a MRML Annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationTextUnselectedColor(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetColor();

}

//---------------------------------------------------------------------------
// Set the unselected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextUnselectedColor(const char* id, double * color)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the measurement value of a MRML Annotation node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationMeasurement(const char* id, bool showUnits)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the annotation MRML node.")
    return 0;
    }

  if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    return "";
    }
  else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
    std::ostringstream ss;
    ss << vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement();
    if (showUnits)
      {
      ss << " [mm]";
      }

    return ss.str().c_str();
    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
    std::ostringstream ss;
    ss << vtkMRMLAnnotationAngleNode::SafeDownCast(annotationNode)->GetAngleMeasurement();
    if (showUnits)
      {
      ss << " [degrees]";
      }

    return ss.str().c_str();
    }
  else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {
    return "";
    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
    {
    return "";
    }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
    {
    return "";
    }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {
    double measurement1;
    double measurement2;
    if (vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement().size() != 2)
      {
      // measurement is not ready
      measurement1 = 0;
      measurement2 = 0;
      }
    else
      {
      // measurement is ready
      measurement1 = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[0];
      measurement2 = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[1];
      }

    std::ostringstream ss;
    ss << measurement1;
    if (showUnits)
      {
      ss << " [mm]";
      }
    ss << " x ";
    ss << measurement2;
    if (showUnits)
      {
      ss << " [mm]";
      }

    return ss.str().c_str();
    }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
    {
    return "";
    }

  return "";

}


//---------------------------------------------------------------------------
// Return the text of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationIcon(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationIcon: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationIcon: Could not get the annotationMRML node.")
    return 0;
    }

  if (annotationNode->IsA("vtkMRMLAnnotationFiducialNode"))
    {

    return ":/Icons/AnnotationPoint.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationRulerNode"))
    {

    return ":/Icons/AnnotationDistance.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationAngleNode"))
    {

    return ":/Icons/AnnotationAngle.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationStickyNode"))
    {

    return ":/Icons/AnnotationNote.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationTextNode"))
    {

    return ":/Icons/AnnotationText.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationROINode"))
    {

    return ":/Icons/AnnotationROI.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {

    return ":/Icons/AnnotationBidimensional.png";

    }
  else if (annotationNode->IsA("vtkMRMLAnnotationSplineNode"))
    {

    return ":/Icons/AnnotationSpline.png";

    }

  return 0;

}

//---------------------------------------------------------------------------
// Return the lock flag for this annotation
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationLockedUnlocked(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the annotationMRML node.")
    return 0;
    }

  // lock this annotation
  return annotationNode->GetLocked();

}

//---------------------------------------------------------------------------
// Toggles the lock on this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationLockedUnlocked(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the annotationMRML node.")
    return;
    }

  // lock this annotation
  annotationNode->SetLocked(!annotationNode->GetLocked());

}


//---------------------------------------------------------------------------
// Return the visibility flag for this annotation
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationVisibility(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the annotationMRML node.")
    return 0;
    }

  // lock this annotation
  return annotationNode->GetVisible();

}

//---------------------------------------------------------------------------
// Toggles the visibility of this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationVisibility(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the annotationMRML node.")
    return;
    }

  // show/hide this annotation
  annotationNode->SetVisible(!annotationNode->GetVisible());

}

//---------------------------------------------------------------------------
// Toggles the selected flag of this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationSelected(const char * id, bool selected)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the annotationMRML node.")
    return;
    }

  // select this annotation
  annotationNode->SetSelected(selected);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Backup an AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::BackupAnnotationNode(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRML node.")
    return;
    }

  annotationNode->CreateBackup();

  for(int i=0; i<annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode = vtkMRMLAnnotationDisplayNode::SafeDownCast(annotationNode->GetNthDisplayNode(i));

    if(!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node.")
      return;
      }

    displayNode->CreateBackup();

    }

}

//---------------------------------------------------------------------------
// Restore a backup version of a AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RestoreAnnotationNode(const char * id)
{
  vtkDebugMacro("RestoreAnnotationNode: " << id)

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationMRML node.")
    return;
    }

  for(int i=0; i<annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode = vtkMRMLAnnotationDisplayNode::SafeDownCast(annotationNode->GetNthDisplayNode(i));

    if(!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node.")
      return;
      }

    if (displayNode->IsA("vtkMRMLAnnotationTextDisplayNode"))
      {
      // restore display nodes, one by one
       vtkMRMLAnnotationTextDisplayNode * backupDisplayNode = vtkMRMLAnnotationTextDisplayNode::SafeDownCast(displayNode->GetBackup());

       if (!backupDisplayNode)
         {
         vtkErrorMacro("RestoreAnnotationNode: There was no backup display node.")
         return;
         }

       // now restore
       displayNode->Copy(backupDisplayNode);
      }

    }

  vtkMRMLAnnotationNode * backupNode = annotationNode->GetBackup();

  if (!backupNode)
    {
    vtkErrorMacro("RestoreAnnotationNode: There was no backup.")
    return;
    }

  // now restore
  annotationNode->Copy(backupNode);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

