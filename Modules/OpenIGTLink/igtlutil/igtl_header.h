/*=========================================================================

  Program:   Open ITK Link Library
  Module:    $RCSfile: $
  Language:  C
  Date:      $Date: $
  Version:   $Revision: $

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __IGTL_HEADER_H
#define __IGTL_HEADER_H

#define IGTL_HEADER_VERSION   1
#define IGTL_HEADER_SIZE      58

#include "igtl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)     /* For 1-byte boundary in memroy */

/*
 * Message header of OpenIGTLink message.
 *
 * igtl_header is an overall data header for OpenIGTLink protocol.
 * It is transfered at beginning of every OpenIGTLink message to give
 * type and size of following data body to a receiver.
 * These parameters allow the receiver to parse or skip the data body.
 */

typedef struct {
  igtl_uint16    version;          /* protocol version number */
  char           name[12];          /* data type name          */
  char           device_name[20];  /* device name             */
  igtl_uint64    timestamp;        /* time stamp message      */
  igtl_uint64    body_size;        /* size of the body        */
  igtl_uint64    crc;              /* CRC                     */
} igtl_header;

#pragma pack()

/*
 * Byte order converter for the header structre
 * 
 * igtl_header_convert_byte_order convers endianness of each
 * member variable in igtl_header structre from host byte order
 * to network byte order, or vice versa.
 *
 */
void igtl_header_convert_byte_order(igtl_header * header);

#ifdef __cplusplus
}
#endif

#endif // __IGTL_HEADER_H

