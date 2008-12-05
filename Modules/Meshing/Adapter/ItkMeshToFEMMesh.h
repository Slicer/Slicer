#ifndef __ItkMeshToFEMMesh_h__
#define __ItkMeshToFEMMesh_h__

#include "itkLightProcessObject.h"
#include <iostream>
#include "itkMesh.h"
#include "itkAutomaticTopologyMeshSource.h"
#include "itkTriangleCell.h"
#include "itkQuadrilateralCell.h"
#include "itkTetrahedronCell.h"
#include "itkHexahedronCell.h"
#include "itkFEM.h"
#include "itkCellInterface.h"

namespace itk
{
template<class TInputMesh>
class ITK_EXPORT ItkMeshToFEMMesh : public LightProcessObject

  {

  public:

    typedef ItkMeshToFEMMesh Self;    typedef SmartPointer<Self> Pointer;


    typedef TInputMesh MeshType;

    typedef typename MeshType::Pointer                        MeshTypePointer;
    typedef typename MeshType::CellType                       CellType;
    typedef typename MeshType::PointType                      PointType;
    typedef typename MeshType::CellPixelType                  CellPixelType;
    typedef typename MeshType::CellsContainerPointer          CellsContainerPointer;
    typedef typename MeshType::CellsContainer::ConstIterator  CellIterator;
    typedef typename itk::VertexCell< CellType >              VertexType;
    typedef typename itk::HexahedronCell< CellType >          HexahedronType;
    typedef typename HexahedronType::PointIdIterator          PointIdIterator;

    
    typedef itk::fem::Solver                                  SolverType;
    typedef SolverType*                                       SolverPointerType;
    typedef itk::fem::MaterialLinearElasticity                MaterialType;
    typedef itk::fem::Element3DC0LinearHexahedronStrain       HexElementType;
    typedef HexElementType::Node                              HexNodeType;
    typedef itk::fem::Element2DC0LinearTriangularStrain       TriangElementType;
    typedef TriangElementType::Node                           TriangNodeType;

    /**
    Useful Internal Typedefs
    
    typedef itk::AutomaticTopologyMeshSource< MeshType >  MeshSourceType;
    typedef MeshSourceType::Pointer  MeshSourceTypePointer;
*/
    itkNewMacro( Self );

    itkSetObjectMacro ( Input, MeshType );

    itkSetStringMacro( FileName );

    void Update ( );
    SolverPointerType GetOutput ( );
  
protected:
  
    MeshTypePointer   m_Input;
    std::string       m_FileName;   
    SolverPointerType       m_Solver ;
    
    ItkMeshToFEMMesh( );
    virtual ~ItkMeshToFEMMesh( );
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "ItkMeshToFEMMesh.txx"
#endif

#endif
