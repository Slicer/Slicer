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

#ifndef __vtkMRMLMarkupsFiducialNode_h
#define __vtkMRMLMarkupsFiducialNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkSmartPointer.h>

/// \brief MRML node to represent a fiducial markup
/// Fiducial Markups nodes contain a list of fiducial points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsFiducialNode *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetIcon() {return ":/Icons/MarkupsMouseModePlace.png";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MarkupsFiducial";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Calls the superclass UpdateScene
  void UpdateScene(vtkMRMLScene *scene);

  /// Alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );


  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Return a cast display node, returns null if none
  vtkMRMLMarkupsDisplayNode *GetMarkupsDisplayNode();

  // Wrapping some of the generic markup methods

  /// Get the number of fiducials in this node
  int GetNumberOfFiducials() { return this->GetNumberOfMarkups(); } ;
  /// Add a new fiducial from x,y,z coordinates and return the fiducial index
  int AddFiducial(double x, double y, double z);
  int AddFiducial(double x, double y, double z, std::string label);
  /// Add a new fiducial from an array and return the fiducial index
#if (VTK_MAJOR_VERSION >= 6)
  int AddFiducialFromArray(double pos[3], std::string label = std::string());
#else
  int AddFiducialFromArray(double pos[3]);
  int AddFiducialFromArray(double pos[3], std::string label);
#endif
  /// Get the position of the nth fiducial, returning it in the pos array
  void GetNthFiducialPosition(int n, double pos[3]);
  /// Set the position of the nth fiducial from x, y, z coordinates
  void SetNthFiducialPosition(int n, double x, double y, double z);
  /// Set the position of the nth fiducial from a double array
  void SetNthFiducialPositionFromArray(int n, double pos[3]);
  /// Get selected property on Nth fiducial
  bool GetNthFiducialSelected(int n = 0);
  /// Set selected property on Nth fiducial
  void SetNthFiducialSelected(int n, bool flag);
  /// Get visibility property on Nth fiducial
  bool GetNthFiducialVisibility(int n = 0);
  /// Set visibility property on Nth fiducial. If the visibility is set to
  /// true on the node/list as a whole, the nth fiducial visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all fiducials are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthFiducialVisibility(int n, bool flag);
  /// Get label on nth fiducial
  std::string GetNthFiducialLabel(int n = 0);
  /// Set label on nth fiducial
  void SetNthFiducialLabel(int n, std::string label);
  /// Get associated node id on nth fiducial
  std::string GetNthFiducialAssociatedNodeID(int n = 0);
  /// Set associated node id on nth fiducial
  void SetNthFiducialAssociatedNodeID(int n, const char* id);
  /// Set world coordinates on nth fiducial
  void SetNthFiducialWorldCoordinates(int n, double coords[4]);
  /// Get world coordinates on nth fiducial
  void GetNthFiducialWorldCoordinates(int n, double coords[4]);

protected:
  vtkMRMLMarkupsFiducialNode();
  ~vtkMRMLMarkupsFiducialNode();
  vtkMRMLMarkupsFiducialNode(const vtkMRMLMarkupsFiducialNode&);
  void operator=(const vtkMRMLMarkupsFiducialNode&);

};

#endif
