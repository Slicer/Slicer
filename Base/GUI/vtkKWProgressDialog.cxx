#include "vtkKWProgressDialog.h"

#include "vtkKWLabel.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWApplication.h"

#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWProgressDialog );

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkKWProgressDialog , "$Revision: 1.49 $");

//----------------------------------------------------------------------------
vtkKWProgressDialog::vtkKWProgressDialog(void)
{
    this->Message=NULL;
    this->Progress=NULL;
    this->MessageText="";
}

//----------------------------------------------------------------------------
vtkKWProgressDialog::~vtkKWProgressDialog(void)
{
    if(this->Message!=NULL)
    {
        this->Message->SetParent(NULL);
        this->Message->Delete();
        this->Message=NULL;
    }

    if(this->Progress!=NULL)
    {
        this->Progress->SetParent(NULL);
        this->Progress->Delete();
        this->Progress=NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWProgressDialog::CreateWidget(void)
{
    // Check if already created

    if (this->IsCreated())
    {
        vtkErrorMacro(<< this->GetClassName() << " already created");
        return;
    }

    // Call the superclass to create the whole widget

    this->Superclass::CreateWidget();

    this->Message=vtkKWLabel::New();
    this->Message->SetParent(this);
    this->Message->Create();
    this->Message->SetText(this->MessageText.c_str());
    this->Script("pack %s",this->Message->GetWidgetName());

    this->Progress=vtkKWProgressGauge::New();
    this->Progress->SetParent(this);
    this->Progress->Create();
    this->Script("pack %s",this->Progress->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkKWProgressDialog::UpdateProgress(float progress)
{
    this->Progress->SetValue(100*progress);
}

//----------------------------------------------------------------------------
void vtkKWProgressDialog::SetMessageText(const char* messageText)
{
    if(messageText==NULL)
    {
        vtkErrorMacro("messageText is not valid");
        return;
    }
    this->MessageText=messageText;
    if(this->Message!=NULL)
    {
        this->Message->SetText(this->MessageText.c_str());
    }
}

//----------------------------------------------------------------------------
void vtkKWProgressDialog::PrintSelf(std::ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
    os<<indent<<"vtkKWProgressDialog";
    os<<indent<<"Text: "<<this->MessageText;
    os<<indent<<"Progress: "<<this->Progress->GetValue();

}
void vtkKWProgressDialog::Display(void)
{
    Superclass::Display();
    this->GetApplication()->ProcessPendingEvents();
}
