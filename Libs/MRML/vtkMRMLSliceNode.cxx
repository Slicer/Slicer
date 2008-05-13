/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScene.h"

#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include "vnl/vnl_double_3.h"

//------------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceNode");
  if(ret)
    {
      return (vtkMRMLSliceNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSliceNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceNode");
  if(ret)
    {
      return (vtkMRMLSliceNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceNode;
}

//----------------------------------------------------------------------------
// Constructor
vtkMRMLSliceNode::vtkMRMLSliceNode()
{
    // set by user
  this->SliceToRAS = vtkMatrix4x4::New();
  this->SliceToRAS->Identity();

  this->OrientationString = NULL;

    // calculated by UpdateMatrices()
  this->XYToSlice = vtkMatrix4x4::New();
  this->XYToRAS = vtkMatrix4x4::New();

  // set the default field of view to a convenient size for looking 
  // at slices through human heads (a 1 pixel thick slab 25x25 cm)
  // TODO: how best to represent this as a slab rather than infinitessimal slice?
  this->FieldOfView[0] = 250.0;
  this->FieldOfView[1] = 250.0;
  this->FieldOfView[2] = 1.0;

  this->Dimensions[0] = 256;
  this->Dimensions[1] = 256;
  this->Dimensions[2] = 1;
  this->SliceVisible = 0;
  this->WidgetVisible = 0;

  this->LayoutGridColumns = 1;
  this->LayoutGridRows = 1;

  this->SetOrientationToAxial();
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode::~vtkMRMLSliceNode()
{
  if ( this->SliceToRAS != NULL) 
    {
    this->SliceToRAS->Delete();
    }
  if ( this->XYToSlice != NULL) 
    {
    this->XYToSlice->Delete();
    }
  if ( this->XYToRAS != NULL) 
    {
    this->XYToRAS->Delete();
    }
  if ( this->OrientationString )
    {
    delete [] this->OrientationString;
    }
  this->SetLayoutName(NULL);
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToReformat()
{
    // Don't need to do anything.  Leave the matrices where they were
    // so the reformat starts where you were.

    this->SetOrientationString( "Reformat" );
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToAxial()
{
    // Px -> Patient Left
    this->SliceToRAS->SetElement(0, 0, -1.0);
    this->SliceToRAS->SetElement(1, 0,  0.0);
    this->SliceToRAS->SetElement(2, 0,  0.0);
    // Py -> Patient Anterior
    this->SliceToRAS->SetElement(0, 1,  0.0);
    this->SliceToRAS->SetElement(1, 1,  1.0);
    this->SliceToRAS->SetElement(2, 1,  0.0);
    // Pz -> Patient Inferior
    this->SliceToRAS->SetElement(0, 2,  0.0);
    this->SliceToRAS->SetElement(1, 2,  0.0);
    this->SliceToRAS->SetElement(2, 2,  1.0);

    this->SetOrientationString( "Axial" );
    this->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToSagittal()
{
    // Px -> Patient Posterior
    this->SliceToRAS->SetElement(0, 0,  0.0);
    this->SliceToRAS->SetElement(1, 0, -1.0);
    this->SliceToRAS->SetElement(2, 0,  0.0);
    // Py -> Patient Inferior
    this->SliceToRAS->SetElement(0, 1,  0.0);
    this->SliceToRAS->SetElement(1, 1,  0.0);
    this->SliceToRAS->SetElement(2, 1,  1.0);
    // Pz -> Patient Right
    this->SliceToRAS->SetElement(0, 2,  1.0);
    this->SliceToRAS->SetElement(1, 2,  0.0);
    this->SliceToRAS->SetElement(2, 2,  0.0);

    this->SetOrientationString( "Sagittal" );
    this->UpdateMatrices();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToCoronal()
{
    // Px -> Patient Left
    this->SliceToRAS->SetElement(0, 0, -1.0);
    this->SliceToRAS->SetElement(1, 0,  0.0);
    this->SliceToRAS->SetElement(2, 0,  0.0);
    // Py -> Patient Inferior
    this->SliceToRAS->SetElement(0, 1,  0.0);
    this->SliceToRAS->SetElement(1, 1,  0.0);
    this->SliceToRAS->SetElement(2, 1,  1.0);
    // Pz -> Patient Anterior
    this->SliceToRAS->SetElement(0, 2,  0.0);
    this->SliceToRAS->SetElement(1, 2,  1.0);
    this->SliceToRAS->SetElement(2, 2,  0.0);

    this->SetOrientationString( "Coronal" );
    this->UpdateMatrices();
}

//----------------------------------------------------------------------------
// Local helper to compare matrices -- TODO: is there a standard version of this?
int vtkMRMLSliceNode::Matrix4x4AreEqual(vtkMatrix4x4 *m1, vtkMatrix4x4 *m2)
{
  int i,j;
  for (i = 0; i < 4; i++)
    {
    for (j = 0; j < 4; j++)
      {
      if ( m1->GetElement(i, j) != m2->GetElement(i, j) )
        {
        return 0;
        }
      }
    }
    return 1;
}

//----------------------------------------------------------------------------
//  Set the SliceToRAS matrix by the postion and orientation of the locator
//
void vtkMRMLSliceNode::SetSliceToRASByNTP (double Nx, double Ny, double Nz,
                         double Tx, double Ty, double Tz,
                         double Px, double Py, double Pz,
                         int Orientation)
{
    vnl_double_3 n, t, c;
    vnl_double_3 negN, negT, negC;

    n[0] = Nx; 
    n[1] = Ny; 
    n[2] = Nz; 
    t[0] = Tx; 
    t[1] = Ty; 
    t[2] = Tz; 

    // Ensure N, T orthogonal:
    //    C = N x T
    //    T = C x N
    c = vnl_cross_3d(n, t);
    t = vnl_cross_3d(c, n);

    // Ensure vectors are normalized
    n.normalize();
    t.normalize();
    c.normalize();

    // Get negative vectors
    negN = -n;
    negT = -t;
    negC = -c;

    this->SliceToRAS->Identity();
    // Tip location
    this->SliceToRAS->SetElement(0, 3, Px);
    this->SliceToRAS->SetElement(1, 3, Py);
    this->SliceToRAS->SetElement(2, 3, Pz);

    switch (Orientation)
    {
        // para-Axial 
        case 0: 
            // N
            this->SliceToRAS->SetElement(0, 2, n[0]);
            this->SliceToRAS->SetElement(1, 2, n[1]);
            this->SliceToRAS->SetElement(2, 2, n[2]);

            // C 
            this->SliceToRAS->SetElement(0, 1, c[0]);
            this->SliceToRAS->SetElement(1, 1, c[1]);
            this->SliceToRAS->SetElement(2, 1, c[2]);
            // T 
            this->SliceToRAS->SetElement(0, 0, t[0]);
            this->SliceToRAS->SetElement(1, 0, t[1]);
            this->SliceToRAS->SetElement(2, 0, t[2]);

            break;

        // para-Sagittal 
        case 1: 
            // T
            this->SliceToRAS->SetElement(0, 2, t[0]);
            this->SliceToRAS->SetElement(1, 2, t[1]);
            this->SliceToRAS->SetElement(2, 2, t[2]);

            // negN 
            this->SliceToRAS->SetElement(0, 1, negN[0]);
            this->SliceToRAS->SetElement(1, 1, negN[1]);
            this->SliceToRAS->SetElement(2, 1, negN[2]);
            // negC 
            this->SliceToRAS->SetElement(0, 0, negC[0]);
            this->SliceToRAS->SetElement(1, 0, negC[1]);
            this->SliceToRAS->SetElement(2, 0, negC[2]);

            break;

        // para-Coronal 
        case 2: 
            // C 
            this->SliceToRAS->SetElement(0, 2, c[0]);
            this->SliceToRAS->SetElement(1, 2, c[1]);
            this->SliceToRAS->SetElement(2, 2, c[2]);
            // negN 
            this->SliceToRAS->SetElement(0, 1, negN[0]);
            this->SliceToRAS->SetElement(1, 1, negN[1]);
            this->SliceToRAS->SetElement(2, 1, negN[2]);
            // T 
            this->SliceToRAS->SetElement(0, 0, t[0]);
            this->SliceToRAS->SetElement(1, 0, t[1]);
            this->SliceToRAS->SetElement(2, 0, t[2]);

            break;
    }

    this->UpdateMatrices();  
}

//----------------------------------------------------------------------------
//  Calculate XYToSlice and XYToRAS 
//  Inputs: Dimenionss, FieldOfView, SliceToRAS
//
void vtkMRMLSliceNode::UpdateMatrices()
{
    double spacing[3];
    unsigned int i;
    vtkMatrix4x4 *xyToSlice = vtkMatrix4x4::New();
    vtkMatrix4x4 *xyToRAS = vtkMatrix4x4::New();

    // the mapping from XY output slice pixels to Slice Plane coordinate
    xyToSlice->Identity();
    for (i = 0; i < 3; i++)
      {
      spacing[i] = this->FieldOfView[i] / this->Dimensions[i];
      xyToSlice->SetElement(i, i, spacing[i]);
      xyToSlice->SetElement(i, 3, -this->FieldOfView[i] / 2.);
      }
    //vtkWarningMacro( << "FieldOfView[2] = " << this->FieldOfView[2] << ", Dimensions[2] = " << this->Dimensions[2] );
    //xyToSlice->SetElement(2, 2, 1.);

    xyToSlice->SetElement(2, 3, 0.);

    // the mapping from slice plane coordinates to RAS 
    // (the Orienation as in Axial, Sagittal, Coronal)
    // 
    // The combined transform:
    //
    // | R | = [Slice to RAS ] [ XY to Slice ]  | X |
    // | A |                                    | Y |
    // | S |                                    | Z |
    // | 1 |                                    | 1 |
    //
    // or
    //
    // RAS = XYToRAS * XY
    //
    vtkMatrix4x4::Multiply4x4(this->SliceToRAS, xyToSlice, xyToRAS);
    
    // check to see if the matrix actually changed
    if ( !Matrix4x4AreEqual (xyToRAS, this->XYToRAS) )
      {
      this->XYToSlice->DeepCopy( xyToSlice );
      this->XYToRAS->DeepCopy( xyToRAS );
      this->Modified();  // RSierra 3/9/07 This triggesr the update on
                         // the windows. In the IGT module when the
                         // slices are updated by the tracker it might
                         // be better to trigger the update AFTER all
                         // positions are modified to synchoronously
                         // update what the user sees ...
      }

    xyToSlice->Delete();
    xyToRAS->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::WriteXML(ostream& of, int nIndent)
{
  int i;

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " fieldOfView=\"" << 
        this->FieldOfView[0] << " " <<
        this->FieldOfView[1] << " " <<
        this->FieldOfView[2] << "\"";

  of << indent << " dimensions=\"" << 
        this->Dimensions[0] << " " <<
        this->Dimensions[1] << " " <<
        this->Dimensions[2] << "\"";

  of << indent << " layoutGridRows=\"" << 
        this->LayoutGridRows << "\"";

  of << indent << " layoutGridColumns=\"" << 
        this->LayoutGridColumns << "\"";

  std::stringstream ss;
  int j;
  for (i=0; i<4; i++) 
    {
    for (j=0; j<4; j++) 
      {
      ss << this->SliceToRAS->GetElement(i,j);
      if ( !( i==3 && j==3) )
        {
        ss << " ";
        }
      }
    }
  of << indent << " sliceToRAS=\"" << ss.str().c_str() << "\"";
  of << indent << " layoutName=\"" << this->GetLayoutName() << "\"";
  of << indent << " orientation=\"" << this->OrientationString << "\"";
  of << indent << " sliceVisibility=\"" << (this->SliceVisible ? "true" : "false") << "\"";
  of << indent << " widgetVisibility=\"" << (this->WidgetVisible ? "true" : "false") << "\"";


}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fieldOfView")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      int i;
      for (i=0; i<3; i++) 
        {
        ss >> val;
        this->FieldOfView[i] = val;
        }
      }
    else if (!strcmp(attName, "layoutGridRows")) 
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      
      this->LayoutGridRows = val;
      }
    else if (!strcmp(attName, "layoutGridColumns")) 
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      
      this->LayoutGridColumns = val;
      }
    else if (!strcmp(attName, "sliceVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->SliceVisible = 1;
        }
      else
        {
        this->SliceVisible = 0;
        }
      }
    else if (!strcmp(attName, "widgetVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->WidgetVisible = 1;
        }
      else
        {
        this->WidgetVisible = 0;
        }
      }
   else if (!strcmp(attName, "orientation")) 
      {
      this->SetOrientationString( attValue );
      }
    else if (!strcmp(attName, "layoutName")) 
      {
      this->SetLayoutName( attValue );
      }
   else if (!strcmp(attName, "dimensions")) 
      {
      std::stringstream ss;
      unsigned int val;
      ss << attValue;
      int i;
      for (i=0; i<3; i++) 
        {
        ss >> val;
        this->Dimensions[i] = val;
        }
      }
    else if (!strcmp(attName, "sliceToRAS")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      int i, j;
      for (i=0; i<4; i++) 
        {
        for (j=0; j<4; j++) 
          {
          ss >> val;
          this->SliceToRAS->SetElement(i,j,val);
          }
        }
      }
    }
  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSliceNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSliceNode *node = vtkMRMLSliceNode::SafeDownCast(anode);

  this->SetSliceVisible(node->GetSliceVisible());
  this->SliceToRAS->DeepCopy(node->GetSliceToRAS());
  this->SetOrientationString(node->GetOrientationString());

  this->LayoutGridColumns = node->LayoutGridColumns;
  this->LayoutGridRows = node->LayoutGridRows;
  
  int i;
  for(i=0; i<3; i++) 
    {
    this->FieldOfView[i] = node->FieldOfView[i];
    this->Dimensions[i] = node->Dimensions[i];
    }
  this->UpdateMatrices();

}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);
  os << "FieldOfView:\n ";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << " " << this->FieldOfView[idx];
  }
  os << "\n";

  os << "Dimensions:\n ";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << " " << this->Dimensions[idx];
  }
  os << "\n";

  os << indent << "Layout grid: " << this->LayoutGridRows << "x" << this->LayoutGridColumns << "\n";

  os << indent << "SliceVisible: " <<
    (this->SliceVisible ? "not null" : "(none)") << "\n";
  os << indent << "WidgetVisible: " <<
    (this->WidgetVisible ? "not null" : "(none)") << "\n";
  
  os << indent << "SliceToRAS: \n";
  this->SliceToRAS->PrintSelf(os, indent.GetNextIndent());

  os << indent << "XYToRAS: \n";
  this->XYToRAS->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::UpdateScene(vtkMRMLScene* scene)
{
  vtkMRMLSliceNode *node= NULL;
  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          scene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (node != this && !strcmp(node->GetLayoutName(), this->GetLayoutName()))
      {
      break;
      }
    node = NULL;
    }
  if (node != NULL)
    {
    scene->RemoveNodeNoNotify(node);
    }
}
// End

void vtkMRMLSliceNode::JumpSlice(double r, double a, double s)
{
  vtkMatrix4x4 *sliceToRAS = this->GetSliceToRAS();
  double sr = sliceToRAS->GetElement(0, 3);
  double sa = sliceToRAS->GetElement(1, 3);
  double ss = sliceToRAS->GetElement(2, 3);
  if ( r != sr || a != sa || s != ss ) {
    sliceToRAS->SetElement( 0, 3, r );
    sliceToRAS->SetElement( 1, 3, a );
    sliceToRAS->SetElement( 2, 3, s );
    this->UpdateMatrices();
  }
}

void vtkMRMLSliceNode::JumpAllSlices(double r, double a, double s)
{
  vtkMRMLSliceNode *node= NULL;
  vtkMRMLScene *scene = this->GetScene();
  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          scene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if ( node != NULL && node != this )
      {
      node->JumpSlice(r, a, s);
      }
    }
}

void vtkMRMLSliceNode::SetFieldOfView(double x, double y, double z)
{
  if ( x != this->FieldOfView[0] || y != this->FieldOfView[1]
       || z != this->FieldOfView[2] )
    {
    this->FieldOfView[0] = x;
    this->FieldOfView[1] = y;
    this->FieldOfView[2] = z;
    this->UpdateMatrices();
    }
}

void vtkMRMLSliceNode::SetDimensions(unsigned int x, unsigned int y,
                                     unsigned int z)
{
  if ( x != this->Dimensions[0] || y != this->Dimensions[1]
       || z != this->Dimensions[2] )
    {
    this->Dimensions[0] = x;
    this->Dimensions[1] = y;
    this->Dimensions[2] = z;
    this->UpdateMatrices();
    }
}

void vtkMRMLSliceNode::SetLayoutGrid(int rows, int columns)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level? 
  if (( rows != this->LayoutGridRows )
      || ( columns != this->LayoutGridColumns ))
    {
    // Calculate the scaling and "scaling magnitudes"
    double scaling[3];
    scaling[0] = this->LayoutGridColumns/(double) columns;
    scaling[1] = this->LayoutGridRows / (double) rows;
    scaling[2] = 1.0; // ???

    double scaleMagnitude[3];
    scaleMagnitude[0] = (scaling[0] < 1.0 ? 1.0/scaling[0] : scaling[0]);
    scaleMagnitude[1] = (scaling[1] < 1.0 ? 1.0/scaling[1] : scaling[1]);
    scaleMagnitude[2] = 1.0;
   
    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[0] = int( this->Dimensions[0] * scaling[0] );
    this->Dimensions[1] = int( this->Dimensions[1] * scaling[1] );
    this->Dimensions[2] = rows*columns;

    // adjust the field of view in x and y to maintain aspect ratio
    if (scaleMagnitude[0] < scaleMagnitude[1])
      {
      // keep x fov the same, adjust y
      this->FieldOfView[1] *= (scaling[1] / scaling[0]);
      }
    else
      {
      // keep y fov the same, adjust x
      this->FieldOfView[0] *= (scaling[0] / scaling[1]);
      }
    
    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2]
      *= (rows*columns
          / (double)(this->LayoutGridRows*this->LayoutGridColumns));

    // cache the layout
    this->LayoutGridRows = rows;
    this->LayoutGridColumns = columns;        
    
    this->UpdateMatrices();
    }
}

void vtkMRMLSliceNode::SetLayoutGridRows(int rows)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level? 
  if ( rows != this->LayoutGridRows )
    {
    // Calculate the scaling
    double scaling;
    scaling = this->LayoutGridRows / (double) rows;

    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[1] = int( this->Dimensions[1] * scaling );
    this->Dimensions[2] = rows*this->LayoutGridColumns;

    // adjust the field of view in x to maintain aspect ratio
    this->FieldOfView[0] /= scaling;
    
    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2] *= (rows / (double)this->LayoutGridRows);
    
    // cache the layout
    this->LayoutGridRows = rows;
    
    this->UpdateMatrices();
    }
}

void vtkMRMLSliceNode::SetLayoutGridColumns(int cols)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level? 
  if ( cols != this->LayoutGridColumns )
    {
    // Calculate the scaling
    double scaling;
    scaling = this->LayoutGridColumns / (double) cols;

    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[0] = int( this->Dimensions[0]
                               * (this->LayoutGridColumns / (double) cols));
    this->Dimensions[2] = this->LayoutGridRows*cols;

    // adjust the field of view in y to maintain aspect ratio
    this->FieldOfView[1] /= scaling;
    
    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2] *= (cols / (double)this->LayoutGridColumns);
    
    // cache the layout
    this->LayoutGridColumns = cols;
    
    this->UpdateMatrices();
    }
}
  
  
