// (c) 2000-2007 Electronic Arts Inc.
#ifndef LBZLIBCOMP_H
#define LBZLIBCOMP_H
#include "LbTypes.h"

typedef SLONG (__stdcall *TbZLibBufferCallback) ( VOID *buffer , ULONG size , VOID *user_data );  

// Memory Compress
LB_API(ULONG) LbZLib_MemCompress( VOID *input , VOID *output , ULONG input_size , VOID *workspace );

// Memory Decompress
LB_API(ULONG)	LbZLib_MemDecompress( VOID *input , VOID *output , VOID *workspace );

// Buffered Compress
LB_API(ULONG)	LbZLib_BufferedCompress( VOID *input , VOID *buf , ULONG input_size ,
							   ULONG buf_size , TbZLibBufferCallback callback ,
							   VOID *buffer_user_data, VOID *alloc_user_data);

// Buffered Decompress
LB_API(ULONG)	LbZLib_BufferedDecompress( VOID *output , VOID *buf , ULONG buf_size ,
								 TbZLibBufferCallback callback , VOID *buffer_user_data, VOID *alloc_user_data);

// Gets the uncompressed size from a compressed item.
LB_API(ULONG)	LbZLib_MemDecompressedSize( VOID *input );

//  Workspace size
LB_API(ULONG) LbZLib_CompressWorkspaceSize( VOID );
LB_API(ULONG) LbZLib_DeCompressWorkspaceSize( VOID );


#endif // LBZLIBCOMP_H