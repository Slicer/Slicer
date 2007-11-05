#include "vtkSlicerLabelmapElement.h"
#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include <sstream>
vtkCxxRevisionMacro(vtkSlicerLabelmapElement, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerLabelmapElement);
vtkSlicerLabelmapElement::vtkSlicerLabelmapElement(void)
{
    this->ColorName=NULL;
    this->Color=NULL;
    for(int i=0;i<6;i++)
    {
        this->Opacity[i]=NULL;
    }
}

vtkSlicerLabelmapElement::~vtkSlicerLabelmapElement(void)
{
    if(this->ColorName!=NULL)
    {
    this->ColorName->Delete();
    this->ColorName=NULL;
    }
    if(this->Color!=NULL)
    {
    this->Color->Delete();
    this->Color=NULL;
    }
    for(int i=0;i<6;i++)
    {
        if(this->Opacity[i]!=NULL)
        {
            this->Opacity[i]->Delete();
            this->Opacity[i]=NULL;
        }
    }
}
void vtkSlicerLabelmapElement::CreateWidget(void)
{
    Superclass::CreateWidget();

    this->ColorName=vtkKWLabel::New();
    this->ColorName->SetParent(this);
    this->ColorName->Create();
    this->ColorName->SetText("ColorName");
    this->Script("pack %s -side left -anchor c -expand y", this->ColorName->GetWidgetName());
    
    this->Color=vtkKWLabel::New();
    this->Color->SetParent(this);
    this->Color->SetText("     ");
    this->Color->Create();
    this->Script("pack %s -side left -anchor c -expand y",this->Color->GetWidgetName());

    for(int i=0;i<6;i++)
    {
        this->Opacity[i]=vtkKWPushButton::New();
        this->Opacity[i]->SetParent(this);
        this->Opacity[i]->Create();
        this->Script("pack %s -side left -anchor c -expand y",this->Opacity[i]->GetWidgetName());
    }
}
void vtkSlicerLabelmapElement::Init(std::string colorName,double color[3],int opacityLevel,int max)
{
    std::stringstream expansion;
    expansion<<colorName;
    int sizeBefore=expansion.str().length();
    for(int i=sizeBefore;i<max;i++)
    {
        expansion<<" ";
    }

    this->ColorName->SetText(expansion.str().c_str());
    this->Color->SetBackgroundColor(color);
    for(int i=0;i<=opacityLevel;i++)
    {
        this->Opacity[i]->SetBackgroundColor(.5,.5,.5);
    }
}
