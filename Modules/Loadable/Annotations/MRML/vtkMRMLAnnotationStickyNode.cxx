#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationStickyNode);

//---------------------------------------------------------------------------
int vtkMRMLAnnotationStickyNode::SetSticky(const char* text,int selectedFlag)
{
    if (text)
    {
        this->SetStickyText(text);
    }
    else
    {
        this->SetStickyText("");
    }

    this->SetSelected(selectedFlag);

    return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationStickyNode::Initialize(vtkMRMLScene* mrmlScene)
{
    if (!mrmlScene)
    {
        vtkErrorMacro("Scene was null!");
        return;
    }

    mrmlScene->AddNode(this);

    this->CreateAnnotationTextDisplayNode();
    this->AddText("",1,1);

}



