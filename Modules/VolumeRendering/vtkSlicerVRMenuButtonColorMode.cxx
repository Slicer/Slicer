#include "vtkSlicerVRMenuButtonColorMode.h"
#include "vtkColorTransferFunction.h"
#include "vtkObjectFactory.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include <sstream>

vtkCxxRevisionMacro(vtkSlicerVRMenuButtonColorMode, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerVRMenuButtonColorMode);
vtkSlicerVRMenuButtonColorMode::vtkSlicerVRMenuButtonColorMode(void)
{
    this->ColorTransferFunction=NULL;
    this->Range[0]=0;
    this->Range[1]=0;
    if(vtkSlicerVRMenuButtonColorMode::Values.size()==0)
    {
        vtkSlicerVRMenuButtonColorMode::CreateValues();
    }
}

vtkSlicerVRMenuButtonColorMode::~vtkSlicerVRMenuButtonColorMode(void)
{
}
void vtkSlicerVRMenuButtonColorMode::CreateWidget(void)
{
    Superclass::CreateWidget();
    unsigned int i=0;
    while(i<vtkSlicerVRMenuButtonColorMode::Values.size())
    {
        this->GetWidget()->GetMenu()->AddRadioButton(vtkSlicerVRMenuButtonColorMode::Values[i].Name);
        std::stringstream ss;
        ss<<"ProcessColorModeEvents ";
        ss<<i;
        this->GetWidget()->GetMenu()->SetItemCommand(i,this,ss.str().c_str());
        i++;
    }
}
vtkSlicerVRMenuButtonColorMode::ColorModeList vtkSlicerVRMenuButtonColorMode::CreateValues(void)
{
    ColorModeList tmp;
    tmp.push_back(ColorEntry("Aqua",0,1,1));
    tmp.push_back(ColorEntry("Black",0,0,0));
    tmp.push_back(ColorEntry("Blue",0,0,1));
    tmp.push_back(ColorEntry("Fuchsia",1,0,1));
    tmp.push_back(ColorEntry("Green",0,0.75,0));
    tmp.push_back(ColorEntry("Gray",0.5,0.5,0.5));
    tmp.push_back(ColorEntry("Lime",0,1,0));
    tmp.push_back(ColorEntry("Maroon",0.5,0,0));
    tmp.push_back(ColorEntry("Navy",0,0,0.5));
    tmp.push_back(ColorEntry("Olive",0.5,0.5,0.5));
    tmp.push_back(ColorEntry("Purple",0.5,0,0.5));
    tmp.push_back(ColorEntry("Red",1,0,0));
    tmp.push_back(ColorEntry("Silver",0.75,0.75,0.75));
    tmp.push_back(ColorEntry("Teal",0,0.5,0.5));
    tmp.push_back(ColorEntry("White",1,1,1));
    tmp.push_back(ColorEntry("Yellow",1,1,0));
    return tmp;
}

void vtkSlicerVRMenuButtonColorMode::ProcessColorModeEvents(int index)
{
    if(this->ColorTransferFunction==NULL)
    {
        vtkErrorMacro("SetTransferFunction first");
        return;
    }
    if(this->Range[0]==0&&this->Range[1]==0)
    {
        vtkErrorMacro("SetRange was not called yet");
        return;
    }
    this->ColorTransferFunction->RemoveAllPoints();
    this->ColorTransferFunction->AddRGBPoint(Range[0],
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Red,
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Green,
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Blue);

        this->ColorTransferFunction->AddRGBPoint(Range[1],
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Red,
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Green,
                                                vtkSlicerVRMenuButtonColorMode::Values[index].Blue);
        this->InvokeEvent(vtkSlicerVRMenuButtonColorMode::ColorModeChangedEvent);
}


const vtkSlicerVRMenuButtonColorMode::ColorModeList vtkSlicerVRMenuButtonColorMode::Values=CreateValues();
