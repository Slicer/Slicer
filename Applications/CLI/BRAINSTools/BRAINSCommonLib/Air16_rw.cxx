#include <itkObjectFactory.h>
#include <itkObject.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>

#include <iostream>
#include <fstream>

#include "Air16_rw.h"
#include "ConvertToRigidAffine.h"

#if 0
// HACK: TODO:  All the byte swapping code should use methods for itksys
// routines.

static void
FourByteSwap (void *const pntr)
{
  /*NOTE: This can probably be speed up by using byte swapping */
  const unsigned char b0 = *( (unsigned char *)(pntr) + 0 );
  const unsigned char b1 = *( (unsigned char *)(pntr) + 1 );
  const unsigned char b2 = *( (unsigned char *)(pntr) + 2 );
  const unsigned char b3 = *( (unsigned char *)(pntr) + 3 );

  *( (unsigned char *)(pntr) + 0 ) = b3;
  *( (unsigned char *)(pntr) + 1 ) = b2;
  *( (unsigned char *)(pntr) + 2 ) = b1;
  *( (unsigned char *)(pntr) + 3 ) = b0;
}

static void
EightByteSwap (void *const pntr)
{
  const unsigned char b0 = *( (unsigned char *)(pntr) + 0 );
  const unsigned char b1 = *( (unsigned char *)(pntr) + 1 );
  const unsigned char b2 = *( (unsigned char *)(pntr) + 2 );
  const unsigned char b3 = *( (unsigned char *)(pntr) + 3 );
  const unsigned char b4 = *( (unsigned char *)(pntr) + 4 );
  const unsigned char b5 = *( (unsigned char *)(pntr) + 5 );
  const unsigned char b6 = *( (unsigned char *)(pntr) + 6 );
  const unsigned char b7 = *( (unsigned char *)(pntr) + 7 );

  *( (unsigned char *)(pntr) + 0 ) = b7;
  *( (unsigned char *)(pntr) + 1 ) = b6;
  *( (unsigned char *)(pntr) + 2 ) = b5;
  *( (unsigned char *)(pntr) + 3 ) = b4;
  *( (unsigned char *)(pntr) + 4 ) = b3;
  *( (unsigned char *)(pntr) + 5 ) = b2;
  *( (unsigned char *)(pntr) + 6 ) = b1;
  *( (unsigned char *)(pntr) + 7 ) = b0;
}

IPL_ENDIAN_TYPE
getMachineEndianess (void)
{
  int check;

  /*00000000 */
  ( (unsigned char *)(&check) )[0] = 0;
  /*00000001 */
  ( (unsigned char *)(&check) )[1] = 1;
  /*00000010 */
  ( (unsigned char *)(&check) )[2] = 2;
  /*00000011 */
  ( (unsigned char *)(&check) )[3] = 3;
  switch ( check )
    {
    /*00000000 00000001 00000010 0000011 -> 66051  Mips/SGI */
    case IPL_BIG_ENDIAN:
      return ( IPL_BIG_ENDIAN );
    /*00000011 00000010 00000001 0000000 -> 5046297  Intel x86 Alpha AMD */
    case IPL_LITTLE_ENDIAN:
      return ( IPL_LITTLE_ENDIAN );
    /*00000001 00000000 00000011 0000010 -> 1677986  Vax */
    case IPL_MIXED_ENDIAN:
      return ( IPL_MIXED_ENDIAN );
    default:
      return ( IPL_UNKNOWN_ENDIAN );
    }
}

#endif

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
