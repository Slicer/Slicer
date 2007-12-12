#include "vtkLabelMapPiecewiseFunction.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTimerLog.h"
#include "vtkMRMLScalarVolumeNode.h"
#include <sstream>

vtkLabelMapPiecewiseFunction::vtkLabelMapPiecewiseFunction(void)
{
    this->Opacities=NULL;
    this->ColorNode=NULL;
}

vtkLabelMapPiecewiseFunction::~vtkLabelMapPiecewiseFunction(void)
{
    if(this->Opacities)
    {
        delete[] this->Opacities;
        this->Opacities=NULL;
    }
}

vtkLabelMapPiecewiseFunction* vtkLabelMapPiecewiseFunction::New(void)
{
    // First try to create the object from the vtkObjectFactory

    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLabelMapPiecewiseFunction");
    if(ret)
    {
        return (vtkLabelMapPiecewiseFunction*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkLabelMapPiecewiseFunction;
}

void vtkLabelMapPiecewiseFunction::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}
void vtkLabelMapPiecewiseFunction::Init(vtkMRMLScalarVolumeNode *node,double opacity, int threshold)
{   
    vtkTimerLog *timer1=vtkTimerLog::New();
    timer1->StartTimer();

    vtkDebugMacro("threshold without effect at the moment");
    //test if inputdata is valid
    if(node==NULL||opacity<0||opacity>1||threshold<0)
    {
        vtkErrorMacro("Input is not valid");
        return;
    }
    vtkImageData *imageData=node->GetImageData();
    if(imageData==NULL)
    {
        vtkErrorMacro("No image data");
        return;
    }
    if(imageData->GetPointData()==NULL)
    {
        vtkErrorMacro("No Point data");
        return;
    }
    if(imageData->GetPointData()->GetScalars()==NULL)
    {
        vtkErrorMacro("No Scalars for Point data");
        return;
    }
    if (node->GetLabelMap()==0)
    {
        vtkErrorMacro("this is not a labelMap");
        return;
    }
    if(node->GetVolumeDisplayNode()==NULL)
    {
        vtkErrorMacro("No Volume Display Node");
        return;
    }
    if(node->GetVolumeDisplayNode()->GetColorNode()==NULL)
    {
        vtkErrorMacro("No Color Node");
        return;
    }
    vtkLookupTable *lookup=node->GetVolumeDisplayNode()->GetColorNode()->GetLookupTable();
    if(lookup==NULL)
    {
        vtkErrorMacro("No LookupTable");
        return;
    }
    this->AdjustRange(lookup->GetTableRange());
    Opacities=new double[lookup->GetNumberOfTableValues()];
    this->ColorNode=node->GetVolumeDisplayNode()->GetColorNode();
    this->Size=(int) (lookup->GetTableRange()[1]-lookup->GetTableRange()[0]);
    int index=0;
    for (int i=(int)lookup->GetTableRange()[0];i<lookup->GetTableRange()[1];i++)
    {
        this->AddPoint(i-.5,0);
        this->AddPoint(i-.49,opacity);
        this->AddPoint(i+.5,0);
        this->AddPoint(i+.49,opacity);
        this->Opacities[index++]=opacity;

    }
    timer1->StopTimer();
    vtkDebugMacro("Init Labelmap Piecewise calculated in "<<timer1->GetElapsedTime()<<"seconds");
    timer1->Delete();

}
void vtkLabelMapPiecewiseFunction::EditLabel(int index,double opacity)
{
    if(index>=Size)
    {
        return;
    }
    this->Opacities[index]=opacity;
            this->AddPoint(index-.5,0);
        this->AddPoint(index-.49,opacity);
        this->AddPoint(index+.5,0);
        this->AddPoint(index+.49,opacity);

}
double vtkLabelMapPiecewiseFunction::GetLabel(int index)
{
    if(index>=Size)
    {
        return 0.;
    }
    return this->Opacities[index];
}
std::string vtkLabelMapPiecewiseFunction::GetSaveString()
{
    std::stringstream ss;
    ss<<this->Size;
    for(int i=0;i<this->Size;i++)
    {
        ss<<" "<<(int)(this->Opacities[i]*100);
    }
    return ss.str();
}
void vtkLabelMapPiecewiseFunction::FillFromString(std::string str)
{
    if(this->Opacities!=NULL)
    {
        vtkErrorMacro("Fill from String is not allowed when you already used the opacities");
        return;
    }
    std::stringstream ss;
    ss<<str;
    ss>>this->Size;
    int tmp=0;
    this->Opacities=new double[this->Size];

    for(int i=0;i<this->Size;i++)
    {
        ss>>tmp;
        this->Opacities[i]=tmp*100.;
        this->EditLabel(i,tmp);
    }
}
