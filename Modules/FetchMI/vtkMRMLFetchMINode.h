#ifndef __vtkMRMLFetchMINode_h
#define __vtkMRMLFetchMINode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkTagTable.h"
#include "vtkTagTableCollection.h"

#include "vtkFetchMIWin32Header.h"
#include <string>
#include <vector>


class VTK_FETCHMI_EXPORT vtkMRMLFetchMINode : public vtkMRMLNode
{
  public:
  static vtkMRMLFetchMINode *New();
  vtkTypeRevisionMacro(vtkMRMLFetchMINode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( ResourceDescription, vtkTagTable );
  vtkGetObjectMacro ( TagTableCollection, vtkTagTableCollection );
  vtkGetStringMacro ( SelectedServer );
  vtkSetStringMacro ( SelectedServer );
  void SetServer ( const char* );
  vtkGetStringMacro ( SelectedServiceType );
  vtkSetStringMacro ( SelectedServiceType );
  void SetServiceType ( const char* );
  vtkGetStringMacro ( ErrorMessage);
  vtkSetStringMacro ( ErrorMessage);

  // Description:
  // Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Set all default web servers
  virtual void SetKnownServers ( );
  
  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FetchMI"; };

  virtual void AddNewServer ( const char *name );
  virtual  int GetNumberOfKnownServers () {
  return KnownServers.size();
  }
  virtual const char *GetNthServer ( int ind ) {
  return KnownServers[ind].c_str();
  }
  
  virtual void RaiseErrorEvent ();
  virtual void RaiseErrorChoice ();
  
  // contains uri and SlicerDataType from queries
  vtkTagTable *ResourceDescription;

  vtkTagTableCollection *TagTableCollection;
  char *SelectedServer;
  char *SelectedServiceType;
  char *ErrorMessage;

  //BTX
  std::vector<std::string> KnownServers;
  //ETX

  //BTX
  enum
    {
      KnownServersModifiedEvent = 11000,
      SelectedServerModifiedEvent,
      TagsModifiedEvent,
      ResourceResponseReadyEvent,
      TagResponseReadyEvent,
      SaveSelectionEvent,
      RemoteIOErrorEvent,
      RemoteIOErrorChoiceEvent,
    };
  //ETX

 protected:
  vtkMRMLFetchMINode();
  ~vtkMRMLFetchMINode();
  vtkMRMLFetchMINode(const vtkMRMLFetchMINode&);
  void operator=(const vtkMRMLFetchMINode&);

};

#endif

