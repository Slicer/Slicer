/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkMRMLMarkupsDisplayNode - MRML node to represent display properties for markups
// .SECTION Description
// vtkMRMLMarkupsDisplayNode nodes store display properties of markups,
// keeping elements that are applicable to all parts of the markups in this superclass
//

#ifndef __vtkMRMLMarkupsDisplayNode_h
#define __vtkMRMLMarkupsDisplayNode_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"

#include "vtkMRMLDisplayNode.h"

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLMarkupsDisplayNode *New();
  vtkTypeMacro ( vtkMRMLMarkupsDisplayNode,vtkMRMLDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  // Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  // Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );

  // Get node XML tag name (like Volume, Markups)
  virtual const char* GetNodeTagName() {return "MarkupsDisplay";};

  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  /// Set the text scale of the associated text.
  vtkGetMacro(TextScale,double);
  vtkSetMacro(TextScale,double);

  /// Which kind of glyph should be used to display this markup?
  /// Vertex2D is supposed to start at 1
  enum GlyphShapes
  {
    GlyphMin = 1,
    Vertex2D = GlyphMin,
    Dash2D,
    Cross2D,
    ThickCross2D,
    Triangle2D,
    Square2D,
    Circle2D,
    Diamond2D,
    Arrow2D,
    ThickArrow2D,
    HookedArrow2D,
    StarBurst2D,
    Sphere3D,
    Diamond3D,
    GlyphMax = Sphere3D,
  };
  /// Return the min/max glyph types, for iterating over them in tcl
  int GetMinimumGlyphType() { return vtkMRMLMarkupsDisplayNode::GlyphMin; };
  int GetMaximumGlyphType() { return vtkMRMLMarkupsDisplayNode::GlyphMax; };

  /// The glyph type used to display this fiducial
  void SetGlyphType(int type);
  vtkGetMacro(GlyphType, int);
  /// Returns 1 if the type is a 3d one, 0 else
  int GlyphTypeIs3D(int glyphType);
  int GlyphTypeIs3D() { return this->GlyphTypeIs3D(this->GlyphType); };

  /// Return a string representing the glyph type, set it from a string
  const char* GetGlyphTypeAsString();
  const char* GetGlyphTypeAsString(int g);
  void SetGlyphTypeFromString(const char *glyphString);

  /// Get/Set for Symbol scale
  void SetGlyphScale(double scale);
  vtkGetMacro(GlyphScale,double);

  /// An event that lets the markups logic know to reset this node to the
  /// default values
  enum
  {
    ResetToDefaultsEvent = 19001,
  };

  /// Set SliceProjection flag that controls if the projection of markups
  /// is visible or not in 2D viewers on slices on which it is normally
  /// not visible.
  /// Off by default
  /// \sa SliceIntersectionVisibilty, SliceProjectionColor
  vtkSetMacro(SliceProjection, int);
  vtkGetMacro(SliceProjection, int);

  /// Set SliceProjection to On
  inline void SliceProjectionOn();

  /// Set SliceProjection to Off
  inline void SliceProjectionOff();

  /// Set color of the projection on the 2D viewers
  /// White (1.0, 1.0, 1.0) by default.
  vtkSetVector3Macro(SliceProjectionColor, double);
  vtkGetVector3Macro(SliceProjectionColor, double);

  /// Set opacity of projection on the 2D viewers
  /// 0.6 by default
  vtkSetClampMacro(SliceProjectionOpacity, double, 0.0, 1.0);
  vtkGetMacro(SliceProjectionOpacity, double);

  /// Set projection color to be the same as the fiducial color
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionUseFiducialColorOn();

  /// Manually set projection color
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionUseFiducialColorOff();

  /// Return true if the slice projection use fiducial color option
  /// is on, false otherwise
  inline bool GetSliceProjectionUseFiducialColor();

  /// Set projection's view different if under/over/in the plane
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionOutlinedBehindSlicePlaneOn();

  /// Set projection's view the same if under/over/in the plane
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionOutlinedBehindSlicePlaneOff();

  /// Return true if the outline behind slice plane setting is turned
  /// on, false otherwise
  inline bool GetSliceProjectionOutlinedBehindSlicePlane();

  /// ProjectionUseFiducialColor : Set projection color the same as the
  /// markup color
  /// ProjectionOutlinedBehindSlicePlane : Different shape and opacity when
  /// markup is on top of the slice plane, or under
  /// Projection Off, UseFiducialColor, OutlinedBehindSlicePlane by default
  ///\enum SliceProjectionFlag
  enum SliceProjectionFlag
  {
    ProjectionOff = 0x00,
    ProjectionOn = 0x01,
    ProjectionUseFiducialColor = 0x02,
    ProjectionOutlinedBehindSlicePlane = 0x04
  };

protected:
  vtkMRMLMarkupsDisplayNode();
  ~vtkMRMLMarkupsDisplayNode();
  vtkMRMLMarkupsDisplayNode( const vtkMRMLMarkupsDisplayNode& );
  void operator= ( const vtkMRMLMarkupsDisplayNode& );

  double TextScale;
  int GlyphType;
  double GlyphScale;
  static const char* GlyphTypesNames[GlyphMax+2];

  int SliceProjection;
  double SliceProjectionColor[3];
  double SliceProjectionOpacity;
};

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsDisplayNode::ProjectionOn);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsDisplayNode::ProjectionOn);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionUseFiducialColorOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsDisplayNode::ProjectionUseFiducialColor);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionUseFiducialColorOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsDisplayNode::ProjectionUseFiducialColor);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayNode::GetSliceProjectionUseFiducialColor()
{
  if (this->GetSliceProjection() &
      this->ProjectionUseFiducialColor)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionOutlinedBehindSlicePlaneOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SliceProjectionOutlinedBehindSlicePlaneOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsDisplayNode::ProjectionOutlinedBehindSlicePlane);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayNode::GetSliceProjectionOutlinedBehindSlicePlane()
{
  if (this->GetSliceProjection() &
      this->ProjectionOutlinedBehindSlicePlane)
    {
    return true;
    }
  else
    {
    return false;
    }
}

#endif
