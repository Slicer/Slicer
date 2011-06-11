#include "vtkMRMLNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"
#include "vtkPiecewiseFunction.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkMRMLVolumePropertyNode::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumePropertyNode");
    if(ret)
    {
        return (vtkMRMLVolumePropertyNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLVolumePropertyNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLVolumePropertyNode::CreateNodeInstance(void)
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumePropertyNode");
    if(ret)
    {
        return (vtkMRMLVolumePropertyNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLVolumePropertyNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::vtkMRMLVolumePropertyNode(void)
{
    this->VolumeProperty = NULL;

    vtkVolumeProperty *node  = vtkVolumeProperty::New();
    vtkSetAndObserveMRMLObjectMacro(this->VolumeProperty, node);
    vtkObserveMRMLObjectMacro(node->GetScalarOpacity());
    vtkObserveMRMLObjectMacro(node->GetGradientOpacity());
    vtkObserveMRMLObjectMacro(node->GetRGBTransferFunction());
    node->Delete();

    this->HideFromEditors = 0;
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode::~vtkMRMLVolumePropertyNode(void)
{
  if(this->VolumeProperty)
    {
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetScalarOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetGradientOpacity());
    vtkUnObserveMRMLObjectMacro(this->VolumeProperty->GetRGBTransferFunction());
    vtkSetAndObserveMRMLObjectMacro(this->VolumeProperty, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::WriteXML(ostream& of, int nIndent)
{
    // Write all attributes not equal to their defaults

    Superclass::WriteXML(of, nIndent);

    //vtkIndent indent(nIndent);
    of << " interpolation=\"" <<this->VolumeProperty->GetInterpolationType()<< "\"";
    of << " shade=\"" <<this->VolumeProperty->GetShade()<< "\"";
    of << " diffuse=\"" <<this->VolumeProperty->GetDiffuse()<< "\"";
    of << " ambient=\"" <<this->VolumeProperty->GetAmbient()<< "\"";
    of << " specular=\"" <<this->VolumeProperty->GetSpecular()<< "\"";
    of << " specularPower=\"" <<this->VolumeProperty->GetSpecularPower()<<"\"";
    of << " scalarOpacity=\"" << this->GetPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity())  << "\"";
    of << " gradientOpacity=\"" <<this->GetPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity())<< "\"";
    of << " colorTransfer=\"" <<this->GetColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction())<< "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ReadXMLAttributes(const char** atts)
{
    int disabledModify = this->StartModify();

    Superclass::ReadXMLAttributes(atts);

    const char* attName;
    const char* attValue;
    while (*atts!=NULL){
        attName= *(atts++);
        attValue= *(atts++);
        if(!strcmp(attName,"scalarOpacity"))
        {
          vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
          this->GetPiecewiseFunctionFromString(attValue,scalarOpacity);
          this->VolumeProperty->SetScalarOpacity(scalarOpacity);
          scalarOpacity->Delete();
        }
        else if(!strcmp(attName,"gradientOpacity"))
        {
          vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
          this->GetPiecewiseFunctionFromString(attValue,gradientOpacity);
          this->VolumeProperty->SetGradientOpacity(gradientOpacity);
          gradientOpacity->Delete();
        }
        else if(!strcmp(attName,"colorTransfer"))
        {
          vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
          this->GetColorTransferFunctionFromString(attValue,colorTransfer);
          this->VolumeProperty->SetColor(colorTransfer);
          colorTransfer->Delete();
        }
        else if(!strcmp(attName,"interpolation"))
        {
          int interpolation;
          std::stringstream ss;
          ss <<attValue;
          ss>>interpolation;
          this->VolumeProperty->SetInterpolationType(interpolation);
        }
        else if(!strcmp(attName,"shade"))
        {
            int shade;
            std::stringstream ss;
            ss <<attValue;
            ss>>shade;
            this->VolumeProperty->SetShade(shade);
        }
        else if(!strcmp(attName,"diffuse"))
        {
            double diffuse;
            std::stringstream ss;
            ss<<attValue;
            ss>>diffuse;
            this->VolumeProperty->SetDiffuse(diffuse);
        }
        else if(!strcmp(attName,"ambient"))
        {
            double ambient;
            std::stringstream ss;
            ss<<attValue;
            ss>>ambient;
            this->VolumeProperty->SetAmbient(ambient);
        }
        else if(!strcmp(attName,"specular"))
        {
            double specular;
            std::stringstream ss;
            ss<<attValue;
            ss>>specular;
            this->VolumeProperty->SetSpecular(specular);
        }
        else if(!strcmp(attName,"specularPower"))
        {
            int specularPower;
            std::stringstream ss;
            ss<<attValue;
            ss>>specularPower;
            this->VolumeProperty->SetSpecularPower(specularPower);
        }//else if
    }//while

  this->EndModify(disabledModify);}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVolumePropertyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  this->CopyParameterSet(anode);
  
  this->EndModify(disabledModify);
}

void vtkMRMLVolumePropertyNode::CopyParameterSet(vtkMRMLNode *anode)
{
    //cast
    vtkMRMLVolumePropertyNode *node = (vtkMRMLVolumePropertyNode *) anode;

    this->VolumeProperty->SetIndependentComponents(node->VolumeProperty->GetIndependentComponents());
    this->VolumeProperty->SetInterpolationType(node->VolumeProperty->GetInterpolationType());

    //VolumeProperty
    for (int i=0;i<VTK_MAX_VRCOMP;i++)
    {
        this->VolumeProperty->SetComponentWeight(i,node->GetVolumeProperty()->GetComponentWeight(i));
        //TODO problem no set method
        // vtkPiecewiseFunction *gray=node->GetVolumeProperty()->GetGrayTransferFunction=());L
        //   this->VolumeProperty->SetGry
        //TODO problem no set ColorChannels Method
        //this->VolumeProperty->SetCGetColorChannels(
        //mapping functions
        vtkColorTransferFunction *rgbTransfer=vtkColorTransferFunction::New();
        rgbTransfer->DeepCopy(node->GetVolumeProperty()->GetRGBTransferFunction(i));
        this->VolumeProperty->SetColor(i,rgbTransfer);
        rgbTransfer->Delete();

        vtkPiecewiseFunction *scalar=vtkPiecewiseFunction::New();
        scalar->DeepCopy(node->GetVolumeProperty()->GetScalarOpacity(i));
        this->VolumeProperty->SetScalarOpacity(i,scalar);
        scalar->Delete();
        this->VolumeProperty->SetScalarOpacityUnitDistance(i,this->VolumeProperty->GetScalarOpacityUnitDistance(i));

        vtkPiecewiseFunction *gradient=vtkPiecewiseFunction::New();
        gradient->DeepCopy(node->GetVolumeProperty()->GetGradientOpacity(i));
        this->VolumeProperty->SetGradientOpacity(i,gradient);
        gradient->Delete();
        //TODO Copy default gradient?
        this->VolumeProperty->SetDisableGradientOpacity(i,node->GetVolumeProperty()->GetDisableGradientOpacity(i));
        this->VolumeProperty->SetShade(i,node->GetVolumeProperty()->GetShade(i));
        this->VolumeProperty->SetAmbient(node->VolumeProperty->GetAmbient(i));
        this->VolumeProperty->SetDiffuse(node->VolumeProperty->GetDiffuse(i));
        this->VolumeProperty->SetSpecular(node->VolumeProperty->GetSpecular(i));
        this->VolumeProperty->SetSpecularPower(node->VolumeProperty->GetSpecularPower(i));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::PrintSelf(ostream& os, vtkIndent indent)
{

    Superclass::PrintSelf(os,indent);
    os<<indent<<"VolumeProperty: ";
    this->VolumeProperty->PrintSelf(os,indent.GetNextIndent());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLVolumePropertyNode::GetPiecewiseFunctionString(vtkPiecewiseFunction* function)
{
    std::stringstream resultStream;
    int arraysize=function->GetSize()*2;
    double *data=function->GetDataPointer();
    double *it=data;
    //write header
    resultStream<<arraysize;
    for (int i=0;i<arraysize;i++)
    {
        resultStream<<" ";
        resultStream<<*it;
        it++;

    }
    return resultStream.str();
}

//---------------------------------------------------------------------------
std::string  vtkMRMLVolumePropertyNode::GetColorTransferFunctionString(vtkColorTransferFunction* function)
{

    //maybe size*4
    std::stringstream resultStream;
    //resultStream.str
    int arraysize=function->GetSize()*4;
    double *data=function->GetDataPointer();
    double *it=data;
    //write header
    resultStream<<arraysize;
    for (int i=0;i<arraysize;i++)
    {
        resultStream<<" ";
        resultStream<<*it;
        it++;

    }
    return resultStream.str();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(std::string str,vtkPiecewiseFunction* result)
{
    std::stringstream stream;
    stream<<str;
    int size=0;

    stream>>size;

    if (size==0)
    {
        return;
    }
    double *data=new double[size];
    for(int i=0;i<size;i++)
    {
        stream>>data[i];
    }
    result->FillFromDataPointer(size/2,data);
    delete[] data;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode::GetColorTransferFunctionFromString(std::string str, vtkColorTransferFunction* result)
{
    std::stringstream stream;
    stream<<str;
    int size=0;

    stream>>size;

    if (size==0)
    {
        return;
    }
    double *data=new double[size];
    for(int i=0;i<size;i++)
    {
        stream>>data[i];
    }
    result->FillFromDataPointer(size/4,data);
    delete[] data;

}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLVolumePropertyNode::CreateDefaultStorageNode()
{
  return vtkMRMLVolumePropertyStorageNode::New();
}
