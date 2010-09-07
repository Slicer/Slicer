// AnnotationModule includes
#include "vtkMRMLAnnotationSliceViewDisplayableManager.h"

// MRML includes
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkRenderWindowInteractor.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationSliceViewDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationSliceViewDisplayableManager, "$Revision: 1.1 $");


vtkMRMLAnnotationSliceViewDisplayableManager::vtkMRMLAnnotationSliceViewDisplayableManager()
{

  this->DebugOn();

}

//---------------------------------------------------------------------------
vtkMRMLAnnotationSliceViewDisplayableManager::~vtkMRMLAnnotationSliceViewDisplayableManager()
{

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSliceViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSliceViewDisplayableManager::OnMRMLSliceNodeModifiedEvent()
{
  vtkDebugMacro("OnMRMLSliceNodeModifiedEvent");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSliceViewDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{

  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  vtkMRMLSliceNode * sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);

  if(!sliceNode)
    {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: No slice node.")
    }

  vtkDebugMacro("The event was fired by the " << sliceNode->GetName() << " slice viewer.");

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSliceViewDisplayableManager::OnInteractorStyleEvent(int eventid)
{

  vtkDebugMacro("OnInteractorStyleEvent")

  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    //double x = this->GetInteractor()->GetEventPosition()[0];
    //double y = this->GetInteractor()->GetEventPosition()[1];
    vtkDebugMacro("OnInteractorStyleEvent: Got a click")
    }

}
