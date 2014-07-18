/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.h,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/

#ifndef __vtkMRMLScene_h
#define __vtkMRMLScene_h

/// Current MRML version
/// needs to be changed when incompatible MRML changes introduced
#define CURRENT_MRML_VERSION "Slicer4.4.0"

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <list>
#include <map>
#include <vector>
#include <string>
#include <set>

class vtkCacheManager;
class vtkDataIOManager;
class vtkTagTable;

class vtkCallbackCommand;
class vtkCollection;
class vtkGeneralTransform;
class vtkURIHandler;
class vtkMRMLNode;
class vtkMRMLSceneViewNode;

/// \brief A set of MRML Nodes that supports serialization and undo/redo.
///
/// vtkMRMLScene represents and provides methods to manipulate a list of
/// MRML objects. The list is core and duplicate entries are not prevented.
//
/// \sa vtkMRMLNode
/// \sa vtkCollection
class VTK_MRML_EXPORT vtkMRMLScene : public vtkObject
{
  ///
  /// make the vtkMRMLSceneViewNode a friend since it has internal vtkMRMLScene
  /// so that it can call protected methods, for example UpdateNodeIDs()
  /// but that's the only class that is allowed to do so
  friend class vtkMRMLSceneViewNode;

public:
  static vtkMRMLScene *New();
  vtkTypeMacro(vtkMRMLScene, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Set URL (file name) of the scene
  void SetURL(const char *url);

  /// Get URL (file name) of the scene
  const char *GetURL();

  /// Set Root directory, where URL is pointing
  void SetRootDirectory(const char *dir);

  /// Get Root directory, where URL is pointing
  const char *GetRootDirectory();

  /// Create new scene from URL
  /// Returns nonzero on success
  int Connect();

  /// Add the scene into the existing scene (no clear) from \a URL file or
  /// from \sa SceneXMLString XML string.
  /// Returns nonzero on success
  /// \sa SetURL(), GetLoadFromXMLString(), SetSceneXMLString()
  int Import();

  /// Save scene into URL
  /// Returns nonzero on success
  int Commit(const char* url=NULL);

  /// Remove nodes and clear undo/redo stacks
  void Clear(int removeSingletons);

  /// Reset all nodes to their constructor's state
  void ResetNodes();

  /// Create node with a given class
  vtkMRMLNode* CreateNodeByClass(const char* className);

  /// \brief Register a node class to the scene so that the scene can later
  /// create the same node type from a tag or a class name.
  ///
  /// This is mainly used during scene loading.
  /// The XML element names are used to instantiate the nodes.
  ///
  /// \a node is an instance of the class to instantiate when
  /// CreateNodeByClass() is called with a corresponding className retrieved
  /// using GetClassNameByTag().
  /// \a tagName can be 0 or an XML tag a custom tagName.
  /// If \a tagName is 0 (default), the \a node GetNodeTagName() is used.
  /// Otherwise, tagName is used.
  ///
  /// The signature with tagName != 0 is useful to add support for
  /// scene backward compatibility. Calls with an obsolete tag should be
  /// wrapped with: <code>\#if MRML_SUPPORT_VERSION < 0x0X0Y0Z</code> and <code>\#endif</code>
  /// where X is the major version of Slicer scene to support, Y the minor and
  /// Z the patch version.
  ///
  /// \sa CreateNodeByClass(), GetClassNameByTag()
  void RegisterNodeClass(vtkMRMLNode* node, const char* tagName);

  /// Utility function to RegisterNodeClass(), the node tag name is used when
  /// registering the node.
  ///
  /// \sa RegisterNodeClass()
  void RegisterNodeClass(vtkMRMLNode* node);

  /// Add a path to the list.
  const char* GetClassNameByTag(const char *tagName);

  /// Add a path to the list.
  const char* GetTagByClassName(const char *className);

  /// Return collection of nodes
  vtkCollection* GetNodes();

  /// Add a node to the scene and send NodeAboutToBeAddedEvent, NodeAddedEvent
  /// and SceneModified events.
  /// A unique ID (e.g. "vtkMRMLModeNode1", "vtkMRMLScalarVolumeNode4") is
  /// generated and set to the node. If the node has no name
  /// (i.e. vtkMRMLNode::GetName() == 0), a unique name is given to the node
  /// (e.g. "Model" if it's the first model node added into the scene,
  /// "Model_1" if it's the second, etc.
  /// If node is a singleton, and a node of the same class with the
  /// same singleton tag already exists in the scene, \a nodeToAdd is NOT
  /// added but its properties are copied (c.f. vtkMRMLNode::CopyWithScene())
  /// into the already existing singleton node. That node is then returned.
  vtkMRMLNode* AddNode(vtkMRMLNode *nodeToAdd);

  /// Add a copy of a node to the scene.
  vtkMRMLNode* CopyNode(vtkMRMLNode *n);

  /// Invoke a NodeAddedEvent (used, for instnace, after adding a bunch of nodes with AddNodeNoNotify
  void NodeAdded(vtkMRMLNode *n);
  void NodeAdded() {this->NodeAdded(NULL);};

  /// Remove a path from the list.
  void RemoveNode(vtkMRMLNode *n);

  /// Determine whether a particular node is present. Returns its position
  /// in the list.
  int IsNodePresent(vtkMRMLNode *n);

  /// Initialize a traversal (not reentrant!)
  void InitTraversal();

  /// Get next node in the scene.
  vtkMRMLNode *GetNextNode();

  /// Get next node of the class in the scene.
  vtkMRMLNode *GetNextNodeByClass(const char* className);

  /// Get nodes having the specified name
  vtkCollection *GetNodesByName(const char* name);
  vtkMRMLNode *GetFirstNodeByName(const char* name);

  /// Return the first node in the scene that matches the filtering
  /// criterias if specified.
  vtkMRMLNode *GetFirstNode(const char* byName = 0, const char* byClass = 0,
                            const int* byHideFromEditors = 0);

  /// Get node given a unique ID
  vtkMRMLNode *GetNodeByID(const char* name);
  vtkMRMLNode *GetNodeByID(std::string name);

  /// Get nodes of a specified class having the specified name
  /// You are responsible for deleting the collection.
  vtkCollection *GetNodesByClassByName(const char* className, const char* name);

  /// Get number of nodes in the scene
  int GetNumberOfNodes();

  /// Get n-th node in the scene
  vtkMRMLNode* GetNthNode(int n);

  /// Get n-th node of a specified class  in the scene
  vtkMRMLNode* GetNthNodeByClass(int n, const char* className );

  /// Get number of nodes of a specified class in the scene
  int GetNumberOfNodesByClass(const char* className);

  /// Get vector of nodes of a specified class in the scene
  int GetNodesByClass(const char *className, std::vector<vtkMRMLNode *> &nodes);

  /// You are responsible for deleting the returned collection
  vtkCollection* GetNodesByClass(const char *className);

  /// Search and return the singleton of type className with a singletonTag
  /// tag. Return 0 if such node can't be found in the scene.
  vtkMRMLNode* GetSingletonNode(const char* singletonTag, const char* className);

  /// Search and return a matching singleton in the scene that the input singleton
  /// node will overwrite if it is added to the scene.
  /// There is matching if the singleton tag and className are the same,
  /// or there is an existing node with the generated unique singleton node ID.
  /// Return 0 if such node can't be found in the scene.
  vtkMRMLNode* GetSingletonNode(vtkMRMLNode* n);

  std::list<std::string> GetNodeClassesList();

  /// Get the number of registered node classes (is probably greater than the current number
  /// of nodes instantiated in the scene)
  int GetNumberOfRegisteredNodeClasses();
  /// Get the nth registered node class, returns NULL if n is out of the range 0-GetNumberOfRegisteredNodeClasses
  /// Useful for iterating through nodes to find all the possible storage nodes.
  vtkMRMLNode * GetNthRegisteredNodeClass(int n);

  /// Generate a node name that is unique in the scene.
  /// Calling this function successively with the same baseName returns a
  /// different name: e.g. baseName1, baseName2, baseName3
  std::string GenerateUniqueName(const std::string& baseName);

  /// \deprecated GetUniqueNameByString
  /// Obsolete utility function that provides an unsafe API for
  /// GenerateUniqueName()
  /// \sa GenerateUniqueName, GenerateUniqueID
  const char* GetUniqueNameByString(const char* baseName);

  /// insert a node in the scene after a specified node
  vtkMRMLNode* InsertAfterNode( vtkMRMLNode *item, vtkMRMLNode *newItem);
  /// insert a node in the scene before a specified node
  vtkMRMLNode* InsertBeforeNode( vtkMRMLNode *item, vtkMRMLNode *newItem);

  /// Ger transformation between two nodes
  int GetTransformBetweenNodes( vtkMRMLNode *node1, vtkMRMLNode *node2,
                                vtkGeneralTransform *xform );

  /// Set undo on/off
  void SetUndoOn() {UndoFlag=true;};
  void SetUndoOff() {UndoFlag=false;};
  bool GetUndoFlag() {return UndoFlag;};
  void SetUndoFlag(bool flag) {UndoFlag = flag;};

  /// undo, set the scene to previous state
  void Undo();

  /// redo, set the scene to previously undone
  void Redo();

  /// clear Undo stack, delete undo history
  void ClearUndoStack();

  /// clear Redo stack, delete redo history
  void ClearRedoStack();

  /// returns number of undo steps in the history buffer
  int GetNumberOfUndoLevels() { return (int)this->UndoStack.size();};

  /// returns number of redo steps in the history buffer
  int GetNumberOfRedoLevels() { return (int)this->RedoStack.size();};

  /// Save current state in the undo buffer
  void SaveStateForUndo();
  /// Save current state of the node in the undo buffer
  void SaveStateForUndo(vtkMRMLNode *node);
  /// Save current state of the nodes in the undo buffer
  void SaveStateForUndo(vtkCollection *nodes);
  void SaveStateForUndo(std::vector<vtkMRMLNode *> nodes);

  /// The Scene maintains a map (NodeReferences) to keep track of the relationship
  /// between node IDs and the nodes referencing those IDs.  Each
  /// node can use the call AddReferencedNodeID to tell the scene
  /// that is 'has an interest' in the given ID so that the scene
  /// can notify that node when the ID has been remapped.   It does
  /// this notification through the UpdateNodeReferences call.
  void AddReferencedNodeID(const char *id, vtkMRMLNode *refrencingNode);
  bool IsNodeReferencingNodeID(vtkMRMLNode* referencingNode, const char* id);

  /// Get the total number of node references (number of ReferencedID-ReferencingNode pairs).
  /// Only for testing and debugging.
  int GetNumberOfNodeReferences();
  /// Get the ReferencingNode component of the n-th ReferencedID-ReferencingNode pair.
  /// Only for testing and debugging.
  vtkMRMLNode* GetNthReferencingNode(int n);
  /// Get the ReferencedID component of the n-th ReferencedID-ReferencingNode pair.
  /// Only for testing and debugging.
  const char* GetNthReferencedID(int n);

  void RemoveReferencedNodeID(const char *id, vtkMRMLNode *refrencingNode);

  void RemoveNodeReferences(vtkMRMLNode *node);

  void RemoveReferencesToNode(vtkMRMLNode *node);

  /// Notify nodes about node ID changes.
  /// UpdateReferenceID is called for all the nodes that refer to a node with a changed ID.
  /// If checkNodes is not NULL, then only those nodes are notified that are part of the checkNodes collection.
  void UpdateNodeReferences(vtkCollection* checkNodes=NULL);

  void CopyNodeReferences(vtkMRMLScene *scene);

  void CopyNodeChangedIDs(vtkMRMLScene *scene);

  /// Change node IDs based on the ReferencedIDChanges list
  void UpdateNodeChangedIDs();

  void RemoveUnusedNodeReferences();

  void AddReservedID(const char *id);

  void RemoveReservedIDs();

  /// get the new id of the node that is different from one in the mrml file
  /// or NULL if id has not changed
  const char* GetChangedID(const char* id);

  /// Return collection of all nodes referenced directly or indirectly by
  /// \a node.
  /// The node itself is in first place in the collection. No doublon in the
  /// list.
  /// For a node to be referenced by another node, the latter must call
  /// AddReferencedNodeID().
  /// Note that you are responsible for deleting the returned collection.
  /// \sa GetReferencedSubScene()
  vtkCollection* GetReferencedNodes(vtkMRMLNode *node);

  /// Get vector of nodes containing references to an input node
  void GetReferencingNodes(vtkMRMLNode* referencedNode, std::vector<vtkMRMLNode *> &referencingNodes);

  /// Get a sub-scene containing all nodes directly or indirectly referenced by
  /// the input node.
  /// GetReferencedNodes() is internally called.
  /// \sa AddReferencedNodeID(), GetReferencedNodes()
  void GetReferencedSubScene(vtkMRMLNode *node, vtkMRMLScene* newScene);

  int IsFilePathRelative(const char * filepath);

  vtkSetMacro(ErrorCode,unsigned long);
  vtkGetMacro(ErrorCode,unsigned long);

  /// This property controls whether Import() loads a scene from an XML string
  /// or from an XML file
  /// If true, contents is loaded from SceneXMLString.
  /// If false, contents is loaded from the \a URL file.
  /// \sa Import(), SetSceneXMLString(), SetURL(), SetSaveToXMLString()
  vtkSetMacro(LoadFromXMLString,int);
  vtkGetMacro(LoadFromXMLString,int);

  /// This property controls whether Commit() should save the scene as an XML
  /// string or an XML file.
  /// If true, the contents generated by Commit() will be in SceneXMLString.
  /// If false, the contents will be saved into the \a URL file.
  /// \sa Commit(), GetSceneXMLString(), SetURL(), SetLoadFromXMLString()
  vtkSetMacro(SaveToXMLString,int);
  vtkGetMacro(SaveToXMLString,int);

  vtkSetMacro(ReadDataOnLoad,int);
  vtkGetMacro(ReadDataOnLoad,int);

  void SetErrorMessage(const std::string &error);
  std::string GetErrorMessage();

  /// Set the XML string to read from by Import() if GetLoadFromXMLString() is
  /// true.
  /// \sa Import(), GetLoadFromXMLString(), GetSceneXMLString()
  void SetSceneXMLString(const std::string &xmlString);

  /// Returns the saved scene with an XML format if  SaveToXMLString is true
  /// and Commit() was called prior.
  /// \sa Commit(), SetSaveToXMLString()
  const std::string& GetSceneXMLString();

  void SetErrorMessage(const char * message);
  const char *GetErrorMessagePointer();

  vtkGetObjectMacro ( CacheManager, vtkCacheManager );
  virtual void SetCacheManager(vtkCacheManager* );
  vtkGetObjectMacro ( DataIOManager, vtkDataIOManager );
  virtual void SetDataIOManager(vtkDataIOManager* );
  vtkGetObjectMacro ( URIHandlerCollection, vtkCollection );
  virtual void SetURIHandlerCollection(vtkCollection* );
  vtkGetObjectMacro ( UserTagTable, vtkTagTable);
  virtual void SetUserTagTable(vtkTagTable* );

  /// find a URI handler in the collection that can work on the passed URI
  /// returns NULL on failure
  vtkURIHandler *FindURIHandler(const char *URI);
  /// Returns a URIhandler of a specific type if its name is known.
  vtkURIHandler *FindURIHandlerByName ( const char *name );
  /// Add a uri handler to the collection.
  void AddURIHandler(vtkURIHandler *handler);

  /// The state of the scene reflects what the scene is doing.
  /// The scene is in \a BatchProcessState when nodes are either inserted or
  /// removed contiguously. The methods Connect, Import and Clear
  /// automatically set the scene in specific states. It is possible to
  /// manually set the scene in a specific state using \a StartState and
  /// \a EndState.
  /// When the scene is in \a BatchProcessState mode, observers can ignore
  /// the events NodeAddedEvent and NodeRemovedEvent to
  /// only synchronize with the scene when the scene is no longer in
  ///  \a BatchProcessState (EndBatchProcessEvent being fired).
  ///
  /// The call <code>scene->Connect("myScene.mrml");</code> that closes and
  /// import a scene will fire the events:
  /// vtkMRMLScene::StartBatchProcessEvent,
  /// vtkMRMLScene::StartCloseEvent,
  ///
  /// vtkMRMLScene::NodeAboutToBeRemovedEvent,
  /// vtkMRMLScene::NodeRemovedEvent,
  /// vtkMRMLScene::ProgressCloseEvent,
  /// vtkMRMLScene::ProgressBatchProcessEvent,
  /// ...
  /// vtkMRMLScene::EndCloseEvent,
  /// vtkMRMLScene::StartImportEvent,
  ///
  /// vtkMRMLScene::NodeAboutToBeAddedEvent,
  /// vtkMRMLScene::NodeAddedEvent,
  /// vtkMRMLScene::ProgressImportEvent,
  /// vtkMRMLScene::ProgressBatchProcessEvent,
  /// ...
  /// vtkMRMLScene::EndImportEvent,
  /// vtkMRMLScene::EndBatchProcessEvent
  enum StateType
    {
    BatchProcessState = 0x0001,
    CloseState = 0x0002 | BatchProcessState,
    ImportState = 0x0004 | BatchProcessState,
    RestoreState = 0x0008 | BatchProcessState,
    SaveState = 0x0010
    };

  /// Returns the current state of the scene.
  /// It is a combination of all current states.
  /// Returns 0 if the scene has no current state flag.
  /// \sa IsBatchProcessing, IsClosing, IsImporting, IsRestoring
  /// \sa StartState, EndState
  int GetStates()const;

  /// Return true if the scene is in BatchProcess state, false otherwise
  inline bool IsBatchProcessing()const;
  /// Return true if the scene is in Close state, false otherwise
  inline bool IsClosing()const;
  /// Return true if the scene is in Import state, false otherwise
  inline bool IsImporting()const;
  /// Return true if the scene is in Restore state, false otherwise
  inline bool IsRestoring()const;

  /// Flag the scene as being in a \a state mode.
  /// A matching EndState(\a state) must be called later.
  /// Nested states are supported as long as they all have their own EndState
  /// calls.
  /// StartState() fires the \a state start event if it isn't already in that
  /// state, e.g. StartImportEvent if \a state is ImportState.
  /// If the state is BatchProcessState, CloseState, ImportState or
  /// RestoreState and if the scene is not already in a BatchProcessState
  /// state, it also fires the event StartBatchProcessEvent.
  /// Example:
  /// \code
  /// scene->StartState(vtkMRMLScene::ImportState);
  /// // fires: StartBatchProcessEvent, StartImportEvent
  /// scene->StartState(vtkMRMLScene::ImportState);
  /// // doesn't fire anything
  /// scene->StartState(vtkMRMLScene::RestoreState);
  /// // fires: StartRestoreEvent
  /// scene->EndState(vtkMRMLScene::RestoreState);
  /// // fires: EndRestoreEvent
  /// scene->EndState(vtkMRMLScene::ImportState);
  /// // doesn't fire anything
  /// scene->EndState(vtkMRMLScene::ImportState);
  /// // fires: EndImportEvent, StartBatchProcessEvent
  /// \endcode
  /// StartState internally pushes the state into a stack.
  /// \sa EndState, GetStates
  void StartState(unsigned long state, int anticipatedMaxProgress = 0);

  /// Unflag the scene as being in a \a state mode.
  /// EndState() fires the state end event if the scene is no longer
  /// in the \a state mode.
  /// A matching StartState(\a state) must be called prior.
  /// e.g. EndImportEvent if state is ImportState.
  /// EndState internally pops the state out of the stack
  void EndState(unsigned long state);

  /// TODO: Report progress of the current state.
  void ProgressState(unsigned long state, int progress = 0);

  enum SceneEventType
    {
    NodeAboutToBeAddedEvent = 0x2000,
    NodeAddedEvent,
    NodeAboutToBeRemovedEvent,
    NodeRemovedEvent,

    NewSceneEvent = 66030,
    MetadataAddedEvent = 66032, // ### Slicer 4.5: Simplify - Do not explicitly set for backward compat. See issue #3472
    ImportProgressFeedbackEvent,
    SaveProgressFeedbackEvent,

    /// \internal
    /// not to be used directly
    /// \endinternal
    StateEvent = 0x2000, ///< 1024 (decimal)
    StartEvent = 0x0100,
    EndEvent = 0x0200,
    ProgressEvent = 0x0400,

    StartBatchProcessEvent = StateEvent | StartEvent | BatchProcessState,
    EndBatchProcessEvent = StateEvent | EndEvent | BatchProcessState,
    ProgressBatchProcessEvent = StateEvent | ProgressEvent | BatchProcessState,

    StartCloseEvent = StateEvent | StartEvent | CloseState,
    EndCloseEvent = StateEvent | EndEvent | CloseState,
    ProgressCloseEvent = StateEvent | ProgressEvent | CloseState,

    StartImportEvent = StateEvent | StartEvent | ImportState,
    EndImportEvent = StateEvent | EndEvent | ImportState,
    ProgressImportEvent = StateEvent | EndEvent | ImportState,

    StartRestoreEvent = StateEvent | StartEvent | RestoreState,
    EndRestoreEvent = StateEvent | EndEvent | RestoreState,
    ProgressRestoreEvent = StateEvent | ProgressEvent | RestoreState,

    StartSaveEvent = StateEvent | StartEvent | SaveState,
    EndSaveEvent = StateEvent | EndEvent | SaveState,
    ProgressSaveEvent = StateEvent | ProgressEvent | SaveState,
    };

  /// the version of the last loaded scene file
  vtkGetStringMacro(LastLoadedVersion);
  vtkSetStringMacro(LastLoadedVersion);

  /// the current software version
  vtkGetStringMacro(Version);
  vtkSetStringMacro(Version);

  /// Copies all registered nodes into the parameter scene
  void CopyRegisteredNodesToScene(vtkMRMLScene *scene);

  /// Copies all singleton nodes into the parameter scene
  void CopySingletonNodesToScene(vtkMRMLScene *scene);

  /// Returns true if the scene has been "significantly" modified since the
  /// last time it was read or written
  bool GetModifiedSinceRead();

  /// Search the scene for storable nodes that are "ModifiedSinceRead".
  /// Returns true if at least 1 matching node is found.
  /// If \a modifiedStorableNodes is passed the modified nodes are appended.
  /// Note that the nodes see their reference count being incremented while
  /// being in the list. Don't forget to clear it as soon as you don't need it.
  bool GetStorableNodesModifiedSinceRead(vtkCollection* modifiedStorableNodes = 0);
protected:

  typedef std::map< std::string, std::set<std::string> > NodeReferencesType;

  vtkMRMLScene();
  virtual ~vtkMRMLScene();

  void PushIntoUndoStack();
  void PushIntoRedoStack();

  void CopyNodeInUndoStack(vtkMRMLNode *node);
  void CopyNodeInRedoStack(vtkMRMLNode *node);

  /// Add a node to the scene without invoking a NodeAddedEvent event
  /// Use with extreme caution as it might unsynchronize observer.
  vtkMRMLNode* AddNodeNoNotify(vtkMRMLNode *n);

  void AddReferencedNodes(vtkMRMLNode *node, vtkCollection *refNodes);

  /// Handle vtkMRMLScene::DeleteEvent: clear the scene
  static void SceneCallback( vtkObject *caller, unsigned long eid,
                             void *clientData, void *callData );

  std::string GenerateUniqueID(vtkMRMLNode* node);
  std::string GenerateUniqueID(const std::string& baseID);
  int GetUniqueIDIndex(const std::string& baseID);
  std::string BuildID(const std::string& baseID, int idIndex)const;

  /// Return a unique name for a MRML node. It uses the node tag as the base.
  std::string GenerateUniqueName(vtkMRMLNode* node);

  /// Returns a unique index for a given baseName.
  /// Calling this function succesively with the same baseName returns an
  /// incremented index: 1, 2, 3...
  /// \sa GenerateUniqueName
  int GetUniqueNameIndex(const std::string& baseName);
  /// Combine a basename and an index to produce a full name.
  std::string BuildName(const std::string& baseName, int nameIndex)const;

  /// Syncronize NodeIDs map used to speedup GetByID() method with the Nodes collection
  void UpdateNodeIDs();

  /// Add node to NodeIDs map used to speedup GetByID() method
  void AddNodeID(vtkMRMLNode *node);

  /// Remove node from NodeIDs map used to speedup GetByID() method
  void RemoveNodeID(char *nodeID);

  /// Clear NodeIDs map used to speedup GetByID() method
  void ClearNodeIDs();

  /// Get a NodeReferences iterator for a node reference
  NodeReferencesType::iterator FindNodeReference(const char* referencedId, vtkMRMLNode* referencingNode);

  vtkCollection*  Nodes;
  unsigned long   SceneModifiedTime;

  /// data i/o handling members
  vtkCacheManager *  CacheManager;
  vtkDataIOManager * DataIOManager;
  vtkCollection *    URIHandlerCollection;
  vtkTagTable *      UserTagTable;

  std::vector<unsigned long> States;

  int  UndoStackSize;
  bool UndoFlag;
  bool InUndo;

  std::list< vtkCollection* >  UndoStack;
  std::list< vtkCollection* >  RedoStack;

  std::string                 URL;
  std::string                 RootDirectory;

  std::map<std::string, int> UniqueIDs;
  std::map<std::string, int> UniqueNames;
  std::set<std::string>   ReservedIDs;

  std::vector< vtkMRMLNode* > RegisteredNodeClasses;
  std::vector< std::string >  RegisteredNodeTags;

  NodeReferencesType NodeReferences; // ReferencedIDs (string), ReferencingNodes (node pointer)
  std::map< std::string, std::string > ReferencedIDChanges;
  std::map< std::string, vtkSmartPointer<vtkMRMLNode> > NodeIDs;

  std::string ErrorMessage;

  std::string SceneXMLString;

  int LoadFromXMLString;

  int SaveToXMLString;

  int ReadDataOnLoad;

  unsigned long NodeIDsMTime;

  void RemoveAllNodes(bool removeSingletons);

  char * Version;
  char * LastLoadedVersion;

  vtkCallbackCommand *DeleteEventCallback;

private:

  vtkMRMLScene(const vtkMRMLScene&);   // Not implemented
  void operator=(const vtkMRMLScene&); // Not implemented

  /// Returns nonzero on success
  int LoadIntoScene(vtkCollection* scene);

  unsigned long ErrorCode;

  /// Time when the scene was last read or written.
  vtkTimeStamp StoredTime;
};

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsBatchProcessing()const
{
  return (this->GetStates() & vtkMRMLScene::BatchProcessState)
         == vtkMRMLScene::BatchProcessState;
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsClosing()const
{
  return (this->GetStates() & vtkMRMLScene::CloseState)
         == vtkMRMLScene::CloseState;
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsImporting()const
{
  return (this->GetStates() & vtkMRMLScene::ImportState)
         == vtkMRMLScene::ImportState;
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsRestoring()const
{
  return (this->GetStates() & vtkMRMLScene::RestoreState)
         == vtkMRMLScene::RestoreState;
}

#endif
