/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkLocalAxesActor.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.5 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkLocalAxesActor - a 3D non-orthogonal axes representation
// .SECTION Description
// vtkLocalAxesActor is used to create a local axes co-ordinate system
// which is non-orthogonal. input required are the origin and the 3 points
// describing the 3 end of points of x, y and z axes.

// .SECTION See Also
// vtkAnnotatedCubeActor vtkOrientationMarkerWidget vtkCaptionActor2D
// vtkTextProperty

#ifndef __vtkLocalAxesActor_h
#define __vtkLocalAxesActor_h

#include "vtkCommon.h"

#include "vtkProp3D.h"

class vtkActor;
class vtkCaptionActor2D;
class vtkConeSource;
class vtkCylinderSource;
class vtkLineSource;
class vtkPoints;
class vtkPolyData;
class vtkPropCollection;
class vtkProperty;
class vtkRenderer;
class vtkSphereSource;

class VTK_MIMXCOMMON_EXPORT vtkLocalAxesActor : public vtkProp3D
{
public:
  static vtkLocalAxesActor *New();
  vtkTypeRevisionMacro(vtkLocalAxesActor,vtkProp3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors(vtkPropCollection *);

  // Description:
  // Support the standard render methods.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);
  virtual int RenderOverlay(vtkViewport *viewport);

  // Description:
  // Shallow copy of an axes actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Get the bounds for this Actor as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax). (The
  // method GetBounds(double bounds[6]) is available from the superclass.)
  void GetBounds(double bounds[6]);
  double *GetBounds();

  // Description:
  // Get the actors mtime plus consider its properties and texture if set.
  unsigned long int GetMTime();
  
  // Description:
  // Return the mtime of anything that would cause the rendered image to 
  // appear differently. Usually this involves checking the mtime of the 
  // prop plus anything else it depends on such as properties, textures
  // etc.
  virtual unsigned long GetRedrawMTime();

  // Description:
  // Set/get the resolution of the pieces of the axes actor.
  vtkSetClampMacro(ConeResolution, int, 3, 128);
  vtkGetMacro(ConeResolution, int);
  vtkSetClampMacro(SphereResolution, int, 3, 128);
  vtkGetMacro(SphereResolution, int);
  vtkSetClampMacro(CylinderResolution, int, 3, 128);
  vtkGetMacro(CylinderResolution, int);
  
  // Description:
  // Set/get the radius of the pieces of the axes actor.
  vtkSetClampMacro(ConeRadius, double, 0, VTK_LARGE_FLOAT);
  vtkGetMacro(ConeRadius, double);
  vtkSetClampMacro(SphereRadius, double, 0, VTK_LARGE_FLOAT);
  vtkGetMacro(SphereRadius, double);
  vtkSetClampMacro(CylinderRadius, double, 0, VTK_LARGE_FLOAT);
  vtkGetMacro(CylinderRadius, double);

  // Description:
  // Set the type of the shaft to a cylinder, line, or user defined geometry.
  void SetShaftType( int type );
  void SetShaftTypeToCylinder()
    { this->SetShaftType( vtkLocalAxesActor::CYLINDER_SHAFT ); }
  void SetShaftTypeToLine()
    { this->SetShaftType( vtkLocalAxesActor::LINE_SHAFT ); }
  void SetShaftTypeToUserDefined()
    { this->SetShaftType( vtkLocalAxesActor::USER_DEFINED_SHAFT ); }
  vtkGetMacro(ShaftType, int);

  // Description:
  // Set the type of the tip to a cone, sphere, or user defined geometry.
  void SetTipType( int type );
  void SetTipTypeToCone()
    { this->SetTipType( vtkLocalAxesActor::CONE_TIP ); }
  void SetTipTypeToSphere()
    { this->SetTipType( vtkLocalAxesActor::SPHERE_TIP ); }
  void SetTipTypeToUserDefined()
    { this->SetTipType( vtkLocalAxesActor::USER_DEFINED_TIP ); }
  vtkGetMacro(TipType, int);

  // Description:
  // Set the user defined tip polydata.
  void SetUserDefinedTip( vtkPolyData * );
  vtkGetObjectMacro( UserDefinedTip, vtkPolyData );

  // Description:
  // Set the user defined shaft polydata.
  void SetUserDefinedShaft( vtkPolyData * );
  vtkGetObjectMacro( UserDefinedShaft, vtkPolyData );

  // Description:
  // Get the tip properties.
  vtkProperty *GetXAxisTipProperty();
  vtkProperty *GetYAxisTipProperty();
  vtkProperty *GetZAxisTipProperty();

  // Description:
  // Get the shaft properties.
  vtkProperty *GetXAxisShaftProperty();
  vtkProperty *GetYAxisShaftProperty();
  vtkProperty *GetZAxisShaftProperty();

  // Description:
  // Retrieve handles to the X, Y and Z axis (so that you can set their text
  // properties for example)
  vtkCaptionActor2D *GetXAxisCaptionActor2D()
    {return this->XAxisLabel;}
  vtkCaptionActor2D *GetYAxisCaptionActor2D()
    {return this->YAxisLabel;}
  vtkCaptionActor2D *GetZAxisCaptionActor2D()
    {return this->ZAxisLabel;}

  // Description:
  // Set/get the label text.
  vtkSetStringMacro( XAxisLabelText );
  vtkGetStringMacro( XAxisLabelText );
  vtkSetStringMacro( YAxisLabelText );
  vtkGetStringMacro( YAxisLabelText );
  vtkSetStringMacro( ZAxisLabelText );
  vtkGetStringMacro( ZAxisLabelText );

  // Description:
  // Enable/disable drawing the axis labels.
  vtkSetMacro(AxisLabels, int);
  vtkGetMacro(AxisLabels, int);
  vtkBooleanMacro(AxisLabels, int);

  // Description
  // vtkPoints to store the 3 end points of the axes
  void SetAxesPoints(vtkPoints* );

//BTX
  enum
  {
    CYLINDER_SHAFT,
    LINE_SHAFT,
    USER_DEFINED_SHAFT
  };

  enum
  {
    CONE_TIP,
    SPHERE_TIP,
    USER_DEFINED_TIP
  };  
//ETX

protected:
  vtkLocalAxesActor();
  ~vtkLocalAxesActor();

  vtkCylinderSource *XCylinderSource;
  vtkCylinderSource *YCylinderSource;
  vtkCylinderSource *ZCylinderSource;

  vtkLineSource     *XLineSource;
  vtkLineSource     *YLineSource;
  vtkLineSource     *ZLineSource;

  vtkConeSource     *XConeSource;
  vtkConeSource     *YConeSource;
  vtkConeSource     *ZConeSource;

  vtkSphereSource   *XSphereSource;
  vtkSphereSource   *YSphereSource;
  vtkSphereSource   *ZSphereSource;

  vtkActor          *XAxisShaft;
  vtkActor          *YAxisShaft;
  vtkActor          *ZAxisShaft;

  vtkActor          *XAxisTip;
  vtkActor          *YAxisTip;
  vtkActor          *ZAxisTip;

  void               UpdateProps();

  int                ShaftType;
  int                TipType;

  vtkPolyData       *UserDefinedTip;
  vtkPolyData       *UserDefinedShaft;

  char              *XAxisLabelText;
  char              *YAxisLabelText;
  char              *ZAxisLabelText;

  vtkCaptionActor2D *XAxisLabel;
  vtkCaptionActor2D *YAxisLabel;
  vtkCaptionActor2D *ZAxisLabel;

  int                AxisLabels;


  int                ConeResolution;
  int                SphereResolution;
  int                CylinderResolution;

  double             ConeRadius;
  double             SphereRadius;
  double             CylinderRadius;
  vtkPoints          *AxesPoints;

private:
  vtkLocalAxesActor(const vtkLocalAxesActor&);  // Not implemented.
  void operator=(const vtkLocalAxesActor&);  // Not implemented.
};

#endif

