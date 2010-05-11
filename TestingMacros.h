/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __TestingMacros_h
#define __TestingMacros_h

#include "vtkDebugLeaks.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"

#define DEBUG_LEAKS_ENABLE_EXIT_ERROR() \
  vtkDebugLeaks::SetExitError(true);

#define EXERCISE_BASIC_OBJECT_METHODS( object ) \
    { \
    if ( object == NULL ) \
      { \
      std::cerr << "EXERCISE_BASIC_OBJECT_METHODS( with NULL object )" << std::endl;  \
      return EXIT_FAILURE;  \
      } \
    object->Print( std::cout );  \
    std::cout << "Name of Class = " << object->GetClassName() << std::endl; \
    std::cout << "Name of Superclass = " << object->Superclass::GetClassName() << std::endl; \
    }

#define TRY_EXPECT_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    std::cerr << "Failed to catch expected exception" << std::endl;  \
    return EXIT_FAILURE;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cout << "Caught expected exception" << std::endl;  \
    std::cout << excp << std::endl; \
    }  


#define TRY_EXPECT_NO_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cerr << excp << std::endl; \
    return EXIT_FAILURE;  \
    }  


#define TEST_ITK_SET_GET( variable, command ) \
  if( variable.GetPointer() != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable.GetPointer() << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }


#define TEST_ITK_SET_GET_VALUE( variable, command ) \
  if( variable != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_BOOLEAN( object, variable ) \
  object->Set##variable( false ); \
  object->Set##variable( true ); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->Set##variable( false ); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##On(); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in On/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##Off(); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Off/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_INT( object, variable )    \
  {                                             \
    int pos = (int)(vtkMath::Random() * 100.0); \
    object->Set##variable( pos );               \
    int neg = (int)(vtkMath::Random() * -100.0);    \
    object->Set##variable( neg );                   \
    if( object->Get##variable() != neg )            \
      {                                                    \
      std::cerr << "Error in Set/Get"#variable << std::endl;    \
      return EXIT_FAILURE;                                      \
      }                                                         \
  }

#define TEST_SET_GET_DOUBLE( object, variable ) \
  {                                             \
    double pos = vtkMath::Random() * 100.0;     \
    object->Set##variable( pos );               \
    double neg = vtkMath::Random() * -100.0;    \
    object->Set##variable( neg );               \
    if( object->Get##variable() != neg )        \
      {                                         \
      std::cerr << "Error in Set/Get"#variable << std::endl;    \
      return EXIT_FAILURE;                                      \
      }                                                         \
  }

#define TEST_SET_GET_VECTOR3_DOUBLE( object, variable ) \
  {                                                     \
    double x = vtkMath::Random();                       \
    double y = vtkMath::Random();                       \
    double z = vtkMath::Random();                       \
    object->Set##variable( x, y, z );                   \
    double *val = object->Get##variable();                       \
    if( val == NULL || val[0] != x || val[1] != y || val[2] != z )  \
      {                                                             \
      std::cerr << "Error in Set/Get"#variable << std::endl;        \
      return EXIT_FAILURE;                                          \
      }                                                             \
  }

#define TEST_SET_GET_STRING( object, variable ) \
  { \
  const char * originalStringPointer = object->Get##variable(); \
  std::string originalString; \
  if( originalStringPointer != NULL ) \
    { \
    originalString = originalStringPointer; \
    } \
  std::string string1 = "testingIsGood"; \
  object->Set##variable( string1.c_str() ); \
  if( object->Get##variable() != string1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  std::string string2 = "moreTestingIsBetter"; \
  object->Set##variable( string2.c_str() ); \
  if( object->Get##variable() != string2 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  if( originalStringPointer != NULL ) \
    { \
    object->Set##variable( originalString.c_str() );  \
    } \
  else \
    { \
    object->Set##variable( NULL ); \
    } \
  }

#define EXERCISE_BASIC_MRML_METHODS( className, node ) \
  {\
    vtkSmartPointer < className > node1 = vtkSmartPointer < className >::New(); \
    node1->Copy(node);                                                  \
    node->Reset();                                                      \
    int mod = node->StartModify();                                      \
    std::string nodeTagName = node->GetNodeTagName();                   \
    std::cout << "Node Tag Name = " << nodeTagName << std::endl;        \
    std::string attributeName = std::string("attName");                 \
    std::string attributeValue = std::string("attValue");               \
    node->SetAttribute( attributeName.c_str(), attributeValue.c_str() ); \
    std::string attributeValue2 = node->GetAttribute( attributeName.c_str() ); \
    if( attributeValue != attributeValue2 )                             \
      {                                                                 \
      std::cerr << "Error in Set/GetAttribute() " << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->EndModify(mod);                                               \
    TEST_SET_GET_BOOLEAN( node, HideFromEditors );                      \
    TEST_SET_GET_BOOLEAN( node, Selectable );                           \
    TEST_SET_GET_STRING( node, Description );                           \
    TEST_SET_GET_STRING( node, SceneRootDir );                          \
    TEST_SET_GET_STRING( node, Name );                                  \
    node->UpdateID("newID");                                            \
    if (strcmp(node->GetID(), "newID") != 0)                            \
      {                                                                 \
      std::cerr << "Error in UpdateID()" << std::endl;                  \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->CopyID(node1);                                                \
    if (node->GetID() != node1->GetID())                                \
      {                                                                 \
      std::cerr << "Error in CopyID()" << std::endl;                    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_STRING( node, SingletonTag );                          \
    TEST_SET_GET_BOOLEAN( node, ModifiedSinceRead );                    \
    TEST_SET_GET_BOOLEAN( node, SaveWithScene );                        \
    TEST_SET_GET_BOOLEAN( node, AddToScene );                           \
    TEST_SET_GET_BOOLEAN( node, DisableModifiedEvent);                  \
    TEST_SET_GET_BOOLEAN( node, Selected );                             \
    node->Modified();                                                   \
    node->InvokePendingModifiedEvent();                                 \
    node1->SetName("copywithsinglemodified");                           \
    node->CopyWithSingleModifiedEvent(node1);                           \
    node1->SetName("copywithoutmodified");                              \
    node->CopyWithoutModifiedEvent(node1);                              \
    node1->SetName("copywithscenewithsinglemodified");                  \
    node->CopyWithSceneWithSingleModifiedEvent(node1);                  \
    node1->SetName("copywithscenewithoutmodified");                     \
    node->CopyWithSceneWithoutModifiedEvent(node1);                     \
    vtkMRMLScene * scene = node->GetScene();                            \
                                                                        \
    if( scene != NULL )                                                 \
      {                                                                 \
      std::cerr << "Error in GetScene() " << std::endl;                 \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    node->UpdateReferences();                                           \
    node->UpdateReferenceID("oldID", "newID");                          \
                                                                        \
    std::string stringToEncode = "Thou Shall Test !";                   \
    std::string stringURLEncoded = node1->URLEncodeString( stringToEncode.c_str() ); \
    std::string stringDecoded = node1->URLDecodeString( stringURLEncoded.c_str() ); \
    if( stringDecoded != stringToEncode )                               \
      {                                                                 \
      std::cerr << "Error in URLEncodeString/URLDecodeString() " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    const char *atts[] = {"id", "vtkMRMLMeasurementsNodeTest1", "name", "MyName", "description", "Testing a mrml node", "hideFromEditors", "false", "selectable", "true", "selected", "true", NULL}; \
    node->ReadXMLAttributes(atts);                                      \
    if (strcmp(node->GetID(), "vtkMRMLMeasurementsNodeTest1") != 0)     \
      {                                                                 \
      std::cerr << "Error in ReadXMLAttributes! id should be vtkMRMLMeasurementsNodeTest1, but is " << node->GetID() << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->WriteXML(std::cout, 0);                                       \
    std::cout << std::endl;                                             \
  }

#define EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( node ) \
  {\
    vtkMRMLTransformNode *tnode2 = node->GetParentTransformNode();\
    if (tnode2 != NULL)\
      {\
      std::cerr << "ERROR: parent transform node is not null" << std::endl;\
      return EXIT_FAILURE; \
      }\
    node1->SetAndObserveTransformNodeID(NULL);\
    const char *node_tid = node1->GetTransformNodeID();\
    if (node_tid != NULL)                              \
      {\
      std::cerr << "ERROR: with observing transform node id" << std::endl;\
      return EXIT_FAILURE; \
      }\
    bool canApplyNonLinear = node->CanApplyNonLinearTransforms();\
    std::cout << "Node can apply non linear transforms? " << (canApplyNonLinear == true ? "yes" : "no") << std::endl;\
  }

#define EXERCISE_BASIC_STORABLE_MRML_METHODS( node ) \
  {                                                  \
    if (node->GetNumberOfStorageNodes() != 0)        \
      {                                              \
      std::cerr << "Error in getting number of storage nodes." << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveStorageNodeID("noid");                           \
    node->AddAndObserveStorageNodeID("badid");                          \
    node->SetAndObserveNthStorageNodeID(2, "nothing");                  \
    node->SetSlicerDataType("testing");                                 \
    if (strcmp(node->GetSlicerDataType(), "testing") != 0)              \
      {                                                                 \
      std::cerr << "ERROR set/get slicer data type" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    const char *snodeid = node->GetNthStorageNodeID(0);                 \
    if (strcmp(snodeid, "noid") != 0)                                        \
      {                                                                 \
      std::cerr << "ERROR getting 0th storage node id, instead of noid got " << (snodeid  == NULL ? "null" : snodeid) << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkMRMLStorageNode *snode = node->GetNthStorageNode(0);             \
    if (snode != NULL)                                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th storage node" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    snode = node->CreateDefaultStorageNode();                           \
    if (snode == NULL)                                                  \
      {                                                                 \
      std::cerr << "ERROR creating and getting default storage node" << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    snode->Delete();                                                    \
    vtkTagTable *tagtable = node->GetUserTagTable();                    \
    if (tagtable == NULL)                                               \
      {                                                                 \
      std::cerr << "ERROR getting tag table" << std::endl;              \
      return EXIT_FAILURE;                                              \
      }                                                                 \
  }

#define EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( node ) \
  {                                                  \
    if (node->GetNumberOfDisplayNodes() != 0)        \
      {                                              \
      std::cerr << "Error in getting number of display nodes." << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveDisplayNodeID("noid");                           \
    node->AddAndObserveDisplayNodeID("badid");                          \
    node->SetAndObserveNthDisplayNodeID(2, "nothing");                  \
    const char *dnodeid = node->GetNthDisplayNodeID(0);                 \
    if (strcmp(dnodeid, "noid") != 0)                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th display node id, instead of noid got " << (dnodeid  == NULL ? "null" : dnodeid) << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkMRMLDisplayNode *dnode = node->GetNthDisplayNode(0);             \
    if (dnode != NULL)                                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th display node" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkPolyData *pdata = node1->GetPolyData();                          \
    if (pdata != NULL)                                                  \
      {                                                                 \
      std::cerr << "ERROR getting null polydata" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    pdata = vtkPolyData::New();                                         \
    node1->SetAndObservePolyData(pdata);                                \
    if (node1->GetPolyData() != pdata)                                  \
      {                                                                 \
      std::cerr << "ERROR getting polydata" << std::endl;               \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    pdata->Delete();                                                    \
  }

#define EXERCISE_BASIC_DISPLAY_MRML_METHODS( node ) \
  {                                                     \
    if (node->GetPolyData() != NULL)                    \
      {                                                 \
      std::cerr << "Error getting null polydata" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    if (node->GetImageData() != NULL)                    \
      {                                                 \
      std::cerr << "Error getting null image" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    vtkMRMLDisplayableNode *dnode = node->GetDisplayableNode(); \
    if (dnode != NULL)                                          \
      {                                                 \
      std::cerr << "Error getting null displayable node" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    node->UpdatePolyDataPipeline();                             \
    node->UpdateImageDataPipeline();                            \
    node->SetAndObserveTextureImageData(NULL);                  \
    if (node->GetTextureImageData() != NULL)                    \
      {                                                                 \
      std::cerr << "Error getting null texture image data " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveColorNodeID(NULL);                               \
    if (node->GetColorNodeID() != NULL)                                 \
      {                                                                 \
      std::cerr << "Error getting null color node id " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (node->GetColorNode() != NULL)                                   \
      {                                                                 \
      std::cerr << "Error getting null color node " << std::endl;    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetActiveScalarName("testingScalar");                         \
    if (strcmp(node->GetActiveScalarName(), "testingScalar") != 0)      \
      {                                                                 \
      std::cerr << "Error getting active scalar name" << std::endl;      \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_VECTOR3_DOUBLE(node, Color);                            \
    TEST_SET_GET_VECTOR3_DOUBLE(node, SelectedColor);                   \
    TEST_SET_GET_DOUBLE(node, SelectedAmbient);                         \
    TEST_SET_GET_DOUBLE(node, SelectedSpecular);                        \
    TEST_SET_GET_DOUBLE(node, Opacity);                                 \
    TEST_SET_GET_DOUBLE(node, Ambient);                                 \
    TEST_SET_GET_DOUBLE(node, Diffuse);                                 \
    TEST_SET_GET_DOUBLE(node, Specular);                                \
    TEST_SET_GET_DOUBLE(node, Power);                                   \
    TEST_SET_GET_BOOLEAN(node, Visibility);                             \
    TEST_SET_GET_BOOLEAN(node, Clipping);                               \
    TEST_SET_GET_BOOLEAN(node, SliceIntersectionVisibility);            \
    TEST_SET_GET_BOOLEAN(node, BackfaceCulling);                        \
    TEST_SET_GET_BOOLEAN(node, ScalarVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, VectorVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, TensorVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, AutoScalarRange);                        \
    double range[2] = {-10, 10};                                        \
    node->SetScalarRange(range);                                        \
    double *getrange = node->GetScalarRange();                          \
    if (getrange == NULL || getrange[0] != range[0] || getrange[1] != range[1]) \
      {                                                                 \
      std::cerr << "ERROR getting range" << std::endl;                  \
      return EXIT_FAILURE;                                              \
      }                                                                 \
  }

#include <vtkStringArray.h>

#define EXERCISE_BASIC_STORAGE_MRML_METHODS( node ) \
  {                                                 \
    node->ReadData(NULL);                           \
    node->WriteData(NULL);                          \
    node->StageReadData(NULL);                      \
    node->StageWriteData(NULL);                     \
    TEST_SET_GET_STRING(node, FileName);            \
    const char *f0 = node->GetNthFileName(0);       \
    std::cout << "Filename 0 = " << (f0 == NULL ? "NULL" : f0) << std::endl; \
    TEST_SET_GET_BOOLEAN(node, UseCompression);         \
    TEST_SET_GET_STRING(node, URI);                     \
    vtkURIHandler *handler = vtkURIHandler::New();      \
    node->SetURIHandler(NULL);                          \
    if (node->GetURIHandler() != NULL)                  \
      {                                                 \
      std::cerr << "ERROR getting null uri handler" << std::endl;   \
      return EXIT_FAILURE;                                          \
      }                                                             \
    node->SetURIHandler(handler);                       \
    if (node->GetURIHandler() == NULL)                  \
      {                                                 \
      std::cerr << "ERROR getting not null uri handler" << std::endl;   \
      return EXIT_FAILURE;                                          \
      }                                                             \
    node->SetURIHandler(NULL);                                      \
    handler->Delete();                                              \
    TEST_SET_GET_INT(node, ReadState);                              \
    const char *rstate = node->GetReadStateAsString();              \
    std::cout << "Read state, after int test = " << rstate << std::endl; \
    node->SetReadStatePending();                                    \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Pending = " << rstate << std::endl;   \
    node->SetReadStateIdle();                                       \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Idle = " << rstate << std::endl;      \
    node->SetReadStateScheduled();                                  \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Scheduled = " << rstate << std::endl; \
    node->SetReadStateTransferring();                               \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Transferring = " << rstate << std::endl;   \
    node->SetReadStateTransferDone();                               \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, TransfrerDone = " << rstate << std::endl;   \
    node->SetReadStateCancelled();                                  \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Cancelled = " << rstate << std::endl; \
                                                                    \
    TEST_SET_GET_INT(node, WriteState);                             \
    const char *wstate = node->GetWriteStateAsString();              \
    std::cout << "Write state, after int test = " << wstate << std::endl; \
    node->SetWriteStatePending();                                    \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Pending = " << wstate << std::endl;   \
    node->SetWriteStateIdle();                                       \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Idle = " << wstate << std::endl;      \
    node->SetWriteStateScheduled();                                  \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Scheduled = " << wstate << std::endl; \
    node->SetWriteStateTransferring();                               \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Transferring = " << wstate << std::endl;   \
    node->SetWriteStateTransferDone();                               \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, TransfrerDone = " << wstate << std::endl;   \
    node->SetWriteStateCancelled();                                  \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Cancelled = " << wstate << std::endl; \
                                                                     \
    std::string fullName = node->GetFullNameFromFileName();          \
    std::cout << "fullName = " << fullName.c_str() << std::endl; \
    std::string fullName0 = node->GetFullNameFromNthFileName(0);     \
    std::cout << "fullName0 = " << fullName0.c_str() << std::endl; \
                                                                        \
    vtkStringArray *types = node->GetSupportedWriteFileTypes();         \
    std::cout << "Supported write types:" << std::endl;                 \
    types->Print(std::cout);                                            \
    int sup = node->SupportedFileType(NULL);                            \
    std::cout << "Filename or uri supported? " << sup << std::endl;     \
    sup = node->SupportedFileType("testing.vtk");                       \
    std::cout << ".vtk supported?  " << sup << std::endl;     \
    sup = node->SupportedFileType("testing.nrrd");            \
    std::cout << ".nrrd supported?  " << sup << std::endl;     \
                                                               \
    TEST_SET_GET_STRING(node, WriteFileFormat);                \
    node->AddFileName("testing.txt");                          \
    std::cout << "Number of file names = " << node->GetNumberOfFileNames() << std::endl; \
    int check = node->FileNameIsInList("testing.txt");\
    if (check != 1)                                                     \
      {                                                                 \
      std::cerr << "ERROR: file name not in list!" << std::endl;        \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetNthFileName(0, "moretesting.txt");                       \
    node->ResetNthFileName(100, "notinlist.txt");                       \
    node->ResetNthFileName(0, NULL);                                    \
    check = node->FileNameIsInList("notinlist");                        \
    if (check != 0)                                                     \
      {                                                                 \
      std::cerr << "ERROR: bad file is in list!" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetFileNameList();                                          \
    if (node->GetNumberOfFileNames() != 0)                              \
      {                                                                 \
      std::cerr << "ERROR: " << node->GetNumberOfFileNames() << " files left in list after reset!" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    node->ResetURIList();                                                            \
    std::cout << "Number of uri's after resetting list = " << node->GetNumberOfURIs() << std::endl; \
    node->AddURI("http://www.nowhere.com/filename.txt");                \
    if ( node->GetNumberOfURIs()  != 1)                                 \
      {                                                                 \
      std::cerr << "Error adding one uri, number of uris is incorrect: " <<  node->GetNumberOfURIs()<< std::endl;                \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    const char *uri = node->GetNthURI(0);                               \
    if (uri == NULL || strcmp(uri, "http://www.nowhere.com/filename.txt") != 0)        \
      {                                                                 \
      std::cerr << "0th URI " << uri << " is incorrect." << std::endl;                \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetNthURI(0, "http://www.nowhere.com/newfilename.txt");     \
    node->ResetNthURI(100, "ftp://not.in.list");                        \
    node->ResetNthURI(100, NULL);                                       \
    const char *dataDirName = "/testing/a/directory";                   \
    node->SetDataDirectory(dataDirName);                                \
    node->SetFileName("/tmp/file.txt");                                 \
    node->SetDataDirectory(dataDirName);                                \
    const char *uriPrefix = "http://www.somewhere.com/";                \
    node->SetURIPrefix(uriPrefix);                                      \
                                                                        \
    const char *defaultExt = node->GetDefaultWriteFileExtension();      \
    std::cout << "Default write extension = " << (defaultExt == NULL ? "null" : defaultExt) << std::endl; \
                                                                        \
    std::cout << "Is null file path relative? " << node->IsFilePathRelative(NULL) << std::endl; \
    std::cout << "Is absolute file path relative? " << node->IsFilePathRelative("/spl/tmp/file.txt") << std::endl; \
    std::cout << "Is relative file path relative? " << node->IsFilePathRelative("tmp/file.txt") << std::endl; \
  }

#endif
