#ifndef __vtkSlicerSeedWidgetClass_h
#define __vtkSlicerSeedWidgetClass_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkObject.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"

class vtkSlicerGlyphSource2D;
class vtkSeedWidget;
class vtkPolygonalHandleRepresentation3D;
class vtkOrientedPolygonalHandleRepresentation3D;
class vtkSeedRepresentation;
class vtkPolygonalSurfacePointPlacer;
class vtkProperty;
class vtkCamera;
// a custom class encapsulating the widget classes needed to display the
// fiducial list in 3D
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSeedWidgetClass : public vtkObject
{
public:
  static vtkSlicerSeedWidgetClass* New();
  vtkTypeRevisionMacro(vtkSlicerSeedWidgetClass, vtkObject);
  vtkSlicerSeedWidgetClass();
  ~vtkSlicerSeedWidgetClass();
  // Description:
  // print widgets to output stream
  void PrintSelf ( ostream& os, vtkIndent indent );

  // Description:
  // accessor methods
  vtkGetObjectMacro(HandleRepresentation, vtkPolygonalHandleRepresentation3D);
  vtkGetObjectMacro(OrientedHandleRepresentation, vtkOrientedPolygonalHandleRepresentation3D);
  vtkGetObjectMacro(Representation, vtkSeedRepresentation);
  vtkGetObjectMacro(Widget, vtkSeedWidget);
  vtkGetObjectMacro(ModelPointPlacer, vtkPolygonalSurfacePointPlacer);

  // Description:
  // access the custom 2d poly data
  vtkGetObjectMacro(Glyph, vtkSlicerGlyphSource2D);

  // Description:
  // Get the poly data representing the 3d diamond glyph
  vtkGetObjectMacro(DiamondGlyphPolyData, vtkPolyData);
  vtkSetObjectMacro(DiamondGlyphPolyData, vtkPolyData);

  // Description:
  // Get/Set the sphere source for the 3d sphere glyph
  vtkGetObjectMacro(SphereSource, vtkSphereSource);
  vtkSetObjectMacro(SphereSource, vtkSphereSource);
  
  // Description:
  // add a seed at location passed in, or just add it if position is NULL
  void AddSeed(double *position);

  // Description:
  // remove the nth seed
  void RemoveSeed(int index);

  // Description:
  // remove all seeds
  void RemoveAllSeeds();

  // Description:
  // Return the representation property, useful for setting widget level
  // colour, power, ambient, diffuse, etc.
  vtkProperty *GetProperty();

  // Description:
  // set the colour of the glyphs and text, saving the value in a local var ListColour
  //void SetColor(double *col);
//  void SetColor(double r, double g, double b);
   
  /// Description:
  // set the selected colour of the glyphs and text, saving the value in a
  // local var ListSelectedColor
  //void SetSelectedColor(double *col);
//  void SetSelectedColor(double r, double g, double b);
 

  // Description:
  // set the label text scale on an individual seed
  void SetNthSeedTextScale(int n, double scale);
  // Description:
  // set the label text scale on the handle representation and on all seeds
  void SetTextScale(double scale);
  // Description:
  // returns the label text scale on the handle representation, null if not defined
  double * GetTextScale();

  // Description:
  // set the scale on the glpyhs
  void SetGlyphScale(double scale);
  // Description:
  // get the scale of the glyphs
  double GetGlyphScale();

  // Description:
  // set the glyphs to different poly datas. Return 1 if changed, 0 if not, -1
  // on error
  int SetGlyphToStarburst();
  int SetGlyphToSphere();
  int SetGlyphToDiamond3D();
  int SetGlyphTo2D(int val);

//BTX
  // Description:
  // the different kinds of glyph types
  enum GlyphTypes
  {
    Starburst = 1,
    Sphere = 2,
    Diamond3D = 3,
    TwoD = 4,
  };
//ETX

  // Description:
  // Set the glyph and text visibility for seed
  void SetNthSeedVisibility(int n, int flag);

  // Description:
  // Set the label text visibility for seeds to off if 0, on otherwise
  void SetNthLabelTextVisibility(int n, int flag);
  void SetLabelTextVisibility(int flag);
  // Description:
  // Set the label text for the nth seed
  void SetNthLabelText(int n, const char *txt);

  // Description:
  // Modify a seed's position
  void SetNthSeedPosition(int n, double *position);
  
  // Description:
  // Set the selected property on the nth seed (uses Highlight)
  void SetNthSeedSelected(int n, int selectedFlag);

  // Description:
  // lock the seeds so they can't be moved. UNDER CONSTRUCTION
  void SetNthSeedLocked(int n, int lockedFlag);
  void SetSeedsLocked(int lockedFlag);

  // Descripton:
  // update the camera for all the followers associated with this individual
  // seed
  void SetNthSeedCamera(int n, vtkCamera *cam);
  // Description:
  // update the camera for all the followers associated with this seed widget
  void SetCamera(vtkCamera *cam);

  // Description:
  // set/get colours that were set from the list this widget represents
  vtkGetVectorMacro(ListColor, double, 3);
  vtkSetVectorMacro(ListColor, double, 3);
  vtkGetVectorMacro(ListSelectedColor, double, 3);
  vtkSetVectorMacro(ListSelectedColor, double, 3);

  // Description:
  // get the current glyph type
  vtkGetMacro(GlyphType, int);
protected:

  // Description:
  // input to the handle representation
  vtkSlicerGlyphSource2D *Glyph;

  // Description:
  // the poly data representing the 3d diamond glyph
  vtkPolyData * DiamondGlyphPolyData;
  
  // Description:
  // represents the sphere 3d glyph
  vtkSphereSource * SphereSource;
  
  // Description:
  // the representation for the seed point handles
  vtkPolygonalHandleRepresentation3D *HandleRepresentation;
  // Description:
  // the representation for the oriented seed point handles
  vtkOrientedPolygonalHandleRepresentation3D *OrientedHandleRepresentation;
  
  // Description:
  // the representation for the seeds
  vtkSeedRepresentation *Representation;
  // Description:
  // the top level widget used to bind together the handles and the representation
  vtkSeedWidget *Widget;
  // Descriptinon:
  // point placers to constrain the points to a model's polydata surface
  vtkPolygonalSurfacePointPlacer *ModelPointPlacer;

  // Description:
  // save the color and selected colour values as passed in from the node
  // represented by this widget
  double ListColor[3];
  double ListSelectedColor[3];

  // Description:
  // the current glyph type
  int GlyphType;
  
private:
  vtkSlicerSeedWidgetClass ( const vtkSlicerSeedWidgetClass& ); // Not implemented
  void operator = ( const vtkSlicerSeedWidgetClass& ); // Not implemented
};

#endif
