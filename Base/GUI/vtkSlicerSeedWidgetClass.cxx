#include "vtkSlicerSeedWidgetClass.h"

#include "vtkProperty.h"

#include "vtkSlicerGlyphSource2D.h"

#include "vtkSeedWidget.h"
#include "vtkPolygonalHandleRepresentation3D.h"
#include "vtkOrientedPolygonalHandleRepresentation3D.h"
#include "vtkSeedRepresentation.h"
#include "vtkPolygonalSurfacePointPlacer.h"

#include "vtkHandleWidget.h"

// for the glyphs
#include "vtkCellArray.h"
#include "vtkPoints.h"

#include "vtkSource.h"

// for updating the camera/views
#include "vtkCamera.h"
#include "vtkPropCollection.h"
#include "vtkFollower.h"

// for scaling
#include "vtkHomogeneousTransform.h"
#include "vtkMatrix4x4.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro (vtkSlicerSeedWidgetClass);
vtkCxxRevisionMacro ( vtkSlicerSeedWidgetClass, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerSeedWidgetClass::vtkSlicerSeedWidgetClass()
{
  this->ModelPointPlacer = vtkPolygonalSurfacePointPlacer::New();

  // make a starburst glyph
  this->Glyph = vtkSlicerGlyphSource2D::New();
  this->Glyph->SetGlyphTypeToStarBurst();
  this->Glyph->Update();
  this->Glyph->SetScale(1.0);

  // Create the 3d diamond glyphs
  vtkPoints * diamondGlyphPoints = vtkPoints::New();
  diamondGlyphPoints->SetNumberOfPoints(6);
  diamondGlyphPoints->InsertPoint(0, 0.5, 0, 0);
  diamondGlyphPoints->InsertPoint(1, 0, 0.5, 0);
  diamondGlyphPoints->InsertPoint(2, 0, 0, 0.5);
  diamondGlyphPoints->InsertPoint(3, -0.5, 0, 0);
  diamondGlyphPoints->InsertPoint(4, 0, -0.5, 0);
  diamondGlyphPoints->InsertPoint(5, 0, 0, -0.5);

  vtkCellArray * diamondGlyphPolys = vtkCellArray::New();
  diamondGlyphPolys->InsertNextCell( 4 );
  diamondGlyphPolys->InsertCellPoint(0);
  diamondGlyphPolys->InsertCellPoint(1);
  diamondGlyphPolys->InsertCellPoint(3);
  diamondGlyphPolys->InsertCellPoint(4);
  
  diamondGlyphPolys->InsertNextCell(4);
  diamondGlyphPolys->InsertCellPoint(1);
  diamondGlyphPolys->InsertCellPoint(2);
  diamondGlyphPolys->InsertCellPoint(4);
  diamondGlyphPolys->InsertCellPoint(5);

  diamondGlyphPolys->InsertNextCell(4);
  diamondGlyphPolys->InsertCellPoint(2);
  diamondGlyphPolys->InsertCellPoint(0);
  diamondGlyphPolys->InsertCellPoint(5);
  diamondGlyphPolys->InsertCellPoint(3);

  vtkCellArray * diamondGlyphLines = vtkCellArray::New(); 
          
  diamondGlyphLines->InsertNextCell(2);
  diamondGlyphLines->InsertCellPoint(0);
  diamondGlyphLines->InsertCellPoint(3);

  diamondGlyphLines->InsertNextCell(2);
  diamondGlyphLines->InsertCellPoint(1);
  diamondGlyphLines->InsertCellPoint(4);

  diamondGlyphLines->InsertNextCell(2);                                         
  diamondGlyphLines->InsertCellPoint(2);
  diamondGlyphLines->InsertCellPoint(5);

  this->DiamondGlyphPolyData = vtkPolyData::New();
  this->DiamondGlyphPolyData->SetPoints(diamondGlyphPoints);
  diamondGlyphPoints->Delete();
  this->DiamondGlyphPolyData->SetPolys(diamondGlyphPolys);
  this->DiamondGlyphPolyData->SetLines(diamondGlyphLines);
  diamondGlyphPolys->Delete();
  diamondGlyphLines->Delete();

  // make a sphere glyph
  this->SphereSource = vtkSphereSource::New();
  this->SphereSource->SetRadius(0.5);
  this->SphereSource->SetPhiResolution(10);
  this->SphereSource->SetThetaResolution(10);
  this->SphereSource->Update();

  this->ListColor[0]=0.4; this->ListColor[1]=1.0; this->ListColor[2]=1.0;
  this->ListSelectedColor[0]=1.0; this->ListSelectedColor[1]=0.5; this->ListSelectedColor[2]=0.5;
  
  // Seed Widget set up
  // the handle repersentation for 3d glyphs
  this->HandleRepresentation = vtkPolygonalHandleRepresentation3D::New();
  // init the text scale, otherwise it's scaled according to the handle scale.
  double textscale[3] = {1.0, 1.0, 1.0};
  this->HandleRepresentation->SetLabelTextScale(textscale);
  this->HandleRepresentation->SetHandle(this->SphereSource->GetOutput());
  this->HandleRepresentation->GetProperty()->SetColor(this->GetListColor());

  // the handle representation for 2d glyphs
  this->OrientedHandleRepresentation = vtkOrientedPolygonalHandleRepresentation3D::New();
  this->OrientedHandleRepresentation->SetLabelTextScale(textscale);
  this->OrientedHandleRepresentation->SetHandle(this->Glyph->GetOutput());
  this->OrientedHandleRepresentation->GetProperty()->SetColor(this->GetListColor());

  this->Representation = vtkSeedRepresentation::New();
  // default to the starburst
  this->Representation->SetHandleRepresentation(this->OrientedHandleRepresentation);
  this->GlyphType = vtkSlicerSeedWidgetClass::Starburst;

 

  this->Widget = vtkSeedWidget::New();
  // turn off cursor changes for now, not allowing native adding of points
  //this->Widget->ManagesCursorOff();
  this->Widget->EnabledOff();
  this->Widget->CreateDefaultRepresentation();
  this->Widget->SetRepresentation(this->Representation);

//  this->DebugOn();
}

//---------------------------------------------------------------------------
vtkSlicerSeedWidgetClass::~vtkSlicerSeedWidgetClass()
{
  if (this->Widget)
    {
    // enabled off should remove observers
    this->Widget->EnabledOff();
    this->Widget->SetInteractor(NULL);
    this->Widget->SetRepresentation(NULL);
    this->Widget->Delete();
    this->Widget = NULL;
    }
 
  if (this->Representation)
    {
    this->Representation->SetHandleRepresentation(NULL);
    this->Representation->Delete();
    this->Representation = NULL;
    }

  if (this->HandleRepresentation)
    {
    this->HandleRepresentation->SetHandle(NULL);
    this->HandleRepresentation->Delete();
    this->HandleRepresentation = NULL;
    }
  if (this->OrientedHandleRepresentation)
    {
    this->OrientedHandleRepresentation->SetHandle(NULL);
    this->OrientedHandleRepresentation->Delete();
    this->OrientedHandleRepresentation = NULL;
    }
  
 if (this->Glyph)
    {
    this->Glyph->Delete();
    this->Glyph = NULL;
    }

  if (this->DiamondGlyphPolyData)
    {
    this->DiamondGlyphPolyData->Delete();
    this->DiamondGlyphPolyData = NULL;
    }

  if (this->SphereSource)
    {
    this->SphereSource->Delete();
    this->SphereSource = NULL;
    } 
   
 
 
 
  if (this->ModelPointPlacer)
    {
    this->ModelPointPlacer->RemoveAllProps();
    this->ModelPointPlacer->Delete();
    this->ModelPointPlacer = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  this->vtkObject::PrintSelf ( os, indent );
  if (this->ModelPointPlacer)
    {
    os << indent << "There is a ModelPointPlacer:\n";
    //this->ModelPointPlacer->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Glyph)
    {
    os << indent << "There is a Glyph defined\n";
//    this->Glyph->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Widget)
    {
    os << indent << "Widget:\n";
    this->Widget->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->HandleRepresentation)
    {
    os << indent << "HandleRepresentation:\n";
    this->HandleRepresentation->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->OrientedHandleRepresentation)
    {
    os << indent << "OrientedHandleRepresentation:\n";
    this->OrientedHandleRepresentation->PrintSelf(os,indent.GetNextIndent());
    }
  if (this->Representation)
    {
    os << indent << "Representation:\n";
    this->Representation->PrintSelf(os,indent.GetNextIndent());
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::AddSeed(double *position)
{
  if (this->GetWidget() == NULL)
    {
    return;
    }
  vtkHandleWidget *handle = this->GetWidget()->CreateNewHandle();
  // set the position if it was passed in
  if (handle &&
      position != NULL &&
      handle->GetRepresentation())
    {
    vtkHandleRepresentation::SafeDownCast(handle->GetRepresentation())->SetWorldPosition(position);
    }
  else
    {
    vtkWarningMacro("AddSeed: unable to set world position, hand rep is " << (handle->GetRepresentation() == NULL ? "null" : "not null"));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedPosition(int n, double *position)
{
  if (position == NULL ||
      this->GetWidget() == NULL)
    {
    return;
    }
  vtkHandleWidget *handle = this->Widget->GetSeed(n);
  if (!handle ||
      handle->GetRepresentation() == NULL)
    {
    return;
    }
  vtkHandleRepresentation::SafeDownCast(handle->GetRepresentation())->SetWorldPosition(position);
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::RemoveSeed(int index)
{
  this->GetWidget()->DeleteSeed(index);
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::RemoveAllSeeds()
{
  int numSeeds = this->Representation->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    this->GetWidget()->DeleteSeed(n);
    }
}

//---------------------------------------------------------------------------
vtkProperty * vtkSlicerSeedWidgetClass::GetProperty()
{
/*
  if (this->HandleRepresentation)
    {
    return this->HandleRepresentation->GetProperty();
    }
  else
    {
    return NULL;
    }
*/
  // since can have different handle reps, get current one from the seed
  // widget
  if (this->Widget &&
      this->Widget->GetRepresentation())
    {
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (sr &&
        sr->GetHandleRepresentation())
      {
      vtkAbstractPolygonalHandleRepresentation3D *hr = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation());
      if (hr)
        {
        return hr->GetProperty();
        }
      else
        {
        vtkErrorMacro("GetProperty: can't get abstract handle representation");
        }
      }
    else
      {
      vtkErrorMacro("GetProperty: can't get the seed representation");
      }
    }
  return NULL;
}

/*
//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetColor(double *col)
{
  if (col)
    {
    this->SetListColour(col);
//    this->SetColor(col[0], col[1], col[2]);
    }
}
//---------------------------------------------------------------------------
//void vtkSlicerSeedWidgetClass::SetColor(double r, double g, double b)
//{
  // this->SetListColour(r, g, b);
//}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetSelectedColor(double *col)
{
  if (col)
    {
    //this->SetSelectedColor(col[0], col[1], col[2]);
    this->SetListSelectedColour(col);
    }
}
//---------------------------------------------------------------------------
//void vtkSlicerSeedWidgetClass::SetSelectedColor(double r, double g, double b)
//{
  //this->SetListSelectedColour(r, g, b);
//}
*/
//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedTextScale(int n, double scale)
{
  if (this->Widget == NULL ||
      this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    return;
    }
    
  // vtkPolygonalHandleRepresentation3D *rep = NULL;
  //rep = vtkPolygonalHandleRepresentation3D::SafeDownCast(this->Widget->GetSeed(n)->GetRepresentation());
  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (rep)
    {
    double s[3];
    s[0] = s[1] = s[2] = scale;
    rep->SetLabelTextScale(s);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetTextScale(double scale)
{
  double s[3];
  s[0] = s[1] = s[2] = scale;
  if (this->HandleRepresentation)
    {
    //this->HandleRepresentation->SetLabelTextScale(s);
    }
  // iterate through all seeds, they're not getting set
  int numSeeds = this->Representation->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    this->SetNthSeedTextScale(n, scale);
    }
}

//---------------------------------------------------------------------------
double *vtkSlicerSeedWidgetClass::GetTextScale()
{
  //if (this->HandleRepresentation)
  if (this->Widget &&
      this->Widget->GetRepresentation())
    {
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (sr)
      {
      vtkAbstractPolygonalHandleRepresentation3D *rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation());
      if (rep)
        {
        return rep->GetLabelTextScale();
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetGlyphScale(double scale)
{
  // iterate through the handles
  if (this->Widget == NULL || this->Widget->GetRepresentation() == NULL)
    {
    return;
    }
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    vtkWarningMacro("SetGlyphScale: no representation for this widget.");
    return;
    }
  int numSeeds = sr->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    if (sr->GetHandleRepresentation(n) != NULL)
      {
      vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
      rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
      if (rep)
        {
        rep->SetUniformScale(scale);
        // call modified so text location updates
        rep->Modified();
        }
      else
        {
        vtkWarningMacro("SetGlyphScale: unable to get abstract polygonal handle representation 3d of handle representation for seed " << n);
        }
      }
    else
      {
      vtkWarningMacro("SetGlyphScale: unable to get handle representation " << n);
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedVisibility(int n, int flag)
{
  this->SetNthLabelTextVisibility(n, flag);

  if (this->Widget == NULL ||
      this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    return;
    }

  vtkWidgetRepresentation *rep = NULL;
  rep = this->Widget->GetSeed(n)->GetRepresentation();
  if (rep)
    {
    if (flag)
      {
      vtkDebugMacro("SetNthSeedVisibility: seed " << n << ", flag = " << flag << ", turning visib on");
      rep->VisibilityOn();
      }
    else
      {
      vtkDebugMacro("SetNthSeedVisibility: seed " << n << ", flag = " << flag << ", turning visib off");
      rep->VisibilityOff();
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthLabelTextVisibility(int n, int flag)
{
  if (this->Widget == NULL ||
      this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    return;
    }

//  vtkPolygonalHandleRepresentation3D *rep = NULL;
//  rep =
//  vtkPolygonalHandleRepresentation3D::SafeDownCast(this->Widget->GetSeed(n)->GetRepresentation());
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    return;
    }
  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (rep)
    {
    if (flag)
      {
      vtkDebugMacro("SetNthLabelTextVisibility: turning seed " << n << "'s label text visibility on");
      rep->LabelVisibilityOn();
      }
    else
      {
      vtkDebugMacro("SetNthLabelTextVisibility: turning seed " << n << "'s label text visibility off");
      rep->LabelVisibilityOff();
      }
    }
  else
    {
    vtkErrorMacro("SetNthLabelTextVisibility: can't get handle representation for seed " << n);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetLabelTextVisibility(int flag)
{
  if (this->Representation == NULL)
    {
    return;
    }
  
  int numSeeds = this->Representation->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    this->SetNthLabelTextVisibility(n, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthLabelText(int n, const char *txt)
{
  if (this->Widget == NULL || txt == NULL)
    {
    vtkErrorMacro("SetNthLabelText: either the widget is null, or txt = " << (txt == NULL ? "null" : txt)); 
    return;
    }
  if (this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    vtkErrorMacro("SetNthLabelText: unable to get seed " << n << " or it's representation");
    return;
    }
  //vtkPolygonalHandleRepresentation3D *rep = NULL;
  //rep =
  //vtkPolygonalHandleRepresentation3D::SafeDownCast(this->Widget->GetSeed(n)->GetRepresentation());
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetNthLabelText: unable to get the seed representation on the widget...");
    return;
    }
  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (rep)
    {
    rep->SetLabelText(txt);
    }
  else
    {
    vtkErrorMacro("SetNthLabelText: unable to get the polygonal handle representation on the " << n << "th seed, can't set text to " << txt);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedSelected(int n, int selectedFlag)
{
   if (this->Widget == NULL)
    {
    return;
    }
  if (this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    return;
    }
  //vtkPolygonalHandleRepresentation3D *rep = NULL;
  //rep =
  //vtkPolygonalHandleRepresentation3D::SafeDownCast(this->Widget->GetSeed(n)->GetRepresentation());
  vtkHandleWidget *handle = this->Widget->GetSeed(n);
  
  //vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  //if (!sr)
    {
    //  return;
    }
  
  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(handle->GetRepresentation()); //sr->GetHandleRepresentation(n));
  
  if (rep)
    {
    vtkDebugMacro("SetNthSeedSelected n = " << n << ", selected flag = " << selectedFlag << ", iterating through props to find actors");
    vtkProperty *prop = rep->GetProperty();
    vtkProperty *textProp = NULL;
    if (rep->GetLabelTextActor())
      {
      textProp = rep->GetLabelTextActor()->GetProperty();
      }
    if (prop)
      {
      double *col = NULL;
      if (selectedFlag)
        {
        // use the selected colour
        vtkDebugMacro("SetNthSeedSelected n = " << n << " getting selected colour ");
        col = this->GetListSelectedColor();
        }
      else
        {
        // use the regular colour
        vtkDebugMacro("SetNthSeedSelected n = " << n << " getting regular colour ");
        col = this->GetListColor(); 
        }
      if (col != NULL)
        {
        vtkDebugMacro("SetNthSeedSelected n = " << n << " using colour " << col[0] << ", " << col[1] << ", " << col[2]);
        prop->SetColor(col);
        if (textProp)
          {
          textProp->SetColor(col);
          }
        }
      }
    else
      {
      vtkErrorMacro("Unable to get propery for handle representation on seed " << n);
      }
    /*
    vtkPropCollection *pc = vtkPropCollection::New();
    rep->GetActors(pc);
    vtkCollectionSimpleIterator pit;
    vtkProp *aProp;
    for (pc->InitTraversal(pit);
         (aProp = pc->GetNextProp(pit)); )
      {
      vtkActor *a = vtkActor::SafeDownCast(aProp);
      if (a && a->GetProperty())
        {
        double *col = NULL;
        if (selectedFlag)
          {
          // use the selected colour
          vtkDebugMacro("SetNthSeedSelected n = " << n << " getting selected colour ");
          col = this->GetListSelectedColor(); // this->HandleRepresentation->GetSelectedProperty()->GetColor();
          }
        else
          {
          // use the regular colour
          vtkDebugMacro("SetNthSeedSelected n = " << n << " getting regular colour ");
          col = this->GetListColor(); // this->HandleRepresentation->GetProperty()->GetColor();
          }
        if (col != NULL)
          {
          vtkDebugMacro("SetNthSeedSelected n = " << n << " using colour " << col[0] << ", " << col[1] << ", " << col[2]);
          a->GetProperty()->SetColor(col);
          }
        }
      }
    */
    }
  else
    {
    vtkErrorMacro("SetNthSeedSelected: unable to get the polygonal handle representation on the " << n << "th seed, can't set selected to " << selectedFlag);
    }
}

//---------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::SetGlyphToStarburst()
{
  if (this->GlyphType == vtkSlicerSeedWidgetClass::Starburst)
    {
    return 0;
    }
  if (this->Glyph &&
      this->HandleRepresentation &&
      this->Widget &&
      this->Widget->GetRepresentation())
    {
    this->Glyph->SetGlyphTypeToStarBurst();
    this->Glyph->Update();
    this->HandleRepresentation->SetHandle(this->Glyph->GetOutput());
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (!sr)
      {
      vtkErrorMacro("SetGlyphToStarburst: unable to get seed representation");
      return -1;
      }
    int enabledState = this->Widget->GetEnabled();
    this->Widget->EnabledOff();
    // this will make any new seeds be starbursts
    sr->SetHandleRepresentation(this->HandleRepresentation);
    // now set the rest of them
    int numSeeds = sr->GetNumberOfSeeds();
    for (int n = 0; n < numSeeds; n++)
      {
      this->Widget->GetSeed(n)->SetRepresentation(this->HandleRepresentation);
      }
    this->Widget->SetEnabled(enabledState);
    this->Widget->CompleteInteraction();
    }
  this->GlyphType = vtkSlicerSeedWidgetClass::Starburst;
  return 1;
}


//---------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::SetGlyphToSphere()
{
  if (this->GlyphType == vtkSlicerSeedWidgetClass::Sphere)
    {
    return 0;
    }
  
  if (this->SphereSource &&
      this->Widget)
    {
    this->HandleRepresentation->SetHandle(this->SphereSource->GetOutput());
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (!sr)
      {
      vtkErrorMacro("SetGlyphToSphere: unable to get seed representation");
      return -1;
      }
    int enabledState = this->Widget->GetEnabled();
    this->Widget->EnabledOff();
    // this will make any new seeds be represented as spheres
    sr->SetHandleRepresentation(this->HandleRepresentation);
    // now change all the old ones
    int numSeeds = sr->GetNumberOfSeeds();
    double textscale[3] = {1.0, 1.0, 1.0};
    for (int n = 0; n < numSeeds; n++)
      {
      vtkPolygonalHandleRepresentation3D *hr = vtkPolygonalHandleRepresentation3D::New();
      hr->SetHandle(this->SphereSource->GetOutput());
      hr->SetLabelTextScale(textscale);
      hr->GetProperty()->SetColor(this->GetListColor());
      this->Widget->GetSeed(n)->SetRepresentation(hr);
      hr->Delete();
      }
    this->Widget->SetEnabled(enabledState);
    this->Widget->CompleteInteraction();

    this->GlyphType = vtkSlicerSeedWidgetClass::Sphere;
    return 1;
    /*
    vtkSeedRepresentation *seedRep = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (seedRep)
      {
      vtkPolygonalHandleRepresentation3D *polyRep = vtkPolygonalHandleRepresentation3D::SafeDownCast(seedRep->GetHandleRepresentation());
      if (polyRep)
        {
        vtkPolyData *pd = this->SphereSource->GetOutput();
        polyRep->SetHandle(pd);
        }
      }
    */
    }
  else
    {
    vtkErrorMacro("SetGlyphToSphere: Unable to set the sphere source as the handle for the seed widget");
    return -1;
    }
}

//---------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::SetGlyphToDiamond3D()
{
  if (this->GlyphType == vtkSlicerSeedWidgetClass::Diamond3D)
    {
    return 0;
    }
  if (this->DiamondGlyphPolyData &&
      this->HandleRepresentation)
    {
    this->HandleRepresentation->SetHandle(this->DiamondGlyphPolyData);
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (!sr)
      {
      vtkErrorMacro("SetGlyphToDiamond3D: unable to get seed representation");
      return -1;
      }
    // set for any new ones
    sr->SetHandleRepresentation(this->HandleRepresentation);
    // now change all the old ones
    int enabledState = this->Widget->GetEnabled();
    this->Widget->EnabledOff();
    int numSeeds = sr->GetNumberOfSeeds();
    for (int n = 0; n < numSeeds; n++)
      {
      this->Widget->GetSeed(n)->SetRepresentation(this->HandleRepresentation);
      }
    this->Widget->SetEnabled(enabledState);
    this->Widget->CompleteInteraction();
    this->GlyphType = vtkSlicerSeedWidgetClass::Diamond3D;
    return 1;
    }
  else
    {
    return -1;
    }
}

//---------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::SetGlyphTo2D(int val)
{
  if (this->GlyphType == vtkSlicerSeedWidgetClass::TwoD + val)
    {
    return 0;
    }
  if (this->Glyph &&
      this->OrientedHandleRepresentation &&
      this->Widget &&
      this->Widget->GetRepresentation())
    {
    vtkDebugMacro("SetGlyphTo2D: calling SetGlyphType with val = " << val);
    this->Glyph->SetGlyphType(val);
    this->Glyph->Update();
    this->OrientedHandleRepresentation->SetHandle(this->Glyph->GetOutput());
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (!sr)
      {
      vtkErrorMacro("SetGlyphTo2D: unable to get seed representation");
      return -1;
      }
    sr->SetHandleRepresentation(this->OrientedHandleRepresentation);
    this->GlyphType = vtkSlicerSeedWidgetClass::TwoD + val;
    return 1;
    }
  else
    {
    vtkErrorMacro("SetGlyphTo2D: something's null... ");
    return -1;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedLocked(int n, int lockedFlag)
{
   if (this->Widget == NULL)
    {
    return;
    }
   if (this->Widget->GetSeed(n) == NULL)
    {
    return;
    }

   vtkDebugMacro("SetNthSeedLocked: setting seed " << n << " pickable and dragable to !" << lockedFlag);
   this->Widget->GetSeed(n)->GetRepresentation()->SetPickable(!lockedFlag);
   this->Widget->GetSeed(n)->GetRepresentation()->SetDragable(!lockedFlag);
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetSeedsLocked(int lockedFlag)
{
  if (this->Widget == NULL)
    {
    return;
    }
  vtkDebugMacro("SetSeedsLocked: setting  process events on seed widget to !" << lockedFlag);
  // stop or start responding to all events
  this->Widget->SetProcessEvents(!lockedFlag);

}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedCamera(int n, vtkCamera *cam)
{
  if (!cam)
    {
    return;
    }
  if (this->Widget == NULL ||
      this->Widget->GetSeed(n) == NULL ||
      this->Widget->GetSeed(n)->GetRepresentation() == NULL)
    {
    return;
    }

  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(this->Widget->GetSeed(n)->GetRepresentation());
  if (rep)
    {
    vtkPropCollection *pc = vtkPropCollection::New();
    rep->GetActors(pc);
    vtkCollectionSimpleIterator pit;
    vtkProp *aProp;
    for (pc->InitTraversal(pit);
         (aProp = pc->GetNextProp(pit)); )
      {
      vtkFollower *a = vtkFollower::SafeDownCast(aProp);
      if (a)
        {
        a->SetCamera(cam);
        }
      }
    pc->Delete();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetCamera(vtkCamera *cam)
{
  if (!cam)
    {
    return;
    }
  if (this->Widget == NULL ||
      this->Representation == NULL)
    {
    return;
    }

  // iterate through actors and set their cameras
  int numSeeds = this->Representation->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    this->SetNthSeedCamera(n, cam);
    }
}
