#include <itkObjectFactory.h>
#include <itkObject.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>

#include <iostream>
#include <fstream>

#include "Air16_rw.h"
#include "ConvertToRigidAffine.h"


void swapAir16(struct AIR_Air16 *air)
{
  FourByteSwap ( &( air->s.bits ) );
  FourByteSwap ( &( air->s.x_dim ) );
  FourByteSwap ( &( air->s.y_dim ) );
  FourByteSwap ( &( air->s.z_dim ) );
  EightByteSwap ( &( air->s.x_size ) );
  EightByteSwap ( &( air->s.y_size ) );
  EightByteSwap ( &( air->s.z_size ) );
  FourByteSwap ( &( air->r.bits ) );
  FourByteSwap ( &( air->r.x_dim ) );
  FourByteSwap ( &( air->r.y_dim ) );
  FourByteSwap ( &( air->r.z_dim ) );
  EightByteSwap ( &( air->r.x_size ) );
  EightByteSwap ( &( air->r.y_size ) );
  EightByteSwap ( &( air->r.z_size ) );
  for ( int i = 0; i < 4; i++ )
    {
    for ( int j = 0; j < 4; j++ )
      {
      EightByteSwap ( &( air->e[i][j] ) );
      }
    }
}

CrossOverAffineSystem<double, 3> ::AffineTransformPointer ReadAir16File (
  std::string airFile)
{
  FILE *AirFilePointer = fopen(airFile.c_str(), "rb");

  if ( AirFilePointer == NULL )
    {
    std::cout << "Air file '" << airFile << "' couldn't be opened 'rb'."
      << std::endl;
    exit(-1);
    }
  struct AIR_Air16 airBlk;
  fread(&airBlk, sizeof( struct AIR_Air16 ), 1, AirFilePointer);
  fclose(AirFilePointer);

  // Need to implement byteswapping code here.
  const double F64_UINT_MAX = UINT_MAX;
  if ( airBlk.s.bits > vcl_sqrt(F64_UINT_MAX) )  // copied this test from
    // scanair source code.
    {
    swapAir16(&airBlk);
    }
  if ( ( airBlk.s.x_dim <= 0 ) || ( airBlk.s.x_dim > 2048 ) )
    {
    swapAir16(&airBlk);
    fprintf(
      stderr,
      "Apparently, the s.bits field was garbage, following the brains2 heuristic on the range of x_dim.\n");
    }

  // Transfer values from air structures.
  // Transfer to intermediate 4x4 matrix so that it can be checked easily in
  // octave or matlab

  CrossOverAffineSystem<double, 3> ::VnlTransformMatrixType44 Q_AIR(0.0);
  for ( int j = 0; j < 4; j++ )
    {
    for ( int i = 0; i < 4; i++ )
      {
      Q_AIR.put(i, j, airBlk.e[j][i]); // NOTE:  Need to transpose the "e"
      // matrix
      }
    }

  std::cout << "=======================Q_AIR input\n" << Q_AIR;

  CrossOverAffineSystem<double,3> ::AffineTransformPointer result
    = CrossOverAffineSystem<double,3> ::AffineTransformType::New();
  AssignRigid::AssignConvertedTransform( result, Q_AIR );

  return result;
}
