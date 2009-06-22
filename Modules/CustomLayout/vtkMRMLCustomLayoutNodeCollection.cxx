#include <string>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLCustomLayoutNodeCollection.h"
#include "vtkMRMLCustomLayoutNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLCustomLayoutNodeCollection, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkMRMLCustomLayoutNodeCollection* vtkMRMLCustomLayoutNodeCollection::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCustomLayoutNodeCollection");
  if(ret)
    {
      return (vtkMRMLCustomLayoutNodeCollection*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCustomLayoutNodeCollection;
}


//----------------------------------------------------------------------------
vtkMRMLCustomLayoutNodeCollection::vtkMRMLCustomLayoutNodeCollection()
{
}




//----------------------------------------------------------------------------
vtkMRMLCustomLayoutNodeCollection::~vtkMRMLCustomLayoutNodeCollection()
{
  this->RemoveAllItems();
}



//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNodeCollection::DeleteCustomLayoutByName ( const char *name )
{
  int num = this->GetNumberOfItems();
  vtkMRMLCustomLayoutNode *n;
  for (int i=0; i < num; i++ )
    {
    n = vtkMRMLCustomLayoutNode::SafeDownCast ( this->GetItemAsObject ( i ) );
    if ( n != NULL && !(strcmp (n->GetName(), name )) )
      {
      this->RemoveItem ( i );
      }
    }
}


//----------------------------------------------------------------------------
vtkMRMLCustomLayoutNode* vtkMRMLCustomLayoutNodeCollection::GetLayoutByName (const char *name)
{
  int num = this->GetNumberOfItems();
  vtkMRMLCustomLayoutNode *n;
  for (int i=0; i < num; i++ )
    {
    n = vtkMRMLCustomLayoutNode::SafeDownCast ( this->GetItemAsObject ( i ) );
    if ( n != NULL &&  !(strcmp (n->GetName(), name )) )
      {
      return ( n );
      }
    }
  return ( NULL );
}



//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNodeCollection::AddCustomLayout (vtkMRMLCustomLayoutNode *layout )
{
  if ( layout == NULL )
    {
    vtkErrorMacro ("AddCustomLayout: got NULL layout node to add.");
    return;
    }
  if ( layout->GetName() == NULL )
    {
    vtkErrorMacro ("AddCustomLayout: got empty layout name -- layout not added to collection.");
    return;
    }

  this->AddItem ( layout );
}


//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNodeCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


