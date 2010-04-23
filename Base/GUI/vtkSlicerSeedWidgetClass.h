#ifndef __vtkSlicerSeedWidgetClass_h
#define __vtkSlicerSeedWidgetClass_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkObject.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"

#include <map>

class vtkSlicerGlyphSource2D;
class vtkSeedWidget;
class vtkPolygonalHandleRepresentation3D;
class vtkOrientedPolygonalHandleRepresentation3D;
class vtkSeedRepresentation;
class vtkPolygonalSurfacePointPlacer;
class vtkProperty;
class vtkCamera;
/// a custom class encapsulating the widget classes needed to display the
/// fiducial list in 3D
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSeedWidgetClass : public vtkObject
{
public:
  static vtkSlicerSeedWidgetClass* New();
  vtkTypeRevisionMacro(vtkSlicerSeedWidgetClass, vtkObject);
  vtkSlicerSeedWidgetClass();
  ~vtkSlicerSeedWidgetClass();
  /// 
  /// print widgets to output stream
  void PrintSelf ( ostream& os, vtkIndent indent );

  /// 
  /// accessor methods
  vtkGetObjectMacro(HandleRepresentation, vtkPolygonalHandleRepresentation3D);
  vtkGetObjectMacro(OrientedHandleRepresentation, vtkOrientedPolygonalHandleRepresentation3D);
//  vtkGetObjectMacro(Representation, vtkSeedRepresentation);
  vtkGetObjectMacro(Widget, vtkSeedWidget);
  vtkGetObjectMacro(ModelPointPlacer, vtkPolygonalSurfacePointPlacer);

  /// 
  /// access the custom 2d poly data
  vtkGetObjectMacro(Glyph, vtkSlicerGlyphSource2D);

  /// 
  /// Get the poly data representing the 3d diamond glyph
  vtkGetObjectMacro(DiamondGlyphPolyData, vtkPolyData);
  vtkSetObjectMacro(DiamondGlyphPolyData, vtkPolyData);

  /// 
  /// Get/Set the sphere source for the 3d sphere glyph
  vtkGetObjectMacro(SphereSource, vtkSphereSource);
  vtkSetObjectMacro(SphereSource, vtkSphereSource);

  /// 
  /// get the current glyph type
  vtkGetMacro(GlyphType, int);
  ///
  /// get the text scale stored in this class
  vtkGetMacro(TextScale, double);
  ///
  /// get the glyph scale as stored in this class
  vtkGetMacro(GlyphScale, double);
  /// 
  /// set/get colours that were set from the list this widget represents
  vtkGetVectorMacro(ListColor, double, 3);
  vtkSetVectorMacro(ListColor, double, 3);
  vtkGetVectorMacro(ListSelectedColor, double, 3);
  vtkSetVectorMacro(ListSelectedColor, double, 3);

  
  /// 
  /// add a seed at location passed in, or just add it if position is NULL
  /// save the id for removing it later. Returns the index of the added seed.
  int AddSeed(double *position, const char *id);

  ///
  /// remove a seed using it's id string
  void RemoveSeedByID(const char *id);
  
  /// 
  /// remove the nth seed
  void RemoveSeed(int index);

  /// 
  /// remove all seeds
  void RemoveAllSeeds();

  /// 
  /// Return the representation property, useful for setting widget level
  /// colour, power, ambient, diffuse, etc.
  vtkProperty *GetProperty();
 
  /// 
  /// set the label text scale on an individual seed
  void SetNthSeedTextScale(int n, double scale);
  /// 
  /// set the label text scale on the handle representation and on all seeds
  void SetTextScale(double scale);
  /// 
  /// returns the label text scale on the handle representation, null if not defined
  double * GetTextScaleFromWidget();
 
  /// 
  /// set the scale on the glpyhs, iterates through all the handles in the
  /// seed representation and sets uniform scale.
  void SetGlyphScale(double scale);
  void SetNthSeedGlyphScale(int n, double scale);

  /// 
  /// set the glyphs to different poly datas. Return 1 if changed, 0 if not, -1
  /// on error
  int SetGlyphToStarburst();
  int SetGlyphToSphere();
  int SetGlyphToDiamond3D();
  int SetGlyphTo2D(int val);

//BTX
  /// 
  /// the different kinds of glyph types
  enum GlyphTypes
  {
    Starburst = 1,
    Sphere = 2,
    Diamond3D = 3,
    TwoD = 4,
  };
//ETX

  /// 
  /// Set the glyph and text visibility for seed
  void SetNthSeedVisibility(int n, int flag);

  /// 
  /// Set the label text visibility for seeds to off if 0, on otherwise
  void SetNthLabelTextVisibility(int n, int flag);
  void SetLabelTextVisibility(int flag);
  /// 
  /// Set/Get the label text for the nth seed. Get returns NULL on error.
  void SetNthLabelText(int n, const char *txt);
  char *GetNthLabelText(int n);
  
  /// 
  /// Modify a seed's position
  void SetNthSeedPosition(int n, double *position);
  
  /// 
  /// Set the selected property on the nth seed (uses Highlight)
  void SetNthSeedSelected(int n, int selectedFlag);

  /// 
  /// lock the seeds so they can't be moved. UNDER CONSTRUCTION
  void SetNthSeedLocked(int n, int lockedFlag);
  void SetSeedsLocked(int lockedFlag);

  ///
  /// update the camera for all the followers associated with this individual
  /// seed
  void SetNthSeedCamera(int n, vtkCamera *cam);
  /// 
  /// update the camera for all the followers associated with this seed widget
  void SetCamera(vtkCamera *cam);


  ///
  /// Returns 1 if the nth seed in the list exists already, 0 otherwise. Will
  /// return 0 if the whole seed widget doesn't exist
  int GetNthSeedExists(int n);

  ///
  /// Set the opacity for all seed reps on the list
  void SetOpacity(double opacity);

  //
  // set the material properties on all seeds
  void  SetMaterialProperties(double opacity, double ambient, double diffuse, double specular, double power);
  //
  // set the material properties on the nth seed
  void  SetNthSeedMaterialProperties(int n,
                                     double opacity, double ambient, double diffuse, double specular, double power);
  
  ///
  /// Set properties on a seed
  void SetNthSeed(int n, vtkCamera *cam, double *position, const char *text,
                  int visibilityFlag, int lockedFlag, int selectedFlag,
                  double *colour, double *selectedColour,
                  double textScale, double glyphScale,
                  int glyphType,
                  double opacity, double ambient, double diffuse, double specular, double power);

  ///
  /// Get the fiducial id that the nth seed is working from
//BTX
  std::string GetIDFromIndex(int index);
//ETX
  ///
  /// Get the seed index used to represent the fiducial id 'id'. Returns -1
  /// if not found.
  int GetIndexFromID(const char *id);
  
  ///
  /// Swap two indices, called when two fiducials swap places in the fid list
  /// node. Each will need to be updated.
  void SwapIndexIDs(int index1, int index2);
  
protected:

  vtkGetObjectMacro(Representation, vtkSeedRepresentation);
  
  /// 
  /// input to the handle representation
  vtkSlicerGlyphSource2D *Glyph;

  /// 
  /// the poly data representing the 3d diamond glyph
  vtkPolyData * DiamondGlyphPolyData;
  
  /// 
  /// represents the sphere 3d glyph
  vtkSphereSource * SphereSource;
  
  /// 
  /// the representation for the seed point handles
  vtkPolygonalHandleRepresentation3D *HandleRepresentation;
  /// 
  /// the representation for the oriented seed point handles
  vtkOrientedPolygonalHandleRepresentation3D *OrientedHandleRepresentation;
  
  /// 
  /// the representation for the seeds
  vtkSeedRepresentation *Representation;
  /// 
  /// the top level widget used to bind together the handles and the representation
  vtkSeedWidget *Widget;
  /// Descriptinon:
  /// point placers to constrain the points to a model's polydata surface
  vtkPolygonalSurfacePointPlacer *ModelPointPlacer;

  /// 
  /// save the color and selected colour values as passed in from the node
  /// represented by this widget
  double ListColor[3];
  double ListSelectedColor[3];

  /// 
  /// the current glyph type
  int GlyphType;

  ///
  /// the current glyph and text scale
  double GlyphScale;
  double TextScale;

  ///
  /// link the mrml point ids with the widget seed indices
//BTX
  std::map<std::string, int> PointIDToWidgetIndex;
//ETX

private:
  vtkSlicerSeedWidgetClass (const vtkSlicerSeedWidgetClass& ); /// Not implemented
  void operator = ( const vtkSlicerSeedWidgetClass& ); /// Not implemented
};

#endif
