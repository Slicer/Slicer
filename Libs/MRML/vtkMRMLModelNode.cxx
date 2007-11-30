/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"


#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"

#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkColorTransferFunction.h"

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLModelNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelNode");
  if(ret)
    {
    return (vtkMRMLModelNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelNode");
  if(ret)
    {
    return (vtkMRMLModelNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelNode;
}


//----------------------------------------------------------------------------
vtkMRMLModelNode::vtkMRMLModelNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLModelNode::~vtkMRMLModelNode()
{
}

//-----------------------------------------------------------
void vtkMRMLModelNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  int nnodes = this->GetNumberOfDisplayNodes();
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *mnode = scene->GetNodeByID(this->GetNthDisplayNodeID(n));
    if (mnode) 
      {
      vtkMRMLModelDisplayNode *node  = dynamic_cast < vtkMRMLModelDisplayNode *>(mnode);
      if (node)
        {
        // set input/output to/from display pipeline
        node->SetPolyData(this->GetPolyData());
        //this->SetAndObservePolyData(node->GetPolyData());
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{

  if (this->PolyData == vtkPolyData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    for (unsigned int i=0; i<this->DisplayNodes.size(); i++)
      {
      vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetNthDisplayNode(i));
      if (dnode != NULL)
        {
        dnode->SetPolyData(this->GetPolyData());
        }
      }
  }
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLModelNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelNode::SetAndObservePolyData(vtkPolyData *polyData)
{
  int ndisp = this->GetNumberOfDisplayNodes();
  for (int n=0; n<ndisp; n++) 
    {
    vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetNthDisplayNode(n));
    if (dnode)
      {
      dnode->SetPolyData(polyData);
      }
    }
  Superclass::SetAndObservePolyData(polyData);
}

//---------------------------------------------------------------------------
void vtkMRMLModelNode::AddPointScalars(vtkDataArray *array)
{
  if (array == NULL)
    {
    return;
    }
  if (this->PolyData == NULL)
    {
    vtkErrorMacro("AddPointScalars: No poly data on model " << this->GetName());
    return;
    }
  
  int numScalars = this->PolyData->GetPointData()->GetNumberOfArrays();
  vtkDebugMacro("Model node has " << numScalars << " point scalars now, adding " << array->GetName());
  if (numScalars > 0)
    {
    // add array
    this->PolyData->GetPointData()->AddArray(array);
    } 
  else
    {
    // set the scalars
    this->PolyData->GetPointData()->SetScalars(array);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelNode::AddCellScalars(vtkDataArray *array)
{
  if (array == NULL)
    {
    return;
    }
  if (this->PolyData == NULL)
    {
    vtkErrorMacro("AddCellScalars: No poly data on model " << this->GetName());
    return;
    }
  
  int numScalars = this->PolyData->GetCellData()->GetNumberOfArrays();
  vtkDebugMacro("Model node has " << numScalars << " cell scalars now, adding " << array->GetName());
  if (numScalars > 0)
    {
    // add array
    this->PolyData->GetCellData()->AddArray(array);
    } 
  else
    {
    // set the scalars
    this->PolyData->GetCellData()->SetScalars(array);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelNode::RemoveScalars(const char *scalarName)
{
  if (scalarName == NULL)
    {
    vtkErrorMacro("Scalar name is null");
    return;
    }
  if (this->PolyData == NULL)
    {
    vtkErrorMacro("RemoveScalars: No poly data on model " << this->GetName());
    return;
    }
  // try removing the array from the points first
  if (this->PolyData->GetPointData())
    {
    this->PolyData->GetPointData()->RemoveArray(scalarName);
    // it's a void method, how to check if it succeeded?
    }
  // try the cells
  if (this->PolyData->GetCellData())
    {
    this->PolyData->GetCellData()->RemoveArray(scalarName);
    }
}


//---------------------------------------------------------------------------
const char * vtkMRMLModelNode::GetActivePointScalarName(const char *type)
{
  if (this->PolyData == NULL)
    {
    return "";
    }
  if (this->PolyData->GetPointData() == NULL)
    {
    return "";
    }
  if (type == NULL)
    {
    vtkErrorMacro("GetActivePointScalarName: type is null");
    return "";
    }
  if (strcmp(type, "scalars") == 0)
    {
    if (this->PolyData->GetPointData()->GetScalars())
      {
      return this->PolyData->GetPointData()->GetScalars()->GetName();
      }
    }
  else if (strcmp(type, "vectors") == 0)
    {
    if (this->PolyData->GetPointData()->GetVectors())
      {
      return this->PolyData->GetPointData()->GetVectors()->GetName();
      }
    }
  else if (strcmp(type, "normals") == 0)
    {
    if (this->PolyData->GetPointData()->GetNormals())
      {
      return this->PolyData->GetPointData()->GetNormals()->GetName();
      }
    }
  else if (strcmp(type, "tcoords") == 0)
    {
    if (this->PolyData->GetPointData()->GetTCoords())      
      {
      return this->PolyData->GetPointData()->GetTCoords()->GetName();
      }
    }
  else if (strcmp(type, "tensors") == 0)
    {
    if (this->PolyData->GetPointData()->GetTensors())
      {
      return this->PolyData->GetPointData()->GetTensors()->GetName();
      }
    }
  else
    {
    vtkErrorMacro("Unknown point scalar type " << type);
    return "";
    }
  vtkDebugMacro("GetActivePointScalarName: node " << this->GetName() << " unable to get " << type << " data to get the name, so active name is returned as an empty string");
  return "";
}

//---------------------------------------------------------------------------
const char * vtkMRMLModelNode::GetActiveCellScalarName(const char *type)
{
  if (this->PolyData == NULL)
    {
    return "";
    }
  if (this->PolyData->GetCellData() == NULL)
    {
    return "";
    }
   if (type == NULL)
    {
    vtkErrorMacro("GetActiveCellScalarName: type is null");
    return "";
    }
  if (strcmp(type, "scalars") == 0)
    {
    if (this->PolyData->GetCellData()->GetScalars())
      {
      return this->PolyData->GetCellData()->GetScalars()->GetName();
      }
    }
  else if (strcmp(type, "vectors") == 0)
    {
    if (this->PolyData->GetCellData()->GetVectors())
      {
      return this->PolyData->GetCellData()->GetVectors()->GetName();
      }
    }
  else if (strcmp(type, "normals") == 0)
    {
    if (this->PolyData->GetCellData()->GetNormals())
      {
      return this->PolyData->GetCellData()->GetNormals()->GetName();
      }
    }
  else if (strcmp(type, "tcoords") == 0)
    {
    if (this->PolyData->GetCellData()->GetTCoords())      
      {
      return this->PolyData->GetCellData()->GetTCoords()->GetName();
      }
    }
  else if (strcmp(type, "tensors") == 0)
    {
    if (this->PolyData->GetCellData()->GetTensors())
      {
      return this->PolyData->GetCellData()->GetTensors()->GetName();
      }
    }
  else
    {
    vtkErrorMacro("Unknown point scalar type " << type);
    return "";
    }
  vtkDebugMacro("GetActiveCellScalarName: unable to get " << type << " data to get the name");
  return "";
}


//---------------------------------------------------------------------------
int vtkMRMLModelNode::SetActiveScalars(const char *scalarName, const char *typeName)
{
  int retval = -1;
  if (this->PolyData == NULL || scalarName == NULL)
    {
    if (this->PolyData == NULL)
      {
      vtkErrorMacro("SetActiveScalars: No poly data on model " << this->GetName());
      }
    else
      {
      vtkErrorMacro("SetActiveScalars: model " << this->GetName() << " scalar name is null");
      }
    return retval;
    }

  if (strcmp(scalarName, "") == 0)
    {
    return retval;
    }

  int attribute =  vtkDataSetAttributes::SCALARS;
  if (typeName != NULL && (strcmp(typeName, "") != 0))
    {
    for (int a = 0; a < vtkDataSetAttributes::NUM_ATTRIBUTES; a++)
      {
      if (strcmp(typeName, vtkDataSetAttributes::GetAttributeTypeAsString(a)) == 0)
        {
        attribute =  a;
        }
      }
    }
  // is it a point scalar?
  retval = this->SetActivePointScalars(scalarName, attribute);
  if (retval != -1)
    {
    vtkDebugMacro("Set active point " << typeName << " to " << scalarName << " (" <<
                  this->PolyData->GetPointData()->GetAttributeTypeAsString(retval) <<
                  ") on model " << this->GetName());
    if (this->GetModelDisplayNode() != NULL)
      {
      this->GetModelDisplayNode()->SetActiveScalarName(scalarName);
      }
    return retval;
    }
  // is it a cell scalar?
  retval =  this->SetActiveCellScalars(scalarName, attribute);
  if (retval != -1)
    {
    if (this->GetModelDisplayNode() != NULL)
      {
      this->GetModelDisplayNode()->SetActiveScalarName(scalarName);
      }
    vtkDebugMacro("Set active cell " << typeName << " to " << scalarName << " (" <<
                  this->PolyData->GetCellData()->GetAttributeTypeAsString(retval) << ") on model " <<
                  this->GetName());
    return retval;
    }
  vtkDebugMacro("Unable to find scalar attribute " << typeName << " " << scalarName << " on model " << this->GetName());
  return retval;
}

//---------------------------------------------------------------------------
int vtkMRMLModelNode::SetActivePointScalars(const char *scalarName, int attributeType)
{
  if (this->PolyData == NULL || scalarName == NULL)
    {
    vtkDebugMacro("No poly data on model " << this->GetName() << " or the scalar name is null");
    return -1;
    }
  if (this->PolyData->GetPointData() == NULL)
    {
    vtkDebugMacro("No point data on this model " << this->GetName());
    return -1;
    }
  // is this array present?
  if (this->PolyData->GetPointData()->HasArray(scalarName) == 0)
    {
    vtkDebugMacro("Model " << this->GetName() << " doesn't have a point data array named " << scalarName);
    return -1;
    }

  // is the array named scalarName already an active attribute? get it's index first
  int arrayIndex;
  this->PolyData->GetPointData()->GetArray(scalarName, arrayIndex);
  vtkDebugMacro("SetActivePointScalars: got the array index of " << scalarName << ": " << arrayIndex);
  // is it currently one of the attributes?
  int thisAttributeType = this->PolyData->GetPointData()->IsArrayAnAttribute(arrayIndex);
  vtkDebugMacro("\tarray index " << arrayIndex << " is an attribute type " << thisAttributeType);
  if (thisAttributeType != -1 && thisAttributeType == attributeType)
    {
    // it's already the active attribute
    return attributeType;
    }
  else
    {
    // it's not currently the active attribute, so set it
    if (this->PolyData->GetPointData()->SetActiveAttribute(arrayIndex, attributeType) != -1)
      {
      return attributeType;
      }
    else
      {
      return -1;
      }
    }
}

//---------------------------------------------------------------------------
int vtkMRMLModelNode::SetActiveCellScalars(const char *scalarName, int attributeType)
{
  if (this->PolyData == NULL || scalarName == NULL)
    {
    vtkDebugMacro("No poly data on model " << this->GetName() << " or the scalar name is null");
    return -1;
    }
  if (this->PolyData->GetCellData() == NULL)
    {
    vtkDebugMacro("No cell data on this model " << this->GetName());
    return -1;
    }

  // is this array present?
  if (this->PolyData->GetCellData()->HasArray(scalarName) == 0)
    {
    vtkDebugMacro("Model " << this->GetName() << " doesn't have a cell array named " << scalarName);
    return -1;
    }

  // is the array named scalarName already an active attribute? get it's index first
  int arrayIndex;
  this->PolyData->GetCellData()->GetArray(scalarName, arrayIndex);
  vtkDebugMacro("SetActiveCellScalars: got the array index of " << scalarName << ": " << arrayIndex);
  // is it currently one of the attributes?
  int thisAttributeType = this->PolyData->GetCellData()->IsArrayAnAttribute(arrayIndex);
  vtkDebugMacro("\tarray index " << arrayIndex << " is an attribute type " << thisAttributeType);
  if (thisAttributeType != -1 && thisAttributeType == attributeType)
    {
    // it's already the active attribute
    return attributeType;
    }
  else
    {
    // it's not currently the active attribute, so set it
    if (this->PolyData->GetCellData()->SetActiveAttribute(arrayIndex, attributeType) != -1)
      {
      return attributeType;
      }
    else
      {
      return -1;
      }
    }
}

//---------------------------------------------------------------------------
int vtkMRMLModelNode::CompositeScalars(const char* backgroundName, const char* overlayName,
                                       float overlayMin, float overlayMax,
                                       int showOverlayPositive, int showOverlayNegative,
                                       int reverseOverlay)
{

    if (backgroundName == NULL || overlayName == NULL)
      {
      vtkErrorMacro("CompositeScalars: one of the input array names is null");
      return 0;
      }

    bool haveCurvScalars = false;
    // is there a curv scalar in the composite?
    if (strstr(backgroundName, "curv") != NULL ||
        strstr(overlayName, "curv") != NULL)
      {
      haveCurvScalars = true;
      }
    
    // get the scalars to composite, putting any curv file in scalars 1
    vtkDataArray *scalars1, *scalars2;
    if (!haveCurvScalars ||
        strstr(backgroundName, "curv") != NULL)
      {
      scalars1 = this->PolyData->GetPointData()->GetScalars(backgroundName);
      scalars2 = this->PolyData->GetPointData()->GetScalars(overlayName);
      }
    else
      {
      scalars1 = this->PolyData->GetPointData()->GetScalars(overlayName);
      scalars2 = this->PolyData->GetPointData()->GetScalars(backgroundName);
      }
    if (scalars1 == NULL || scalars2 == NULL)
      {
      vtkErrorMacro("CompositeScalars: unable to find the named scalar arrays " << backgroundName << " and/or " << overlayName);
      return 0;
      }
    if (scalars1->GetNumberOfTuples() != scalars1->GetNumberOfTuples())
      {
      vtkErrorMacro("CompositeScalars: sizes of scalar arrays don't match");
      return 0;
      }
    // Get the number of elements and initialize the composed scalar
    // array.
    int cValues = 0;
    cValues = scalars1->GetNumberOfTuples();

    vtkFloatArray* composedScalars = vtkFloatArray::New();

    std::stringstream ss;
    ss << backgroundName;
    ss << "+";
    ss << overlayName;
    std::string composedName = std::string(ss.str());
    composedScalars->SetName(composedName.c_str());
    composedScalars->Allocate( cValues );
    composedScalars->SetNumberOfComponents( 1 );
   
    // For each value, check the overlay value. If it's < min, use
    // the background value. If we're reversing, reverse the overlay
    // value. If we're not showing one side, use the background
    // value. If we are showing curvature (and have it), the
    // background value is our curvature value.
    float overlayMid = 0.5 * (overlayMax - overlayMin) + overlayMin; // 2.0;
    vtkDebugMacro("CompositeScalars: using overlay mid = " << overlayMid);
    float overlay = 0.0;
    float background = 0.0;
    for( int nValue = 0; nValue < cValues; nValue++ )
      {
      background = scalars1->GetTuple1(nValue);
      overlay = scalars2->GetTuple1(nValue);
      
      if( reverseOverlay )
        {
        overlay = -overlay;
        }
      if( overlay > 0 && !showOverlayPositive )
        {
        overlay = 0;
        }
      
      if( overlay < 0 && !showOverlayNegative )
        {
        overlay = 0;
        }
      
      // Insert the appropriate color into the composed array.
      if( overlay < overlayMin &&
          overlay > -overlayMin )
        {
        composedScalars->InsertNextValue( background );
        }
      else
        {
        composedScalars->InsertNextValue( overlay );
        }
      }
    
    // set up a colour node
    vtkMRMLProceduralColorNode *colorNode = vtkMRMLProceduralColorNode::New();
    colorNode->SetName(composedName.c_str());
    // set the type to avoid error messages when copy it, as the default is -1
    colorNode->SetType(vtkMRMLFreeSurferProceduralColorNode::Custom);
    vtkColorTransferFunction *func = colorNode->GetColorTransferFunction();

    // adapted from FS code that assumed that one scalar was curvature, the
    // other heat overlay
    const double EPS = 0.00001; // epsilon
    double curvatureMin = 0;
    
    if (haveCurvScalars)
      {
      curvatureMin = 0.5;
      }
    bool bUseGray = true;
    if( overlayMin <= curvatureMin )
      {
      curvatureMin = overlayMin - EPS;
      bUseGray = false;
      }
    func->AddRGBPoint( -overlayMax, 0, 1, 1 );
    func->AddRGBPoint( -overlayMid, 0, 0, 1 );
    func->AddRGBPoint( -overlayMin, 0, 0, 1 );
    
    if( bUseGray && overlayMin != 0 )
      {
      func->AddRGBPoint( -overlayMin + EPS, 0.5, 0.5, 0.5 );
      if( haveCurvScalars)
        {
        func->AddRGBPoint( -curvatureMin - EPS, 0.5, 0.5, 0.5 );
        }
      }
    if( haveCurvScalars && overlayMin != 0 )
      {
      func->AddRGBPoint( -curvatureMin, 0.6, 0.6, 0.6 );
      func->AddRGBPoint(  0,            0.6, 0.6, 0.6 );
      func->AddRGBPoint(  EPS,          0.4, 0.4, 0.4 );
      func->AddRGBPoint(  curvatureMin, 0.4, 0.4, 0.4 );
      }
    
    if ( bUseGray && overlayMin != 0 )
      {
      if( haveCurvScalars )
        {
        func->AddRGBPoint( curvatureMin + EPS, 0.5, 0.5, 0.5 );
        }
      func->AddRGBPoint( overlayMin - EPS, 0.5, 0.5, 0.5 );
      }

    func->AddRGBPoint( overlayMin, 1, 0, 0 );
    func->AddRGBPoint( overlayMid, 1, 0, 0 );
    func->AddRGBPoint( overlayMax, 1, 1, 0 );
    
    func->Build();
    
    // use the new colornode
    this->Scene->AddNode(colorNode);
    vtkDebugMacro("CompositeScalars: created color transfer function, and added proc color node to scene, id = " << colorNode->GetID());
    if (colorNode->GetID() != NULL)
      {
      this->GetModelDisplayNode()->SetAndObserveColorNodeID(colorNode->GetID());
      this->GetModelDisplayNode()->SetScalarRange(-overlayMax, overlayMax);
      }
    
    // add the new scalars
    this->AddPointScalars(composedScalars);

    // make them active
    this->GetModelDisplayNode()->SetActiveScalarName(composedName.c_str());

    // clean up
    colorNode->Delete();
    colorNode = NULL;
    composedScalars->Delete();
    composedScalars = NULL;
    
    return 1;
}

