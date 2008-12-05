/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkLocalAxesActor.cxx,v $
Language:  C++
Date:      $Date: 2008/04/28 02:59:24 $
Version:   $Revision: 1.10 $

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

#include "vtkLocalAxesActor.h"


#include "vtkActor.h"
#include "vtkCaptionActor2D.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPropCollection.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"



vtkCxxRevisionMacro(vtkLocalAxesActor, "$Revision: 1.10 $");
vtkStandardNewMacro(vtkLocalAxesActor);

vtkCxxSetObjectMacro( vtkLocalAxesActor, UserDefinedTip, vtkPolyData );
vtkCxxSetObjectMacro( vtkLocalAxesActor, UserDefinedShaft, vtkPolyData );


vtkLocalAxesActor::vtkLocalAxesActor()
{

  this->AxisLabels = 1;
  this->AxesPoints = vtkPoints::New();

  this->XAxisLabelText = NULL;
  this->YAxisLabelText = NULL;
  this->ZAxisLabelText = NULL;

  this->SetXAxisLabelText("X");
  this->SetYAxisLabelText("Y");
  this->SetZAxisLabelText("Z");

  this->XAxisShaft = vtkActor::New();
  this->XAxisShaft->GetProperty()->SetColor(1, 0, 0);
  this->YAxisShaft = vtkActor::New();
  this->YAxisShaft->GetProperty()->SetColor(0, 1, 0);
  this->ZAxisShaft = vtkActor::New();
  this->ZAxisShaft->GetProperty()->SetColor(0, 0, 1);

  this->XAxisTip = vtkActor::New();
  this->XAxisTip->GetProperty()->SetColor(1, 0, 0);
  this->YAxisTip = vtkActor::New();
  this->YAxisTip->GetProperty()->SetColor(0, 1, 0);
  this->ZAxisTip = vtkActor::New();
  this->ZAxisTip->GetProperty()->SetColor(0, 0, 1);

  this->XCylinderSource = vtkCylinderSource::New();
  this->YCylinderSource = vtkCylinderSource::New();
  this->ZCylinderSource = vtkCylinderSource::New();


  this->XLineSource = vtkLineSource::New();
  this->YLineSource = vtkLineSource::New();
  this->ZLineSource = vtkLineSource::New();

  this->XConeSource = vtkConeSource::New();
  this->YConeSource = vtkConeSource::New();
  this->ZConeSource = vtkConeSource::New();

  this->XSphereSource = vtkSphereSource::New();
  this->YSphereSource = vtkSphereSource::New();
  this->ZSphereSource = vtkSphereSource::New();

  vtkPolyDataMapper *XshaftMapper = vtkPolyDataMapper::New();
  vtkPolyDataMapper *YshaftMapper = vtkPolyDataMapper::New();
  vtkPolyDataMapper *ZshaftMapper = vtkPolyDataMapper::New();

  this->XAxisShaft->SetMapper( XshaftMapper );
  this->YAxisShaft->SetMapper( YshaftMapper );
  this->ZAxisShaft->SetMapper( ZshaftMapper ); 

  XshaftMapper->Delete();
  YshaftMapper->Delete();
  ZshaftMapper->Delete();

  vtkPolyDataMapper *XtipMapper = vtkPolyDataMapper::New();
  vtkPolyDataMapper *YtipMapper = vtkPolyDataMapper::New();
  vtkPolyDataMapper *ZtipMapper = vtkPolyDataMapper::New();

  this->XAxisTip->SetMapper( XtipMapper );
  this->YAxisTip->SetMapper( YtipMapper );
  this->ZAxisTip->SetMapper( ZtipMapper );

  XtipMapper->Delete();
  YtipMapper->Delete();
  ZtipMapper->Delete();


  this->ConeResolution = 16;
  this->SphereResolution = 16;
  this->CylinderResolution = 16;

  this->ShaftType = vtkLocalAxesActor::LINE_SHAFT;
  this->TipType   = vtkLocalAxesActor::CONE_TIP;

  this->UserDefinedTip = NULL;
  this->UserDefinedShaft = NULL;



  this->XAxisLabel = vtkCaptionActor2D::New();
  this->YAxisLabel = vtkCaptionActor2D::New();
  this->ZAxisLabel = vtkCaptionActor2D::New();

  this->XAxisLabel->ThreeDimensionalLeaderOff();
  this->XAxisLabel->LeaderOff();
  this->XAxisLabel->BorderOff();
  this->XAxisLabel->SetPosition(0, 0);
  this->XAxisLabel->SetPosition2(0.04, 0.04);

  this->YAxisLabel->ThreeDimensionalLeaderOff();
  this->YAxisLabel->LeaderOff();
  this->YAxisLabel->BorderOff();
  this->YAxisLabel->SetPosition(0, 0);
  this->YAxisLabel->SetPosition2(0.04, 0.04);

  this->ZAxisLabel->ThreeDimensionalLeaderOff();
  this->ZAxisLabel->LeaderOff();
  this->ZAxisLabel->BorderOff();
  this->ZAxisLabel->SetPosition(0, 0);
  this->ZAxisLabel->SetPosition2(0.04, 0.04);

  this->UpdateProps();
}



vtkLocalAxesActor::~vtkLocalAxesActor()

{

  this->XCylinderSource->Delete();
  this->YCylinderSource->Delete();
  this->ZCylinderSource->Delete();

  this->XLineSource->Delete();
  this->YLineSource->Delete();
  this->ZLineSource->Delete();

  this->XConeSource->Delete();
  this->YConeSource->Delete();
  this->ZConeSource->Delete();

  this->XSphereSource->Delete();
  this->YSphereSource->Delete();
  this->ZSphereSource->Delete();

  this->XAxisShaft->Delete();
  this->YAxisShaft->Delete();
  this->ZAxisShaft->Delete();

  this->XAxisTip->Delete();
  this->YAxisTip->Delete();
  this->ZAxisTip->Delete();

  this->SetUserDefinedTip( NULL );
  this->SetUserDefinedShaft( NULL );

  this->SetXAxisLabelText( NULL );
  this->SetYAxisLabelText( NULL );
  this->SetZAxisLabelText( NULL );

  this->XAxisLabel->Delete();
  this->YAxisLabel->Delete();
  this->ZAxisLabel->Delete();

}



// Shallow copy of an actor.

void vtkLocalAxesActor::ShallowCopy(vtkProp *prop)

{

  vtkLocalAxesActor *a = vtkLocalAxesActor::SafeDownCast(prop);

  if ( a != NULL )
  {
    this->SetAxisLabels( a->GetAxisLabels() );
    this->SetXAxisLabelText( a->GetXAxisLabelText() );
    this->SetYAxisLabelText( a->GetYAxisLabelText() );
    this->SetZAxisLabelText( a->GetZAxisLabelText() );
    this->SetConeResolution( a->GetConeResolution() );
    this->SetSphereResolution( a->GetSphereResolution() );
    this->SetCylinderResolution( a->GetCylinderResolution() );
    this->SetConeRadius( a->GetConeRadius() );
    this->SetSphereRadius( a->GetSphereRadius() );
    this->SetCylinderRadius( a->GetCylinderRadius() );
    this->SetTipType( a->GetTipType() );
    this->SetShaftType( a->GetShaftType() );
    this->SetUserDefinedTip( a->GetUserDefinedTip() );
    this->SetUserDefinedShaft( a->GetUserDefinedShaft() );
  }
  
  // Now do superclass
  this->vtkProp3D::ShallowCopy(prop);

}



void vtkLocalAxesActor::GetActors(vtkPropCollection *ac)

{

  ac->AddItem( this->XAxisShaft );
  ac->AddItem( this->YAxisShaft );
  ac->AddItem( this->ZAxisShaft );
  ac->AddItem( this->XAxisTip );
  ac->AddItem( this->YAxisTip );
  ac->AddItem( this->ZAxisTip );
}



int vtkLocalAxesActor::RenderOpaqueGeometry(vtkViewport *vp)
{
  int renderedSomething = 0;
  this->UpdateProps();

  renderedSomething += this->XAxisShaft->RenderOpaqueGeometry( vp );
  renderedSomething += this->YAxisShaft->RenderOpaqueGeometry( vp );
  renderedSomething += this->ZAxisShaft->RenderOpaqueGeometry( vp );

  renderedSomething += this->XAxisTip->RenderOpaqueGeometry( vp );
  renderedSomething += this->YAxisTip->RenderOpaqueGeometry( vp );
  renderedSomething += this->ZAxisTip->RenderOpaqueGeometry( vp );

  if ( this->AxisLabels )
    {
    renderedSomething += this->XAxisLabel->RenderOpaqueGeometry( vp );
    renderedSomething += this->YAxisLabel->RenderOpaqueGeometry( vp );
    renderedSomething += this->ZAxisLabel->RenderOpaqueGeometry( vp );
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;

}



int vtkLocalAxesActor::RenderTranslucentGeometry(vtkViewport *vp)
{

  int renderedSomething = 0;
  this->UpdateProps();

  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;
}



int vtkLocalAxesActor::RenderOverlay(vtkViewport *vp)
{
  int renderedSomething = 0;

  if ( !this->AxisLabels )
  {
    return renderedSomething;
  }

  this->UpdateProps();

  renderedSomething += this->XAxisLabel->RenderOverlay( vp );
  renderedSomething += this->YAxisLabel->RenderOverlay( vp );
  renderedSomething += this->ZAxisLabel->RenderOverlay( vp );

  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;
}



void vtkLocalAxesActor::ReleaseGraphicsResources(vtkWindow *win)

{

  this->XAxisShaft->ReleaseGraphicsResources( win );
  this->YAxisShaft->ReleaseGraphicsResources( win );
  this->ZAxisShaft->ReleaseGraphicsResources( win );

  this->XAxisTip->ReleaseGraphicsResources( win );
  this->YAxisTip->ReleaseGraphicsResources( win );
  this->ZAxisTip->ReleaseGraphicsResources( win );

  this->XAxisLabel->ReleaseGraphicsResources( win );
  this->YAxisLabel->ReleaseGraphicsResources( win );
  this->ZAxisLabel->ReleaseGraphicsResources( win );
}



void vtkLocalAxesActor::GetBounds(double bounds[6])
{
  double *bds = this->GetBounds();
  bounds[0] = bds[0];
  bounds[1] = bds[1];
  bounds[2] = bds[2];
  bounds[3] = bds[3];
  bounds[4] = bds[4];
  bounds[5] = bds[5];
}



// Get the bounds for this Actor as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkLocalAxesActor::GetBounds()
{
  double bounds[6];
  int i;

  this->XAxisShaft->GetBounds(this->Bounds);
 // this->X

  this->YAxisShaft->GetBounds(bounds);
  for (i=0; i<3; i++)
    {

    this->Bounds[2*i+1] =

      (bounds[2*i+1]>this->Bounds[2*i+1])?(bounds[2*i+1]):(this->Bounds[2*i+1]);

    }



  this->ZAxisShaft->GetBounds(bounds);

  for (i=0; i<3; i++)

    {

    this->Bounds[2*i+1] = 

      (bounds[2*i+1]>this->Bounds[2*i+1])?(bounds[2*i+1]):(this->Bounds[2*i+1]);    

    }



  this->XAxisTip->GetBounds(bounds);

  for (i=0; i<3; i++)

    {

    this->Bounds[2*i+1] = 

      (bounds[2*i+1]>this->Bounds[2*i+1])?(bounds[2*i+1]):(this->Bounds[2*i+1]);    

    }



  this->YAxisTip->GetBounds(bounds);

  for (i=0; i<3; i++)

    {

    this->Bounds[2*i+1] = 

      (bounds[2*i+1]>this->Bounds[2*i+1])?(bounds[2*i+1]):(this->Bounds[2*i+1]);    

    }



  this->ZAxisTip->GetBounds(bounds);

  for (i=0; i<3; i++)

    {

    this->Bounds[2*i+1] = 

      (bounds[2*i+1]>this->Bounds[2*i+1])?(bounds[2*i+1]):(this->Bounds[2*i+1]);    

    }



  double dbounds[6];

  (vtkPolyDataMapper::SafeDownCast(this->YAxisShaft->GetMapper()))->

    GetInput()->GetBounds( dbounds );

  

  for (i=0; i<3; i++)

    {

    this->Bounds[2*i+1] = 

      (dbounds[2*i+1]>this->Bounds[2*i+1])?(dbounds[2*i+1]):(this->Bounds[2*i+1]);    

    }

  return this->Bounds;

}



unsigned long int vtkLocalAxesActor::GetMTime()

{

  unsigned long mTime = this->Superclass::GetMTime();

  return mTime;

}



unsigned long int vtkLocalAxesActor::GetRedrawMTime()

{

  unsigned long mTime = this->GetMTime();

  return mTime;

}



void vtkLocalAxesActor::SetShaftType( int type )

{

  if ( this->ShaftType != type )

    {

    if (type < vtkLocalAxesActor::CYLINDER_SHAFT || \

        type > vtkLocalAxesActor::USER_DEFINED_SHAFT)

      {

      vtkErrorMacro( "Undefined axes shaft type." );

      return;

      }



    if ( type == vtkLocalAxesActor::USER_DEFINED_SHAFT && \

         this->UserDefinedShaft == NULL)

      {

      vtkErrorMacro( "Set the user defined shaft before changing the type." );

      return;

      }



    this->ShaftType = type;



    this->Modified();



    this->UpdateProps();

    }

}



void vtkLocalAxesActor::SetTipType( int type )

{

  if ( this->TipType != type )

    {

    if (type < vtkLocalAxesActor::CONE_TIP || \

        type > vtkLocalAxesActor::USER_DEFINED_TIP)

      {

      vtkErrorMacro( "Undefined axes tip type." );

      return;

      }



    if ( type == vtkLocalAxesActor::USER_DEFINED_TIP && \

         this->UserDefinedTip == NULL)

      {

      vtkErrorMacro( "Set the user defined tip before changing the type." );

      return;

      }



    this->TipType = type;



    this->Modified();



    this->UpdateProps();

    }

}



void vtkLocalAxesActor::UpdateProps()

{



//  this->XCylinderSource->SetResolution( this->CylinderResolution );

//  this->YCylinderSource->SetResolution( this->CylinderResolution );

//  this->ZCylinderSource->SetResolution( this->CylinderResolution );



  this->XConeSource->SetResolution( this->ConeResolution );

  this->YConeSource->SetResolution( this->ConeResolution );

  this->ZConeSource->SetResolution( this->ConeResolution );



//  this->SphereSource->SetThetaResolution( this->SphereResolution );

//  this->SphereSource->SetPhiResolution( this->SphereResolution );

//  this->SphereSource->SetRadius( this->SphereRadius );





      (vtkPolyDataMapper::SafeDownCast(this->XAxisShaft->GetMapper()))->

        SetInput( this->XLineSource->GetOutput() );

          (vtkPolyDataMapper::SafeDownCast(this->YAxisShaft->GetMapper()))->

                  SetInput( this->YLineSource->GetOutput() );

          (vtkPolyDataMapper::SafeDownCast(this->ZAxisShaft->GetMapper()))->

                  SetInput( this->ZLineSource->GetOutput() );



      (vtkPolyDataMapper::SafeDownCast(this->XAxisTip->GetMapper()))->

        SetInput( this->XConeSource->GetOutput() );

          (vtkPolyDataMapper::SafeDownCast(this->YAxisTip->GetMapper()))->

                  SetInput( this->YConeSource->GetOutput() );

          (vtkPolyDataMapper::SafeDownCast(this->ZAxisTip->GetMapper()))->

                  SetInput( this->ZConeSource->GetOutput() );

 



          

  (vtkPolyDataMapper::SafeDownCast(this->XAxisTip->GetMapper()))->

    GetInput()->Update();



  this->XAxisLabel->SetCaption( this->XAxisLabelText );

  this->YAxisLabel->SetCaption( this->YAxisLabelText );

  this->ZAxisLabel->SetCaption( this->ZAxisLabelText );

}



vtkProperty *vtkLocalAxesActor::GetXAxisTipProperty()

{

  return this->XAxisTip->GetProperty();

}



vtkProperty *vtkLocalAxesActor::GetYAxisTipProperty()

{

  return this->YAxisTip->GetProperty();

}



vtkProperty *vtkLocalAxesActor::GetZAxisTipProperty()

{

  return this->ZAxisTip->GetProperty();

}



vtkProperty *vtkLocalAxesActor::GetXAxisShaftProperty()

{

  return this->XAxisShaft->GetProperty();

}



vtkProperty *vtkLocalAxesActor::GetYAxisShaftProperty()

{

  return this->YAxisShaft->GetProperty();

}



vtkProperty *vtkLocalAxesActor::GetZAxisShaftProperty()

{

  return this->ZAxisShaft->GetProperty();

}



void vtkLocalAxesActor::PrintSelf(ostream& os, vtkIndent indent)

{

  this->Superclass::PrintSelf(os,indent);



  os << indent << "UserDefinedShaft: ";

  if (this->UserDefinedShaft)

    {

    os << this->UserDefinedShaft << endl;

    }

  else

    {

    os << "(none)" << endl;

    }

  

  os << indent << "UserDefinedTip: ";

  if (this->UserDefinedTip)

    {

    os << this->UserDefinedTip << endl;

    }

  else

    {

    os << "(none)" << endl;

    }

  

  os << indent << "XAxisLabelText: " << (this->XAxisLabelText ?

                                         this->XAxisLabelText : "(none)")

     << endl;

  os << indent << "YAxisLabelText: " << (this->YAxisLabelText ?

                                         this->YAxisLabelText : "(none)")

     << endl;

  os << indent << "ZAxisLabelText: " << (this->ZAxisLabelText ?

                                         this->ZAxisLabelText : "(none)")

     << endl;



  os << indent << "AxisLabels: " << (this->AxisLabels ? "On\n" : "Off\n");



  os << indent << "ShaftType: " << this->ShaftType << endl;

  os << indent << "TipType: " << this->TipType << endl;

  os << indent << "SphereRadius: " << this->SphereRadius << endl;

  os << indent << "SphereResolution: " << this->SphereResolution << endl;

  os << indent << "CylinderRadius: " << this->CylinderRadius << endl;

  os << indent << "CylinderResolution: " << this->CylinderResolution << endl;

  os << indent << "ConeRadius: " << this->ConeRadius << endl;

  os << indent << "ConeResolution: " << this->ConeResolution << endl;

}



void vtkLocalAxesActor::SetAxesPoints(vtkPoints *axespoints)

{

        this->AxesPoints->Initialize();

/*      this->AxesPoints->SetNumberOfPoints(3);

        for(int i = 0; i < 3; i++)

        {

                this->AxesPoints->SetPoint(i, axespoints->GetPoint(i));

        }*/

        this->AxesPoints->DeepCopy(axespoints);

        // compute parameters for the cylinder, cone and sphere.

        // computation will be on the shortest length of the 3 given

        // axes.

        double length;

        for(int i = 0; i < 3; i++)

        {



                if(i==0)        length = vtkMath::Distance2BetweenPoints(

                        this->GetOrigin(), this->AxesPoints->GetPoint(i));

                else

                {

                        if(length > vtkMath::Distance2BetweenPoints(

                                this->GetOrigin(), this->AxesPoints->GetPoint(i)))

                        {

                                length = vtkMath::Distance2BetweenPoints(

                                        this->GetOrigin(), this->AxesPoints->GetPoint(i));

                        }

                }

        }

        length = sqrt(length);



        this->CylinderRadius = 0.1*length;

        this->ConeRadius = 0.05*length;

        this->SphereRadius = 0.2*length;



        // this->ConeSource->SetDirection( 0, 1, 0 );

        // this->ConeSource->SetHeight( 1.0 );

        this->XLineSource->SetPoint1(this->GetOrigin());

        this->YLineSource->SetPoint1(this->GetOrigin());

        this->ZLineSource->SetPoint1(this->GetOrigin());



        this->XLineSource->SetPoint2(

                this->AxesPoints->GetPoint(0));

        this->YLineSource->SetPoint2(

                this->AxesPoints->GetPoint(1));

        this->ZLineSource->SetPoint2(

                this->AxesPoints->GetPoint(2));



        this->XConeSource->SetCenter(

                this->AxesPoints->GetPoint(0));

        this->YConeSource->SetCenter(

                this->AxesPoints->GetPoint(1));

        this->ZConeSource->SetCenter(

                this->AxesPoints->GetPoint(2));



        this->XConeSource->SetHeight(0.3*length);

        this->YConeSource->SetHeight(0.3*length);

        this->ZConeSource->SetHeight(0.3*length);



        double pt1[3], pt2[3];

        double distance1, distance2, distance3;

        this->GetOrigin(pt1);

        this->AxesPoints->GetPoint(0,pt2);

        distance1 = sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));

        this->XConeSource->SetDirection((pt2[0]-pt1[0])/

                distance1, (pt2[1]-pt1[1])/distance1, (pt2[2]-pt1[2])/distance1);



        this->XAxisLabel->SetAttachmentPoint( pt2[0]+0.4*length*(pt2[0]-pt1[0])/

                distance1, pt2[1]+0.4*length*(pt2[1]-pt1[1])/

                distance1,pt2[2]+0.4*length*(pt2[2]-pt1[2])/distance1);



        this->AxesPoints->GetPoint(1,pt2);

        distance2 = sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));

        this->YConeSource->SetDirection((pt2[0]-pt1[0])/

                distance2, (pt2[1]-pt1[1])/distance2, (pt2[2]-pt1[2])/distance2);



        this->YAxisLabel->SetAttachmentPoint( pt2[0]+0.4*length*(pt2[0]-pt1[0])/

                distance2, pt2[1]+0.4*length*(pt2[1]-pt1[1])/

                distance2,pt2[2]+0.4*length*(pt2[2]-pt1[2])/distance2);



        this->AxesPoints->GetPoint(2,pt2);

        distance3 = sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));

        this->ZConeSource->SetDirection((pt2[0]-pt1[0])/

                distance3, (pt2[1]-pt1[1])/distance3, (pt2[2]-pt1[2])/distance3);

        this->ZAxisLabel->SetAttachmentPoint( pt2[0]+0.4*length*(pt2[0]-pt1[0])/

                distance3, pt2[1]+0.4*length*(pt2[1]-pt1[1])/

                distance3,pt2[2]+0.4*length*(pt2[2]-pt1[2])/distance3);

        this->XAxisLabel->GetCaptionTextProperty()->SetFontSize(8);
        this->XAxisLabel->GetCaptionTextProperty()->SetColor(1,0,0);

        this->YAxisLabel->GetCaptionTextProperty()->SetFontSize(static_cast<int>(0.05*length));
        this->YAxisLabel->GetCaptionTextProperty()->SetColor(0,1,0);

        this->ZAxisLabel->GetCaptionTextProperty()->SetFontSize(static_cast<int>(0.075*length));
        this->ZAxisLabel->GetCaptionTextProperty()->SetColor(0,0,1);
}

