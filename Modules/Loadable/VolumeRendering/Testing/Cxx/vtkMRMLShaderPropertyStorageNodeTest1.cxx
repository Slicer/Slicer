/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLShaderPropertyNode.h"
#include "vtkMRMLShaderPropertyStorageNode.h"
#include "vtkShaderProperty.h"
#include "vtkUniforms.h"

#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
void BuildShaderProperty( vtkMRMLShaderPropertyNode * psNode );
int TestReadWriteData(vtkMRMLScene* scene, const char *extension, vtkMRMLShaderPropertyNode* sp );

int vtkMRMLShaderPropertyStorageNodeTest1(int argc, char * argv[])
{
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
    }

  // Test basic methods
  vtkNew<vtkMRMLShaderPropertyStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  // Create scene
  vtkNew<vtkMRMLScene> scene;
  const char* tempDir = argv[1];
  scene->SetRootDirectory(tempDir);

  // Create shader property node
  vtkNew<vtkMRMLShaderPropertyNode> spNode;
  BuildShaderProperty(spNode.GetPointer());

  CHECK_NOT_NULL(spNode->GetShaderProperty());
  CHECK_NOT_NULL(scene->AddNode(spNode.GetPointer()));
  CHECK_EXIT_SUCCESS(TestReadWriteData(scene, ".sp", spNode.GetPointer()));

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
void BuildShaderProperty( vtkMRMLShaderPropertyNode * spNode )
{
  vtkShaderProperty * sp = spNode->GetShaderProperty();

  // Exercise shader replacement code - this is not actually usable code,
  // it is only for the purpose of testing round trip read/write
  sp->AddVertexShaderReplacement( "VTK::Clipping::Impl", true, "var1 = 1.0;", true );
  sp->AddFragmentShaderReplacement( "VTK::Clipping::Impl", true, "var2 = 2;", true );
  sp->AddGeometryShaderReplacement( "VTK::Clipping::Impl", true, "var3 = 3.0;", true );
  sp->SetVertexShaderCode("int main() { float var1 = 3.0; }");
  sp->SetFragmentShaderCode("int main() { int var2 = 4; }");
  sp->SetGeometryShaderCode("int main() { float var3 = 5.0; }");

  // Exercise all combinations of uniforms types:
  // number of tuples{1,many(2)}, tupleType{scalar,vector,matrix}, scalarType{int,float}
  vtkUniforms * vertexUniforms = sp->GetVertexCustomUniforms();

  // 1 tuple, scalar
  vertexUniforms->SetUniformf( "var1-1t-sf", 1.2f );
  vertexUniforms->SetUniformi( "var2-1t-si", 2 );

  // 1 tuple, vector
  int var3[2] = { 3, 4 };
  vertexUniforms->SetUniform2i("var3-1t-vi", var3);
  float var4[2] = { 3.0f, 4.0f };
  vertexUniforms->SetUniform2f("var4-1t-vf",var4);

  // We put the next cases in the fragment uniforms
  vtkUniforms * fragmentUniforms = sp->GetFragmentCustomUniforms();

  // 1 tuple matrix
  float var5[9] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 };
  vertexUniforms->SetUniformMatrix3x3("var5-1t-matf",var5);

  // 2 tuples, scalar
  int var7[2] = { 1, 2 };
  fragmentUniforms->SetUniform1iv("var7-2t-si",2,var7);
  float var8[2] = {1.0f,2.0f};
  fragmentUniforms->SetUniform1fv("var8-2t-sf",2,var8);

  // We put the next cases in the fragment uniforms
  vtkUniforms * geometryUniforms = sp->GetGeometryCustomUniforms();

  // 2 tuples, vector
  float var9[2][2] = {{1.0f, 2.0f},{3.0f, 4.0f}};
  geometryUniforms->SetUniform2fv("var9-2t-vf",2,var9);

  // 2 tuples, matrix
  float var10[32];
  std::fill(var10,var10+16,5.0f);
  std::fill(var10+16,var10+32,8.0f);
  geometryUniforms->SetUniformMatrix4x4v("var10-2t-mat4x4f",2,var10);
}

//---------------------------------------------------------------------------
int CompareUniforms( vtkUniforms * u1, vtkUniforms * u2 )
{
  CHECK_BOOL(u1->GetNumberOfUniforms()==u2->GetNumberOfUniforms(),true);
  for( int i = 0; i < u1->GetNumberOfUniforms(); ++i )
  {
    const char * name = u1->GetNthUniformName(i);
    CHECK_BOOL(u1->GetUniformScalarType(name)==VTK_INT || u1->GetUniformScalarType(name)==VTK_FLOAT,true);
    CHECK_BOOL(u1->GetUniformScalarType(name)==u2->GetUniformScalarType(name),true);
    if( u1->GetUniformScalarType(name) == VTK_INT )
    {
      std::vector<int> u1Val;
      u1->GetUniform(name,u1Val);
      std::vector<int> u2Val;
      u2->GetUniform(name,u2Val);
      CHECK_BOOL(u1Val == u2Val,true);
    }
    else if( u1->GetUniformScalarType(name) == VTK_FLOAT )
    {
      std::vector<float> u1Val;
      u1->GetUniform(name,u1Val);
      std::vector<float> u2Val;
      u2->GetUniform(name,u2Val);
      CHECK_BOOL(u1Val == u2Val,true);
    }
  }
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int CompareShaderProperty( vtkMRMLShaderPropertyNode * n1, vtkMRMLShaderPropertyNode * n2 )
{
  vtkShaderProperty * sp1 = n1->GetShaderProperty();
  CHECK_NOT_NULL(sp1);
  vtkShaderProperty * sp2 = n2->GetShaderProperty();
  CHECK_NOT_NULL(sp2);

  // Compare whole shader code
  CHECK_STRING(sp1->GetVertexShaderCode(),sp2->GetVertexShaderCode());
  CHECK_STRING(sp1->GetFragmentShaderCode(),sp2->GetFragmentShaderCode());
  CHECK_STRING(sp1->GetGeometryShaderCode(),sp2->GetGeometryShaderCode());

  // Compare partial shader replacements
  CHECK_BOOL(sp1->GetNumberOfShaderReplacements() == sp2->GetNumberOfShaderReplacements(),true);
  for( int i = 0; i < sp1->GetNumberOfShaderReplacements(); ++i)
  {
    std::string name1;
    bool replaceFirst1 = false;
    std::string replacementValue1;
    bool replaceAll1 = false;
    sp1->GetNthShaderReplacement(i,name1,replaceFirst1,replacementValue1,replaceAll1);
    std::string name2;
    bool replaceFirst2 = false;
    std::string replacementValue2;
    bool replaceAll2 = false;
    sp2->GetNthShaderReplacement(i,name2,replaceFirst2,replacementValue2,replaceAll2);
    CHECK_BOOL(sp1->GetNthShaderReplacementTypeAsString(i) == sp2->GetNthShaderReplacementTypeAsString(i),true);
    CHECK_BOOL(name1 == name2,true);
    CHECK_BOOL(replaceFirst1 == replaceFirst2,true);
    CHECK_BOOL(replacementValue1 == replacementValue2,true);
    CHECK_BOOL(replaceAll1 == replaceAll2,true);
  }

  // Compare uniform variables
  vtkUniforms * uv1 = sp1->GetVertexCustomUniforms();
  vtkUniforms * uv2 = sp2->GetVertexCustomUniforms();
  CHECK_EXIT_SUCCESS(CompareUniforms( uv1, uv2 ));

  vtkUniforms * uf1 = sp1->GetFragmentCustomUniforms();
  vtkUniforms * uf2 = sp2->GetFragmentCustomUniforms();
  CHECK_EXIT_SUCCESS(CompareUniforms( uf1, uf2 ));

  vtkUniforms * ug1 = sp1->GetGeometryCustomUniforms();
  vtkUniforms * ug2 = sp2->GetGeometryCustomUniforms();
  CHECK_EXIT_SUCCESS(CompareUniforms( ug1, ug2 ));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestReadWriteData(vtkMRMLScene* scene, const char *extension, vtkMRMLShaderPropertyNode* spNode )
{
  std::string fileName = std::string(scene->GetRootDirectory()) +
    std::string("/vtkMRMLShaderPropertyStorageNodeTest1") +
    std::string(extension);

  vtksys::SystemTools::RemoveFile(fileName);

  // Add storage node
  vtkNew<vtkMRMLShaderPropertyStorageNode> storageNode;
  storageNode->SetFileName(fileName.c_str());

  // Test writing
  CHECK_BOOL(storageNode->WriteData(spNode), true);

  // Test reading
  vtkNew<vtkMRMLShaderPropertyNode> spNodeRead;
  CHECK_BOOL(storageNode->ReadData(spNodeRead), true);

  // Compare read/write nodes
  CHECK_EXIT_SUCCESS(CompareShaderProperty(spNode,spNodeRead));

  return EXIT_SUCCESS;
}
