#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkPiecewiseFunction.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>

vtkMRMLVolumeRenderingNode* vtkMRMLVolumeRenderingNode::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingNode");
    if(ret)
    {
        return (vtkMRMLVolumeRenderingNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLVolumeRenderingNode;
}

vtkMRMLNode* vtkMRMLVolumeRenderingNode::CreateNodeInstance(void)
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingNode");
    if(ret)
    {
        return (vtkMRMLVolumeRenderingNode*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkMRMLVolumeRenderingNode;
}

vtkMRMLVolumeRenderingNode::vtkMRMLVolumeRenderingNode(void)
{   
    this->DebugOff();
    this->VolumeProperty=vtkVolumeProperty::New();
    //Standard is no Labelmap
    this->IsLabelMapOff();
    //Standard is 3D-Volume Texture Mapper
    this->Mapper=0;
    this->CroppingEnabled=0;
    for(int i=0;i<COUNT_CROPPING_REGION_PLANES;i++)
    {
        this->CroppingRegionPlanes[i]=0;
    }
}

vtkMRMLVolumeRenderingNode::~vtkMRMLVolumeRenderingNode(void)
{
    if(this->VolumeProperty)
    {
        this->VolumeProperty->Delete();
        this->VolumeProperty=NULL;
    }
}
void vtkMRMLVolumeRenderingNode::WriteXML(ostream& of, int nIndent)
{
    // Write all attributes not equal to their defaults

    Superclass::WriteXML(of, nIndent);

    //vtkIndent indent(nIndent);
    of << " isLabelmap=\""<<this->GetIsLabelMap() <<"\"";
    of << " interpolation=\"" <<this->VolumeProperty->GetInterpolationType()<< "\"";
    of << " shade=\"" <<this->VolumeProperty->GetShade()<< "\"";
    of << " diffuse=\"" <<this->VolumeProperty->GetDiffuse()<< "\"";
    of << " ambient=\"" <<this->VolumeProperty->GetAmbient()<< "\"";
    of << " specular=\"" <<this->VolumeProperty->GetSpecular()<< "\"";
    of << " specularPower=\"" <<this->VolumeProperty->GetSpecularPower()<<"\"";
    of << " references=\""<<this->References.size()<<" ";
    for(unsigned int i=0;i<this->References.size();i++)
    {
        of<<this->References.at(i);
        if(i!=(this->References.size()-1))
        {
            of<<" ";
        }
    }
    of<<"\"";

    //Save cropping information
    of << " croppingEnabled=\""<< this->CroppingEnabled<< "\"";
    of << " croppingRegionPlanes=\"";
    for(int i=0;i<COUNT_CROPPING_REGION_PLANES;i++)
    {
        of<<this->CroppingRegionPlanes[i];
        if(i!=COUNT_CROPPING_REGION_PLANES-1)
        {
            of<<" ";
        }

    }
    of <<"\"";

    //Only write opacities when LabelMap
    //if(this->GetIsLabelMap())
    //{
    //    vtkLabelMapPiecewiseFunction *opacity=vtkLabelMapPiecewiseFunction::SafeDownCast(this->VolumeProperty->GetScalarOpacity());
    //    of<<" opacityLabelMap=\""<<opacity->GetSaveString()<< "\"";

    //}
    //else
    //{
        of << " scalarOpacity=\"" << this->GetPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity())  << "\"";
        of << " gradientOpacity=\"" <<this->GetPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity())<< "\"";
        of << " colorTransfer=\"" <<this->getColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction())<< "\"";

   // }


}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingNode::ReadXMLAttributes(const char** atts)
{
    Superclass::ReadXMLAttributes(atts);

    const char* attName;
    const char* attValue;
    while (*atts!=NULL){
        attName= *(atts++);
        attValue= *(atts++);
        if(!strcmp(attName,"isLabelmap"))
        {
            int lm;
            std::stringstream ss;
            ss<<attValue;
            ss>>lm;
            this->SetIsLabelMap(lm);
        }
        if(!strcmp(attName,"references"))
        {
            int size=0;
            std::stringstream ss;
            ss << attValue;
            ss>>size;
            for(int i=0;i<size;i++)
            {
                std::string str;
                ss>>str;
                this->AddReference(str.c_str());
            }
        }
        if(!strcmp(attName,"scalarOpacity"))
        {
            if(this->GetIsLabelMap()==1)
            {
                vtkErrorMacro("labelmaps don't have a scalar opacity use opacity labelmap instead"); 


            }
            else
            {
                vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
                this->GetPiecewiseFunctionFromString(attValue,scalarOpacity);
                this->VolumeProperty->SetScalarOpacity(scalarOpacity);
                scalarOpacity->Delete();
            }
        }
        else if(!strcmp(attName,"gradientOpacity"))
        {
            //double check that we don't have a labelmap missmatch
            if(this->GetIsLabelMap()==1)
            {
                vtkErrorMacro("Labelmaps don't have a gradient opacity");
            }
            vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
            this->GetPiecewiseFunctionFromString(attValue,gradientOpacity);
            this->VolumeProperty->SetGradientOpacity(gradientOpacity);
            gradientOpacity->Delete();
        }
        else if(!strcmp(attName,"colorTransfer"))
        {
            //double check that we don't have a labelmap missmatch
            if(this->GetIsLabelMap()==1)
            {
                vtkErrorMacro("Labelmaps don't have a color transfer function");
            }
            vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
            this->GetColorTransferFunction(attValue,colorTransfer);
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
        else if (!strcmp(attName,"croppingEnabled"))
        {
            std::stringstream ss;
            ss<<attValue;
            ss>>this->CroppingEnabled;  
        }
        else if (!strcmp(attName,"croppingRegionPlanes"))
        {
            std::stringstream ss;
            ss<<attValue;
            for(int i=0;i<COUNT_CROPPING_REGION_PLANES;i++)
            {
                ss>>this->CroppingRegionPlanes[i];          
            }
        }

        ////special behavior for labelmaps
        //else if(!strcmp(attName,"opacityLabelMap"))
        //{
        //    if(this->IsLabelMap==0)
        //    {
        //        vtkErrorMacro("grayscale volumes don't have a opacityLabelMap");
        //    }
        //    vtkLabelMapPiecewiseFunction *scalarOpacityLabelmap=vtkLabelMapPiecewiseFunction::New();
        //    scalarOpacityLabelmap->FillFromString(attValue);
        //    this->VolumeProperty->SetScalarOpacity(scalarOpacityLabelmap);
        //    scalarOpacityLabelmap->Delete();
        //}
    }//while
    vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVolumeRenderingNode::Copy(vtkMRMLNode *anode)
{
    Superclass::Copy(anode);
    this->CopyParameterset(anode);

}
void vtkMRMLVolumeRenderingNode::CopyParameterset(vtkMRMLNode *anode)
{
    //cast
    vtkMRMLVolumeRenderingNode *node = (vtkMRMLVolumeRenderingNode *) anode;

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
    this->SetMapper(node->Mapper);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingNode::PrintSelf(ostream& os, vtkIndent indent)
{

    Superclass::PrintSelf(os,indent);
    os<<indent<<"VolumeProperty: ";
    this->VolumeProperty->PrintSelf(os,indent.GetNextIndent());
    os<<indent<<"Mapper: ";
    if(this->Mapper==Texture)
    {
        os<<"vtkSlicerVolumeTextureMapper3D";
    }
    else
    {
        os<<"FixedRayCastMapping";
    }
    os<<indent<<"References: ";
    for(unsigned int i=0;i<this->References.size();i++)
    {
        os<<this->References.at(i);
        if(i!=(this->References.size()-1))
        {
            os<<" ";
        }
    }
}

//-----------------------------------------------------------

void vtkMRMLVolumeRenderingNode::UpdateScene(vtkMRMLScene *scene)
{
    Superclass::UpdateScene(scene);
}


//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    return;
}


std::string vtkMRMLVolumeRenderingNode::GetPiecewiseFunctionString(vtkPiecewiseFunction* function)
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
std::string  vtkMRMLVolumeRenderingNode::getColorTransferFunctionString(vtkColorTransferFunction* function)
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
void vtkMRMLVolumeRenderingNode::GetPiecewiseFunctionFromString(std::string str,vtkPiecewiseFunction* result)
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
void vtkMRMLVolumeRenderingNode::GetColorTransferFunction(std::string str, vtkColorTransferFunction* result)
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

void vtkMRMLVolumeRenderingNode::AddReference(std::string id)
{
    //test if we already have a reference
    if(this->HasReference(id))
    {
        return;
    }
    else 
    {
        this->References.push_back(id);
    }

}
bool vtkMRMLVolumeRenderingNode::HasReference(std::string id)
{
    //loop over vector and comparing
    for(unsigned int i=0;i<this->References.size();i++)
    {
        if(strcmp(this->References.at(i).c_str(),id.c_str())==0)
        {
            return true;
        }
    }
    return false;
}
void vtkMRMLVolumeRenderingNode::RemoveReference(std::string id)
{
    vtkErrorMacro("Not yet implemented");

}

