/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/

// VolumeRendering includes
#include "vtkMRMLShaderPropertyNode.h"
#include "vtkMRMLShaderPropertyStorageNode.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkShaderProperty.h>
#include <vtkStringArray.h>
#include <vtkUniforms.h>

// Json includes
#include <json/json.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLShaderPropertyStorageNode);

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyStorageNode::vtkMRMLShaderPropertyStorageNode()
{
  this->DefaultWriteFileExtension = "sp";
}

//----------------------------------------------------------------------------
vtkMRMLShaderPropertyStorageNode::~vtkMRMLShaderPropertyStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLShaderPropertyStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLShaderPropertyNode");
}

namespace
{

//----------------------------------------------------------------------------
void DecodeValue(const Json::Value & json, int & value)
{
  value = json.asInt();
}

//----------------------------------------------------------------------------
void DecodeValue(const Json::Value & json, float & value)
{
  value = json.asFloat();
}

//----------------------------------------------------------------------------
template< typename ScalarType >
void DecodeTuple(const Json::Value & json, std::vector<ScalarType> & tuple)
{
  tuple.resize(json.size());
  for(Json::ArrayIndex index = 0; index < json.size(); ++index)
    {
    ScalarType value;
    DecodeValue(json[index], value);
    tuple[index] = value;
    }
}

//----------------------------------------------------------------------------
template< typename ScalarType >
void DecodeMatrix(const Json::Value & json, std::vector<ScalarType> & values)
{
  Json::ArrayIndex nbRows = json.size();
  for(Json::ArrayIndex rowIndex = 0; rowIndex < nbRows; ++rowIndex)
    {
    const Json::Value & row = json[rowIndex];

    Json::ArrayIndex nbCols = row.size();
    for(Json::ArrayIndex columnIndex = 0; columnIndex < nbCols; ++columnIndex)
      {
      ScalarType value;
      DecodeValue(row[columnIndex], value);
      values.push_back(value);
      }
    }
}

//----------------------------------------------------------------------------
template< typename ScalarType >
void DecodeValues(const Json::Value & json, vtkUniforms::TupleType tupleType, int vtkNotUsed(nbComponents), int nbTuples, std::vector<ScalarType> & values)
{
  if(nbTuples == 1)
    {
    if(tupleType == vtkUniforms::TupleTypeScalar)
      {
      ScalarType value;
      DecodeValue(json["value"], value);
      values.resize(1, value);
      }
    else if(tupleType == vtkUniforms::TupleTypeVector)
      {
      DecodeTuple(json["value"], values);
      }
    else if(tupleType == vtkUniforms::TupleTypeMatrix)
      {
      DecodeMatrix(json["value"], values);
      }
    }
  else
    {
    if(tupleType == vtkUniforms::TupleTypeScalar)
      {
      DecodeTuple(json["value"], values);
      }
    else if(tupleType == vtkUniforms::TupleTypeVector)
      {
      Json::Value jsonTuple = json["value"];
      for(Json::ArrayIndex index = 0; index < jsonTuple.size(); ++index)
        {
        std::vector<ScalarType> tuple;
        DecodeTuple(jsonTuple[index], tuple);
        values.insert(values.end(), tuple.begin(), tuple.end());
        }
      }
    else if(tupleType == vtkUniforms::TupleTypeMatrix)
      {
      Json::Value jsonTuple = json["value"];
      for(Json::ArrayIndex index = 0; index < jsonTuple.size(); ++index)
        {
        std::vector<ScalarType> matrix;
        DecodeMatrix(jsonTuple[index], matrix);
        values.insert(values.end(), matrix.begin(), matrix.end());
        }
      }
    }
}

//----------------------------------------------------------------------------
void ReadUniforms(Json::Value & uniformsJson, vtkUniforms * uniforms)
{
  for(Json::ArrayIndex index = 0; index < uniformsJson.size(); ++index)
    {
    Json::Value uniformJson = uniformsJson[index];
    std::string uName = uniformJson["name"].asString();
    int scalarType = vtkUniforms::StringToScalarType(uniformJson["scalarType"].asString());
    vtkUniforms::TupleType tupleType = vtkUniforms::StringToTupleType(uniformJson["tupleType"].asString());
    int nbComponents = uniformJson["numberOfComponents"].asInt();
    int nbTuples = uniformJson["numberOfTuples"].asInt();
    if(scalarType == VTK_INT)
      {
      std::vector<int> values;
      DecodeValues(uniformJson, tupleType, nbComponents, nbTuples, values);
      uniforms->SetUniform(uName.c_str(), tupleType, nbComponents, values);
      }
    else if(scalarType == VTK_FLOAT)
      {
      std::vector<float> values;
      DecodeValues(uniformJson, tupleType, nbComponents, nbTuples, values);
      uniforms->SetUniform(uName.c_str(), tupleType, nbComponents, values);
      }
    }
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
int vtkMRMLShaderPropertyStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLShaderPropertyNode *spNode =
    vtkMRMLShaderPropertyNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  std::ifstream ifs;
  ifs.open(fullName.c_str(), ios::in);
  if (!ifs)
    {
    vtkErrorMacro("Cannot open shader property file: " << fullName);
    return 0;
    }

  // Read json file
  Json::Reader reader;
  Json::Value root;
  reader.parse(ifs, root, false);
  ifs.close();

  vtkShaderProperty * sp = spNode->GetShaderProperty();

  // Read vertex shader code
  std::string vertexShaderCode = root["VertexShaderCode"].asString();
  if(!vertexShaderCode.empty())
    {
    sp->SetVertexShaderCode(vertexShaderCode.c_str());
    }

  // Read fragment shader code
  std::string fragmentShaderCode = root["FragmentShaderCode"].asString();
  if(!fragmentShaderCode.empty())
    {
    sp->SetFragmentShaderCode(fragmentShaderCode.c_str());
    }

  // Read geometry shader code
  std::string geometryShaderCode = root["GeometryShaderCode"].asString();
  if(!geometryShaderCode.empty())
    {
    sp->SetGeometryShaderCode(geometryShaderCode.c_str());
    }

  // Read vertex shader uniform variables
  Json::Value vertexUniforms = root["VertexUniforms"];
  ReadUniforms(vertexUniforms, spNode->GetVertexUniforms());

  // Read fragment shader uniform variables
  Json::Value fragmentUniforms = root["FragmentUniforms"];
  ReadUniforms(fragmentUniforms, spNode->GetFragmentUniforms());

  // Read fragment shader uniform variables
  Json::Value geometryUniforms = root["GeometryUniforms"];
  ReadUniforms(geometryUniforms, spNode->GetGeometryUniforms());

  // Read shader replacements
  Json::Value shaderReplacement = root["ShaderReplacements"];
  for(Json::ArrayIndex i = 0; i < shaderReplacement.size(); ++i)
    {
    Json::Value spv = shaderReplacement[i];
    std::string shaderType = spv["ShaderType"].asString();
    std::string replacementSpec = spv["ReplacementSpec"].asString();
    bool replaceFirst = spv["replaceFirst"].asBool();
    std::string replacementValue = spv["ReplacementValue"].asString();
    bool replaceAll = spv["replaceAll"].asBool();
    if(shaderType == std::string("Vertex"))
      {
      sp->AddVertexShaderReplacement(replacementSpec, replaceFirst, replacementValue, replaceAll);
      }
    else if(shaderType == std::string("Fragment"))
      {
      sp->AddFragmentShaderReplacement(replacementSpec, replaceFirst, replacementValue, replaceAll);
      }
    else if(shaderType == std::string("Geometry"))
      {
      sp->AddGeometryShaderReplacement(replacementSpec, replaceFirst, replacementValue, replaceAll);
      }
    }

  return 1;
}

namespace
{

//----------------------------------------------------------------------------
template< typename MatrixValueType >
Json::Value EncodeMatrix(const std::vector<MatrixValueType> & matrix, Json::ArrayIndex matrixWidth, Json::ArrayIndex offset = 0)
{
  Json::Value json;
  for(Json::ArrayIndex rowIndex = 0; rowIndex < matrixWidth; ++rowIndex)
    {
    Json::Value row;
    for(Json::ArrayIndex columnIndex = 0; columnIndex < matrixWidth; ++columnIndex)
      {
      row[columnIndex] = matrix[offset + rowIndex * matrixWidth + columnIndex];
      }
    json[rowIndex] = row;
    }
  return json;
}

//----------------------------------------------------------------------------
template< typename TupleValueType >
Json::Value EncodeTuple(const std::vector<TupleValueType> & tuple, Json::ArrayIndex tupleLength, Json::ArrayIndex offset = 0)
{
  Json::Value json;
  json.resize(tupleLength);
  for(Json::ArrayIndex index = 0; index < tupleLength; ++index)
    {
    json[index] = tuple[index+offset];
    }
  return json;
}

//----------------------------------------------------------------------------
template< typename scalarT >
void EncodeValues(const std::vector<scalarT> & values, Json::ArrayIndex nbComponents, Json::ArrayIndex nbTuples, vtkUniforms::TupleType tt, Json::Value & json)
{
  if(nbTuples == 1)
    {
    if(tt == vtkUniforms::TupleTypeScalar)
      {
      json["value"] = values[0];
      }
    else if(tt == vtkUniforms::TupleTypeVector)
      {
      json["value"] = EncodeTuple(values, nbComponents);
      }
    else if(tt == vtkUniforms::TupleTypeMatrix)
      {
      Json::ArrayIndex matrixWidth = static_cast<Json::ArrayIndex>(sqrt(nbComponents));
      json["value"] = EncodeMatrix(values, matrixWidth);
      }
    }
  else
    {
    if(tt == vtkUniforms::TupleTypeScalar)
      {
      json["value"] = EncodeTuple(values, nbTuples);
      }
    else if(tt == vtkUniforms::TupleTypeVector)
      {
      json["value"].resize(nbTuples);
      for(Json::ArrayIndex index = 0; index < nbTuples; ++index)
        {
        json["value"][index] = EncodeTuple(values, nbComponents, index * nbComponents);
        }
      }
    else if(tt == vtkUniforms::TupleTypeMatrix)
    {
      Json::ArrayIndex matrixWidth = static_cast<Json::ArrayIndex>(sqrt(nbComponents));
      json["value"].resize(nbTuples);
      for(Json::ArrayIndex index = 0; index < nbTuples; ++index)
        {
        json["value"][index] = EncodeMatrix(values, matrixWidth, index * nbComponents);
        }
      }
    }
}

//----------------------------------------------------------------------------
void WriteUniforms(vtkUniforms * uniforms, Json::Value & root)
{
  root.resize(static_cast<Json::ArrayIndex>(uniforms->GetNumberOfUniforms()));
  for(int i = 0; i < uniforms->GetNumberOfUniforms(); ++i)
    {
    std::string uName = uniforms->GetNthUniformName(i);

    vtkUniforms::TupleType tupleType = uniforms->GetUniformTupleType(uName.c_str());
    Json::ArrayIndex nbTuples = static_cast<Json::ArrayIndex>(uniforms->GetUniformNumberOfTuples(uName.c_str()));
    Json::ArrayIndex nbComponents = static_cast<Json::ArrayIndex>(uniforms->GetUniformNumberOfComponents(uName.c_str()));
    int scalarType = uniforms->GetUniformScalarType(uName.c_str());

    Json::Value uniformsJson;
    uniformsJson["name"] = uName;
    uniformsJson["scalarType"] = vtkUniforms::ScalarTypeToString(scalarType);
    uniformsJson["tupleType"] = vtkUniforms::TupleTypeToString(tupleType);
    uniformsJson["numberOfComponents"] = nbComponents;
    uniformsJson["numberOfTuples"] = nbTuples;

    if(scalarType == VTK_INT)
      {
      std::vector<int> values;
      if(uniforms->GetUniform(uName.c_str(), values))
        {
        EncodeValues(values, nbComponents, nbTuples, tupleType, uniformsJson);
        }
      }
    else if(scalarType == VTK_FLOAT)
      {
      std::vector<float> values;
      if(uniforms->GetUniform(uName.c_str(), values))
        {
        EncodeValues(values, nbComponents, nbTuples, tupleType, uniformsJson);
        }
      }
    root[i] = uniformsJson;
    }
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
int vtkMRMLShaderPropertyStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLShaderPropertyNode *spNode = vtkMRMLShaderPropertyNode::SafeDownCast(refNode);

  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLShaderPropertyStorageNode: File name not specified");
    return 0;
    }

  std::ofstream ofs;
  ofs.open(fullName.c_str(), ios::out);

  if (!ofs)
    {
    vtkErrorMacro("Cannot open shader property file: " << fullName);
    return 0;
    }

  Json::Value root;

  vtkShaderProperty * shaderProperty = spNode->GetShaderProperty();
  if(shaderProperty->HasVertexShaderCode())
  {
    std::string vertexShaderCode(shaderProperty->GetVertexShaderCode());
    root["VertexShaderCode"] = vertexShaderCode;
  }
  if(shaderProperty->HasFragmentShaderCode())
  {
    std::string fragmentShaderCode(shaderProperty->GetFragmentShaderCode());
    root["FragmentShaderCode"] = fragmentShaderCode;
  }
  if(shaderProperty->HasGeometryShaderCode())
  {
    std::string geometryShaderCode(shaderProperty->GetGeometryShaderCode());
    root["GeometryShaderCode"] = geometryShaderCode;
  }

  // Collect vertex uniforms json structures
  Json::Value vertexUniforms;
  WriteUniforms(spNode->GetVertexUniforms(), vertexUniforms);
  root["VertexUniforms"] = vertexUniforms;

  // Collect fragment uniforms json structures
  Json::Value fragmentUniforms;
  WriteUniforms(spNode->GetFragmentUniforms(), fragmentUniforms);
  root["FragmentUniforms"] = fragmentUniforms;

  // Collect geometry uniforms json structures
  Json::Value geometryUniforms;
  WriteUniforms(spNode->GetGeometryUniforms(), geometryUniforms);
  root["GeometryUniforms"] = geometryUniforms;

  // Collect shader replacements in json structures
  Json::Value replacements;
  replacements.resize(static_cast<Json::ArrayIndex>(shaderProperty->GetNumberOfShaderReplacements()));
  for(Json::ArrayIndex index = 0; index < replacements.size(); ++index)
    {
    Json::Value property;
    property["ShaderType"] = shaderProperty->GetNthShaderReplacementTypeAsString(index);
    std::string replacementSpec;
    bool replaceFirst = false;
    std::string replacementValue;
    bool replaceAll = false;
    shaderProperty->GetNthShaderReplacement(index, replacementSpec, replaceFirst, replacementValue, replaceAll);
    property["ReplacementSpec"] = replacementSpec;
    property["replaceFirst"] = replaceFirst;
    property["ReplacementValue"] = replacementValue;
    property["replaceAll"] = replaceAll;
    replacements[index] = property;
    }
  root["ShaderReplacements"] = replacements;

  // Write the file
  Json::StyledStreamWriter writer;
  writer.write(ofs, root);

  ofs.close();

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Shader Property (.sp)");
}

//----------------------------------------------------------------------------
void vtkMRMLShaderPropertyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Shader Property (.sp)");
  this->SupportedWriteFileTypes->InsertNextValue("Shader Property (.*)");
}
