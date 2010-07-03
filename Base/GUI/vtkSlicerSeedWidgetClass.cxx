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
  this->GlyphScale = 1.0;
  this->TextScale = 1.0;
  
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
  double textscale[3] = {this->TextScale, this->TextScale, this->TextScale};
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
  this->Widget->ManagesCursorOff();
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

  os << indent << "ID to index map:\n";
  std::map<std::string, int>::iterator it;
  for ( it = this->PointIDToWidgetIndex.begin();
        it != this->PointIDToWidgetIndex.end();
        it++ )
    {
    os << indent << ((*it).first).c_str() << " => " << (*it).second << endl;
    }

  os << indent << "GlyphType  = " << this->GlyphType << "\n";
  os << indent << "GlyphScale = " << this->GlyphScale << "\n";
  os << indent << "TextScale  = " << this->TextScale << "\n";
  
}

//---------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::AddSeed(double *position, const char *pointID)
{
  int seedIndex = -1;
  if (this->GetWidget() == NULL)
    {
    return seedIndex;
    }

  if (this->GetWidget()->GetEnabled() == 0)
    {
    vtkDebugMacro("AddSeed: enabling the widget now that have a seed");
    this->GetWidget()->EnabledOn();
    }
  vtkHandleWidget *handle = this->GetWidget()->CreateNewHandle();
  
  // set the position if it was passed in
  if (handle)
    {
    // turn off managing the cursor, this doesn't stop the cursor from turning
    // into a hand, but in conjunction with the seed widget not managing it,
    // the cursor should only turn into a hand when can pick it up and move it
    handle->ManagesCursorOff();
    // save the id and index
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->GetWidget()->GetRepresentation());
    if (sr)
      {
      seedIndex = sr->GetNumberOfSeeds() - 1;
      this->PointIDToWidgetIndex[std::string(pointID)] = seedIndex;
      vtkDebugMacro("AddSeed: saved mapping of point id " << pointID << " to index " << seedIndex);
      }
    else
      {
      vtkErrorMacro("AddSeed: unable to get seed representation");
      }
    if (handle->GetEnabled() == 0)
      {
      vtkDebugMacro("AddSeed: turning on the new handle");
      handle->EnabledOn();
      }
    if (position != NULL &&
        handle->GetRepresentation())
      {
      vtkHandleRepresentation::SafeDownCast(handle->GetRepresentation())->SetWorldPosition(position);
      }
    else
      {
      vtkWarningMacro("AddSeed: unable to set world position, hand rep is " << (handle->GetRepresentation() == NULL ? "null" : "not null"));
      }
    }
  else
    {
    vtkErrorMacro("AddSeed: unable to create a new handle.");
    }
  return seedIndex;
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedPosition(int n, double *position)
{
  if (position == NULL ||
      this->GetWidget() == NULL)
    {
    vtkWarningMacro("SetNthSeedPosition: position or widget is null, n = " << n);
    return;
    }
  vtkHandleWidget *handle = this->Widget->GetSeed(n);
  if (!handle ||
      handle->GetRepresentation() == NULL)
    {
    vtkWarningMacro("SetNthSeedPosition: can't get handle or representation for seed " << n << ", total seeds = " <<  vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation())->GetNumberOfSeeds());
    return;
    }
  vtkHandleRepresentation::SafeDownCast(handle->GetRepresentation())->SetWorldPosition(position);
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::RemoveSeedByID(const char *id)
{
  if (id == NULL)
    {
    vtkErrorMacro("RemoveSeedByText: id is null");
    return;
    }
  // find the seed with this label text
  std::map<std::string, int>::iterator it;
  std::string idStr = std::string(id);
  it = this->PointIDToWidgetIndex.find(idStr);

  if (it != this->PointIDToWidgetIndex.end())
    {
    int index = it->second; // this->PointIDToWidgetIndex[id];
    vtkDebugMacro("RemoveSeedByID: for id " << id << " got index " << index);
    this->RemoveSeed(index);

    // erase the entry from the map
    this->PointIDToWidgetIndex.erase(it);
    // decrement the other indices that are higher than index
    std::map<std::string, int>::iterator itDecr;
    for (itDecr = this->PointIDToWidgetIndex.begin();
         itDecr != this->PointIDToWidgetIndex.end();
         itDecr++)
      {
      if (itDecr->second > index)
        {
        (itDecr->second)--;
        }
      }
    }
  else
    {
    vtkErrorMacro("RemoveSeedByID: could not find index for id " << id);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::RemoveSeed(int index)
{
  this->GetWidget()->DeleteSeed(index);
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::RemoveAllSeeds()
{
  if (this->Widget == NULL ||
      this->Widget->GetRepresentation() == NULL)
    {
    return;
    }
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    return;
    }
  int numSeeds = sr->GetNumberOfSeeds(); // this->Representation->GetNumberOfSeeds();
  for (int n = 0; n < numSeeds; n++)
    {
    // the number of seeds is reduced by one each time one is deleted
    this->GetWidget()->DeleteSeed(0);
    }
  vtkDebugMacro("RemoveAllSeeds: after removing " << numSeeds << " widget rep thinks it has " << sr->GetNumberOfSeeds());
  // clear out the map
  this->PointIDToWidgetIndex.clear();
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
  this->TextScale = scale;
 
  if (this->Widget &&
      this->Widget->GetRepresentation())
    {
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (sr)
      {
      // iterate through all seeds
      int numSeeds = sr->GetNumberOfSeeds();
      for (int n = 0; n < numSeeds; n++)
        {
        this->SetNthSeedTextScale(n, scale);
        }
      }
    }
}

//---------------------------------------------------------------------------
double *vtkSlicerSeedWidgetClass::GetTextScaleFromWidget()
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
  this->GlyphScale = scale;
  int numSeeds = sr->GetNumberOfSeeds();
  // iterate through the handles
  for (int n = 0; n < numSeeds; n++)
    {
    this->SetNthSeedGlyphScale(n, scale);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthSeedGlyphScale(int n, double scale)
{

  if (this->Widget == NULL || this->Widget->GetRepresentation() == NULL)
    {
    return;
    }
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    vtkWarningMacro("SetNthSeedGlyphScale: no representation for this widget.");
    return;
    }
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
      vtkWarningMacro("SetNthSeedGlyphScale: unable to get abstract polygonal handle representation 3d of handle representation for seed " << n);
      }
    }
  else
    {
    vtkWarningMacro("SetNthSeedGlyphScale: unable to get handle representation " << n);
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

  vtkWidgetRepresentation *widgetRep = NULL;
  widgetRep = this->Widget->GetSeed(n)->GetRepresentation();
  
  if (widgetRep)
    {
    vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
    rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(widgetRep);
    if (rep)
      {
      if (flag)
        {
        vtkDebugMacro("SetNthSeedVisibility: seed " << n << ", flag = " << flag << ", turning visib on");
        rep->VisibilityOn();
        rep->HandleVisibilityOn();
        rep->LabelVisibilityOn();
        }
      else
        {
        vtkDebugMacro("SetNthSeedVisibility: seed " << n << ", flag = " << flag << ", turning visib off");
        rep->VisibilityOff();
        rep->HandleVisibilityOff();
        rep->LabelVisibilityOff();
        }
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
  if (this->Widget &&
      this->Widget->GetRepresentation())
    {
    vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
    if (sr)
      {
      int numSeeds = sr->GetNumberOfSeeds();
      for (int n = 0; n < numSeeds; n++)
        {
        this->SetNthLabelTextVisibility(n, flag);
        }
      }
    }
}

//---------------------------------------------------------------------------
char * vtkSlicerSeedWidgetClass::GetNthLabelText(int n)
{
  if (this->Widget == NULL)
    {
    vtkErrorMacro("GetNthLabelText: the widget is null"); 
    return NULL;
    }
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("GetNthLabelText: unable to get the " << n << "th seed representation on the widget.");
    return NULL;
    }
  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (rep)
    {
    return rep->GetLabelText();
    }
  else
    {
    vtkErrorMacro("GetNthLabelText: unable to get the polygonal handle representation on the " << n << "th seed");
    }
  return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetNthLabelText(int n, const char *txt)
{
  if (this->Widget == NULL || txt == NULL)
    {
    vtkErrorMacro("SetNthLabelText: either the widget is null, or txt = " << (txt == NULL ? "null" : txt)); 
    return;
    }
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
      vtkErrorMacro("Unable to get property for handle representation on seed " << n);
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
//    int enabledState = this->Widget->GetEnabled();
//    this->Widget->EnabledOff();
    // this will make any new seeds be starbursts
    sr->SetHandleRepresentation(this->HandleRepresentation);
    /*
    // now set the rest of them
    int numSeeds = sr->GetNumberOfSeeds();
    for (int n = 0; n < numSeeds; n++)
      {
      this->Widget->GetSeed(n)->SetRepresentation(this->HandleRepresentation);
      }
    */
//    this->Widget->SetEnabled(enabledState);
//    this->Widget->CompleteInteraction();
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
  
  if (this->SphereSource == NULL || 
      this->Widget == NULL ||
      this->HandleRepresentation == NULL)
    {
    vtkErrorMacro("SetGlyphToSphere: null sphere source or widget or handle rep\n");
    return -1;
    }
  this->HandleRepresentation->SetHandle(this->SphereSource->GetOutput());
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetGlyphToSphere: unable to get seed representation");
    return -1;
    }
//  int enabledState = this->Widget->GetEnabled();
//  this->Widget->EnabledOff();
  // this will make any new seeds be represented as spheres
  sr->SetHandleRepresentation(this->HandleRepresentation);
  /*
  // now change all the old ones
  int numSeeds = sr->GetNumberOfSeeds();
  double currentTextScale =  this->GetTextScale();
  double textscale[3] = {currentTextScale, currentTextScale, currentTextScale};
  double currentGlyphScale = this->GetGlyphScale();
  for (int n = 0; n < numSeeds; n++)
    {
    // first get the old handle representation's text, so can copy the text
    // to the new handle
    char *oldText = this->GetNthLabelText(n);
  
    vtkPolygonalHandleRepresentation3D *hr = vtkPolygonalHandleRepresentation3D::New();
    hr->SetHandle(this->SphereSource->GetOutput());
    hr->SetLabelText(oldText);
    hr->SetLabelTextScale(textscale);
    hr->SetUniformScale(currentGlyphScale);
    hr->GetProperty()->SetColor(this->GetListColor());
    // set the new one
    this->Widget->GetSeed(n)->SetRepresentation(hr);
    hr->Delete();
    }
  */
//  this->Widget->SetEnabled(enabledState);
//  this->Widget->CompleteInteraction();

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
    /*
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
    */
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
      this->Widget->GetRepresentation() == NULL)
    {
    return;
    }

  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
  if (sr)
    {
    // iterate through actors and set their cameras
    int numSeeds = sr->GetNumberOfSeeds();
    for (int n = 0; n < numSeeds; n++)
      {
      this->SetNthSeedCamera(n, cam);
      }
    }
}

//--------------------------------------------------------------------------
int vtkSlicerSeedWidgetClass::GetNthSeedExists(int n)
{

  if (this->Widget == NULL)
    {
    return 0;
    }
  if (this->Widget->GetSeed(n) == NULL)
    {
    return 0;
    }
  return 1;
}

//--------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SetOpacity(double opacity)
{
  if (!this->GetWidget() ||
      !(this->GetWidget()->GetRepresentation()))
    {
    return;
    }
  // set it on the list property
  this->GetProperty()->SetOpacity(opacity);

  // and on all the current ones
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->GetWidget()->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetOpacity: unable to get the seed representation on the widget.");
    return;
    }
  int numSeeds = sr->GetNumberOfSeeds();

  for (int n = 0; n < numSeeds; n++)
    {
    vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
    rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
    if (rep &&
        rep->GetProperty())
      {
      rep->GetProperty()->SetOpacity(opacity);
      }
    else
      {
      vtkErrorMacro("SetOpacity: Unable to get rep or property for seed " << n);
      }
    }
}

//--------------------------------------------------------------------------
void  vtkSlicerSeedWidgetClass::SetMaterialProperties(double opacity, double ambient, double diffuse, double specular, double power)
{
  if (!this->GetWidget() ||
      !(this->GetWidget()->GetRepresentation()))
    {
    return;
    }
  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->GetWidget()->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetMaterialProperties: unable to get the seed representation on the widget.");
    return;
    }
  int numSeeds = sr->GetNumberOfSeeds();
  
  for (int n = 0; n < numSeeds; n++)
    {
    this->SetNthSeedMaterialProperties(n, opacity, ambient, diffuse, specular, power);
    }
}

//--------------------------------------------------------------------------
void  vtkSlicerSeedWidgetClass::SetNthSeedMaterialProperties(int n,
                                            double opacity, double ambient, double diffuse, double specular, double power)
{
 if (!this->GetWidget() ||
      !this->GetWidget()->GetRepresentation())
    {
    return;
    }

  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->GetWidget()->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetNthSeed: unable to get the seed representation on the widget.");
    return;
    }

  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (rep)
    {
    vtkProperty *prop = NULL;
    prop = rep->GetProperty();
    // update the properties
    if (prop != NULL)
      {
      prop->SetOpacity(opacity);
      prop->SetAmbient(ambient);
      prop->SetDiffuse(diffuse);
      prop->SetSpecular(specular);
      prop->SetSpecularPower(power);
      prop->Modified();
      }
    else
      {
      vtkWarningMacro("SetNthSeedMaterialProperties: could not get the property for " << n << "th seed");
      }
    // and the same for the text actor
    vtkProperty *textProp = NULL;
    if (rep->GetLabelTextActor())
      {
      textProp = rep->GetLabelTextActor()->GetProperty();
      if (textProp)
        {
        textProp->SetOpacity(opacity);
        textProp->SetAmbient(ambient);
        textProp->SetDiffuse(diffuse);
        textProp->SetSpecular(specular);
        textProp->SetSpecularPower(power);
        textProp->Modified();
        }
      }
    }
}

//--------------------------------------------------------------------------
void  vtkSlicerSeedWidgetClass::SetNthSeed(int n, vtkCamera *cam, double *position, const char *text,
                                          int visibilityFlag, int lockedFlag, int selectedFlag,
                                          double *listColour, double *listSelectedColour,
                                          double textScale, double glyphScale,
                                          int glyphType,
                                          double opacity, double ambient, double diffuse, double specular, double power)
{
  if (!this->GetWidget() ||
      !this->GetWidget()->GetRepresentation())
    {
    return;
    }

  vtkSeedRepresentation *sr = vtkSeedRepresentation::SafeDownCast(this->GetWidget()->GetRepresentation());
  if (!sr)
    {
    vtkErrorMacro("SetNthSeed: unable to get the seed representation on the widget.");
    return;
    }

  vtkAbstractPolygonalHandleRepresentation3D *rep = NULL;
  rep = vtkAbstractPolygonalHandleRepresentation3D::SafeDownCast(sr->GetHandleRepresentation(n));
  if (!rep)
    {
    vtkErrorMacro("SetNthSeed: Unable to get rep for seed " << n);
    return;
    }
  
  // update the camera
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
  
  // update the glyph type
  vtkDebugMacro("Not updating the glyph type here");
  
  // update the position
  rep->SetWorldPosition(position);
  // update the text
  rep->SetLabelText(text);
  
  // update the flags
  if (visibilityFlag)
    {
    rep->VisibilityOn();
    rep->HandleVisibilityOn();
    rep->LabelVisibilityOn();
    }
  else
    {
    rep->VisibilityOff();
    rep->HandleVisibilityOff();
    rep->LabelVisibilityOff();
    }
  rep->SetPickable(!lockedFlag);
  rep->SetDragable(!lockedFlag);
  // update the colours
  // check if there's a change first
  double *widgetColour = this->GetListColor();
  double *widgetSelectedColour = this->GetListSelectedColor();
  if (widgetColour[0] != listColour[0] ||
      widgetColour[1] != listColour[1] ||
      widgetColour[2] != listColour[2])
    {
    this->SetListColor(listColour);
    }
  if (widgetSelectedColour[0] != listSelectedColour[0] ||
      widgetSelectedColour[1] != listSelectedColour[1] ||
      widgetSelectedColour[2] != listSelectedColour[2])
    {
    this->SetListSelectedColor(listSelectedColour);
    }
  // update the scales
  double s[3];
  s[0] = s[1] = s[2] = textScale;
  rep->SetLabelTextScale(s);
  rep->SetUniformScale(glyphScale);
  
  // update selected colour on text
  vtkProperty *textProp = NULL;
  if (rep->GetLabelTextActor())
    {
    textProp = rep->GetLabelTextActor()->GetProperty();
    if (textProp)
      {
      if (selectedFlag)
        {
        textProp->SetColor(this->GetListSelectedColor());
        }
      else
        {
        textProp->SetColor(this->GetListColor()); 
        }
      }
    }
  else
    {
    vtkWarningMacro("SetNthSeed: could not get label text actor for seed " << n);
    }
  
  vtkProperty *prop = NULL;
  prop = rep->GetProperty();
  // update the colours properties
  if (prop != NULL)
    {
    if (selectedFlag)
      {
      prop->SetColor(this->GetListSelectedColor());
      }
    else
      {
      prop->SetColor(this->GetListColor());
      }
    }
  // set material properties
  this->SetNthSeedMaterialProperties(n, opacity, ambient, diffuse, specular, power);
  
  // trigger a modified event
  rep->Modified();
}


//--------------------------------------------------------------------------
std::string vtkSlicerSeedWidgetClass::GetIDFromIndex(int index)
{
  std::string returnString = std::string("");
  std::map<std::string, int>::iterator it;
  for ( it = this->PointIDToWidgetIndex.begin();
        it != this->PointIDToWidgetIndex.end();
        it++ )
    {
    if (it->second == index)
      {
      vtkDebugMacro("GetIDFromIndex: found index " << index << ", id = " << it->first);
      returnString = it->first;
      return returnString;
      }
    }
  return returnString;
}

//--------------------------------------------------------------------------
int  vtkSlicerSeedWidgetClass::GetIndexFromID(const char *id)
{
  if (id == NULL)
    {
    vtkErrorMacro("GetIndexFromID: null input id!");
    return -1;
    }
  std::string idStr = std::string(id);
  if (this->PointIDToWidgetIndex.find(idStr) == this->PointIDToWidgetIndex.end())
    {
    // returning -1 lets the caller know it needs to add a seed
    vtkDebugMacro("GetIndexFromID: Unable to find id " << id);
    return -1;
    }
  else
    {
    // if it's not already in the array, this would add an element
    return this->PointIDToWidgetIndex[idStr];
    }
}

//--------------------------------------------------------------------------
void vtkSlicerSeedWidgetClass::SwapIndexIDs(int index1, int index2)
{
  std::string id1 = this->GetIDFromIndex(index1);
  std::string id2 = this->GetIDFromIndex(index2);

  if (id1.compare("") == 0)
    {
    vtkErrorMacro("SwapIndexIDs: could not get an id from index1 " << index1);
    return;
    }
  if (id2.compare("") == 0)
    {
    vtkErrorMacro("SwapIndexIDs: could not get an id from index2 " << index2);
    return;
    }

  // swap the seeds associated with the ids
  this->PointIDToWidgetIndex[id1] = index2;
  this->PointIDToWidgetIndex[id2] = index1;
}
