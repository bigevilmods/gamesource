// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  TM2 - Sony PS2 Texture Gimex                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 020911 FB TM2 based on spec & hsi                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* Reentrant                                                        */
/* Files: tm2read.cpp tm2write.cpp tm2gimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* TM2 files where are used by Sony examples                        */
/*                                                                  */
// TIM2 - PlayStation(R) 2 2D Graphics
// - File Format Specification
// [CONFIDENTIAL]
// -----------------------------------------------------------
// 02/26/2001
// Copyright(C) web technology Corp.
//
// TIM2 Format Specification ver.4
//
// 1. Introduction
//
// This document describes the "TIM2 format," which is a 2D image format for the PlayStation 2 (PS2).
//
//
// 2. Overview
//
// The TIM2 format is an image format that considers ease of handling by a PS2 program, by arranging data to accommodate the PS2 "EmotionEngine (TM)" (EE) and "Graphics Synthesizer(R)" (GS). The alignment of each data value is adjusted to match a 16-byte or 128-byte boundary so that the data can be directly transferred with EE DMA.  In addition, pixel data is maintained, as much as possible, in a format that can be transferred asis to GS local memory.
//
// A TIM2 format image file has ".TM2" or ".tm2" as its extension.
//
// The TIM2 format maintains many kinds of information. However, not all of this information need always be included. Information should be selected and used as necessary.
//
//     The TIM2 format has the following structure.
//          |---------------|
//          | File header   |
//          |---------------|
//          | Picture data  |
//          |---------------|
//          |       :       |
//          |---------------|
//          | Picture data  |
//          |---------------|
//     - An arbitrary number of picture data parts follows the file header, which has a fixed size of 16 bytes.
//     - The number of picture data parts is specified in the file header.
//     - The offset of each picture data part from the beginning of the file is adjusted so that the data is aligned at a 16-byte or 128-byte boundary.
//     - Whether 16 or 128 bytes is used for the boundary alignment is specified in the file header.
//
//
//     Each picture data part has the following structure.
//          |-------------------|
//          | Picture header    |
//          |-------------------|
//          | MIPMAP header     |
//          |-------------------|
//          | User space        |
//          |-------------------|
//          | Image data        |
//          |-------------------|
//          | CLUT data         |
//          |-------------------|
//     - The picture header has a fixed length of 48 bytes.
//     - The size of the MIPMAP header can be obtained from the number of MIPMAPs, which is specified in the picture header.
//     - The header size, which is the total of the sizes of the picture header, MIPMAP header, and user space, is specified within the picture header. The size of the user space can be obtained by using this value.
//     - The size of the image data and the size of the CLUT data are specified in the picture header.
//     - The MIPMAP header, user space, and CLUT data may each not exist.
//     - The beginning of the image data and the beginning of the CLUT data are aligned on a 16-byte or 128-byte boundary. If the image data consists of multiple MIPMAP textures, the second and subsequent textures are always aligned on a 16-byte boundary.
//
//
// 3. Detailed Description of the Format
//
// ee-gcc sizes apply at all locations that are described using C language types such as char and short. Little endian is used for the byte alignment of data in the TIM2 format, in a similar manner as in the EE.
//
//          |---------------------------|
//          | Type name | Size (bytes)  |
//          |-----------|---------------|
//          | char      |       1       |
//          | short     |       2       |
//          | int       |       4       |
//          | long      |       8       |
//          | u_long128 |      16       |
//          |---------------------------|
//
// 3.1 File Header
//
// The file header is located at the beginning of a TIM2 file. One file header exists per file. The size is fixed at 16 bytes and the structure is as follows.
//
//    |-----------------------------------------------------------------------|
//    | Offset    | Size  | Member Name   | Use                               |
//    |-----------|-------|---------------|-----------------------------------|
//    | +0        | 4     | FileId        | File ID                           |
//    | +4        | 1     | FormatVersion | File format version number        |
//    | +5        | 1     | FormatId      | Format ID                         |
//    | +6        | 2     | Pictures      | Number of picture data parts      |
//    | +8        | 8     | Reserved      | Padding                           |
//    |-----------------------------------------------------------------------|
//
//     typedef struct tagTIM2_FILEHEADER {
//             char    FileId[4];      // File ID (always "T", "I", "M", "2")
//             u_char  FormatVersion;  // File format version number
//             u_char  FormatId;       // Format ID
//             u_short Pictures;       // Number of picture data parts
//             char    Reserved[8];    // Padding (always 0x00)
//     } TIM2_FILEHEADER;
//
//     Description of Members
//
//     char FileId[4];
//         This is a four-character string that identifies the file as a TIM2 file. The four characters T, I, M, and 2 are stored sequentially in the order shown.
//
//     u_char FormatVersion;
//         This is the TIM2 file format version number. It takes the following values.
//    |-----------------------------------------------------------------------|
//    | Version Number| Use                                                   |
//    |---------------|-------------------------------------------------------|
//    | 0x00 to 0x7F  | These are reserved version numbers. The user cannot   |
//    |               | define them independently.                            |
//    |---------------|-------------------------------------------------------|
//    | 0x80 to 0xBF  | These are private version numbers. They are used for  |
//    |               | incompatible independent extensions. They cannot be   |
//    |               | read with official tools.                             |
//    |---------------|-------------------------------------------------------|
//    | 0xC0 to 0xFF  | These are private version numbers. They are used for  |
//    |               | compatible independent extensions. They can be read   |
//    |               | with official tools.                                  |
//    |-----------------------------------------------------------------------|
//     The format version number defined by this document is 0x04.
//
//     u_char FormatId;
//         This member indicates the format ID of the TIM2 file. It takes the value 0x00 or 0x01 and defines the data alignment size within the TIM2 file.
//
//       |-----------------------------------------------|
//       | Format ID     | Use                           |
//       |---------------|-------------------------------|
//       | 0x00          |  16-byte alignment mode       |
//       |---------------|-------------------------------|
//       | 0x01          |  128-byte alignment mode      |
//       |-----------------------------------------------|
//
//         EE DMA transfers are most efficient when the starting address is on an 8QWORD (128-byte) boundary. With 128-byte alignment mode, the alignments of the starting points of the picture header, image data, and CLUT data are each adjusted relative to the preceding data so that they are aligned on a 128-byte boundary. For example, since the size of the TIM2 file header is fixed at 16 bytes, 112 bytes of padding are added before the start of the picture header in 128-byte alignment mode.
//
//     u_short Pictures;
//         This member indicates the number of picture data parts that are stored in the TIM2 file.
//
//     char  Reserved[8];
//         This is a padding member for filling out the file header to a fixed size of 16 bytes. It is filled with 0x00.
//
//
// 3.2 Picture Header
//
// The offset of the picture header from the beginning of the file is adjusted so that it is aligned on a 16-byte or 128-byte boundary. The size of the picture header is fixed at 48 bytes and the structure is as follows.
//
//     |---------------------------------------------------------------------|
//     | Offset    | Size  | Member Name   | Use                             |
//     |-----------|-------|---------------|---------------------------------|
//     |      +0   |   4   | TotalSize     | Total size in bytes of all      |
//     |           |       |               | picture data                    |
//     |      +4   |   4   | ClutSize      | Size in bytes of CLUT data part |
//     |      +8   |   4   | ImageSize     | Size in bytes of image data part|
//     |     +12   |   2   | HeaderSize    | Size in bytes of header part    |
//     |     +14   |   2   | ClutColors    | Total number of colors in CLUT  |
//     |           |       |               | data part                       |
//     |     +16   |   1   | PictFormat    | Picture format ID               |
//     |     +17   |   1   | MipMapTextures| Number of MIPMAP textures       |
//     |     +18   |   1   | ClutType      | CLUT data part type             |
//     |     +19   |   1   | ImageType     | Image data part type            |
//     |     +20   |   2   | ImageWidth    | Picture width                   |
//     |     +22   |   2   | ImageHeight   | Picture height                  |
//     |     +24   |   8   | GsTex0        | GS TEX0 register data           |
//     |     +32   |   8   | GsTex1        | GS TEX1 register data           |
//     |     +40   |   4   | GsTexaFbaPabe | GS TEXA, FBA, PABE register data|
//     |     +44   |   4   | GsTexClut     | GS TEXCLUT register data        |
//     |---------------------------------------------------------------------|
//
//       typedef struct tagTIM2_PICTUREHEADER {
//               u_int   TotalSize;      // Total size in bytes of all picture
// 							data
//               u_int   ClutSize;       // Size in bytes of CLUT data part
//               u_int   ImageSize;      // Size in bytes of image data part
//               u_short HeaderSize;     // Size in bytes of header part
//               u_short ClutColors;     // Total number of colors in CLUT data
// 					 part
//               u_char  PictFormat;     // Picture format ID (always 0)
//               u_char  MipMapTextures; // Number of MIPMAP textures
//               u_char  ClutType;       // CLUT data part type
//               u_char  ImageType;      // Image data part type
//               u_short ImageWidth;     // Picture width
//               u_short ImageHeight;    // Picture height
//               u_long  GsTex0;         // GS TEX0 register data
//               u_long  GsTex1;         // GS TEX1 register data
//               u_int   GsRegs;         // GS TEXA, FBA, PABE register data
//               u_int   GsTexClut;      // GS TEXCLUT register data
//       } TIM2_PICTUREHEADER;
//
//     Description of Members
//
//     u_int TotalSize;
//         This member indicates the total size of all picture data. To ensure the alignment, the value must be a multiple of 16 or 128. The value of the TotalSize member is equal to the total of the values of the ClutSize, ImageSize, and HeaderSize members.
//
//     u_int ClutSize;
//         This member indicates the size in bytes of the CLUT data part. To ensure the alignment, the value must be a multiple of 16 or 128.
//
//     u_int   ImageSize;
//         This member indicates the size in bytes of the image data part. To ensure the alignment, the value must be a multiple of 16 or 128. When there are two or more MIPMAP textures, this value is equal to the total of the MIPMAP texture data sizes that are indicated in the MIPMAP header, which is described below.
//
//     u_short HeaderSize;
//         This member indicates the size in bytes of the header part. To ensure the alignment, the value must be a multiple of 16 or 128. The value of this member is the alignment-adjusted value of the total size of the picture header part, MIPMAP header part, and user space part.
//
//     u_short ClutColors;
//         This member indicates the number of colors that are contained in the CLUT data. The number of colors is entered directly, not the bit depth or CLUT byte size. When the pixel format of the image data part is specified as TIM2_IDTEX4 in the ImageType member, ClutColors must be a multiple of 16 colors. Similarly, when the pixel format is TIM2_IDTEX8, ClutColors must be a multiple of 256 colors. Also, when the pixel format of the image data part is TIM2_IDTEX4 and the 16-color, CSM1 CLUT data arrangement order has been changed by setting bits of the ClutType member to 1, ClutColors must be a multiple of 32 colors.
//
//     u_char  PictFormat;
//         This member indicates the picture format ID. Currently, this is always 0x00.
//
//     u_char  MipMapTextures;
//         This member indicates the number of MIPMAP textures. It takes the value obtained by counting the LV0 texture in the number of MIPMAP textures. That is, when a picture consists of only the LV0 texture, this is 0x01. When textures up to the LV6 texture, which is the maximum MIPMAP level, are included, this is 0x07. When a picture consists of only the LV0 texture, the user space immediately follows the picture header. When there are two or more MIPMAP textures, a MIPMAP header immediately follows the picture header. When the picture data contains no texture image data and the picture is only the CLUT part, the value of the MipMapTextures member is 0x00. For details, refer to the "CLUT2 Format" document. If the FileID member of the file header indicates that the file is a TIM2 file, the MipMapTextures member is guaranteed not to be 0x00 for at least the first picture.
//
//     u_char  ClutType;
//         This member indicates the CLUT data storage mode and pixel format. It takes the following values.
//
//         |-------------------------------------------------------------|
//         | Bit   | Use                                                 |
//         |-------|-----------------------------------------------------|
//         |    7  | Indicates the CLUT storage mode. (0=CSM1, 1=CSM2)   |
//         |-------|-----------------------------------------------------|
//         |    6  | CLUT data rearrangement flag (valid only for 16     |
//         |       | colors and CSM1)                                    |
//         |-------|-----------------------------------------------------|
//         | 5 to 0| Indicates the CLUT data pixel format.               |
//         |       | 0 = No CLUT (TIM2_NONE)                             |
//         |       | 1 = 16-bit color (TIM2_RGB16)                       |
//         |       | 2 = 24-bit color (TIM2_RGB24)                       |
//         |       | 3 = 32-bit color (TIM2_RGB32)                       |
//         |-------------------------------------------------------------|
//
//         Besides specifying the CLUT storage mode with bit 7, you can specify CLUT data rearrangement with bit 6 when a 16-color CLUT is to be stored using CSM1. The CLUT data part format is defined according to a combination of this ClutType member and the ImageType member. For details, see Section 4.4, "Concerning Format Specifications for the CLUT Data Part" and Section 4.5, "Concerning the Pixel Arrangement in CLUT Data."
//
//         Note:  This method of appending a numeric value is unrelated to the value when the pixel format is set in the GS register.
//
//     u_char  ImageType;
//         This member indicates the image data pixel format. It takes the following values.
//
//         |---------------------------------------------------------|
//         | Bit   | Use                                             |
//         |-------|-------------------------------------------------|
//         | 7 to 0| Indicates the image data pixel format.          |
//         |       | 0x01 = 16-bit color (TIM2_RGB16)                |
//         |       | 0x02 = 24-bit color (TIM2_RGB24)                |
//         |       | 0x03 = 32-bit color (TIM2_RGB32)                |
//         |       | 0x04 = 4-bit texture (TIM2_IDTEX4)              |
//         |       | 0x05 = 8-bit texture (TIM2_IDTEX8)              |
//         |---------------------------------------------------------|
//
//         Note:  This method of applying a numeric value is unrelated to the value when the pixel format is set in the GS register.
//
//     u_short ImageWidth;
//         This member indicates the picture width. Although the value that is specified here need not be a power of 2, it must be a value for which the size in bytes of the texture data constituting one raster, up to the highest order MIPMAP texture, is obtained as an integer value. Also, when index color is used as the pixel format (TIM2_IDTEX8 or TIM2_IDTEX4), the texture width must be a multiple of 2 or a multiple of 4, respectively. The following table summarizes conditions for the picture width according to the number of MIPMAP textures and the pixel format.
//
//         |---------------------------------------------------------------|
//         | Number of     | Pixel Format        | ImageWidth Member       |
//         | MIPMAP        |                     |   Numeric Value         |
//         | Textures      |                     |   Conditions            |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | None                    |
//         |               |---------------------|-------------------------|
//         | LV0 only      | TIM2_IDTEX8         | Integer multiple of 2   |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 4   |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 2   |
//         |               |---------------------|-------------------------|
//         | Up to LV1     | TIM2_IDTEX8         | Integer multiple of 4   |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 8   |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 4   |
//         |               |---------------------|-------------------------|
//         | Up to LV2     | TIM2_IDTEX8         | Integer multiple of 8   |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 16  |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 8   |
//         |               |---------------------|-------------------------|
//         | Up to LV3     | TIM2_IDTEX8         | Integer multiple of 16  |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 32  |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 16  |
//         |               |---------------------|-------------------------|
//         | Up to LV4     | TIM2_IDTEX8         | Integer multiple of 32  |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 64  |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 32  |
//         |               |---------------------|-------------------------|
//         | Up to LV5     | TIM2_IDTEX8         | Integer multiple of 64  |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 128 |
//         |---------------|---------------------|-------------------------|
//         |               | TIM2_RGB32,24,16    | Integer multiple of 64  |
//         |               |---------------------|-------------------------|
//         | Up to LV6     | TIM2_IDTEX8         | Integer multiple of 128 |
//         |               |---------------------|-------------------------|
//         |               | TIM2_IDTEX4         | Integer multiple of 256 |
//         |---------------------------------------------------------------|
//
//     u_short ImageHeight;
//         This member indicates the picture height. The value that is specified here need not be a power of 2.
//
//     u_long  GsTex0;
//         This member maintains data in the same form, as data set in the GS TEX0 register.
//
//         |-------------------------------------------------------------|
//         | Bit     | Field Name    | Use                               |
//         |---------|---------------|-----------------------------------|
//         | 63 to 61|      CLD      | CLUT buffer load control          |
//         | 60 to 56|      CSA      | CLUT entry offset                 |
//         |   55    |      CSM      | CLUT storage mode                 |
//         | 54 to 51|      CPSM     | CLUT pixel storage format         |
//         | 50 to 37|      CBP      | CLUT buffer base point            |
//         | 36 to 35|      TFX      | Texture function                  |
//         |   34    |      TCC      | Texture color component           |
//         | 33 to 30|      TH       | Texture height                    |
//         | 29 to 26|      TW       | Texture width                     |
//         | 25 to 20|      PSM      | Texture pixel storage format      |
//         | 19 to 14|      TBW      | Texture buffer width              |
//         | 13 to 0 |      TBP0     | Texture base point                |
//         |-------------------------------------------------------------|
//
//     u_long  GsTex1;
//         This member maintains data in the same form, as data set in the GS TEX1 register.
//
//         |-------------------------------------------------------------|
//         | Bit     | Field Name    | Use                               |
//         |---------|---------------|-----------------------------------|
//         | 63 to 44|               | Reserved (always 0)               |
//         | 43 to 32|     K         | LOD parameter value K             |
//         | 31 to 21|               | Reserved (always 0)               |
//         | 20 to 19|     L         | LOD parameter value L             |
//         | 18 to 10|               | Reserved (always 0)               |
//         |    9    |     MTBA      | Automatic calculation of base     |
//         |         |               | address of MIPMAP textures for    |
//         |         |               | levels 1, 2, 3                    |
//         |  8 to 6 |     MMIN      | Texture reduction filter          |
//         |    5    |     MMAG      | Texture magnification filter      |
//         |  4 to 2 |     MXL       | Maximum MIP level                 |
//         |    1    |               | Reserved (always 0)               |
//         |    0    |     LCM       | LOD calculation formula           |
//         |-------------------------------------------------------------|
//
//     u_int   GsTexaFbaPabe;
//         This member maintains data that is set in the GS TEXA, FBA, and PABE registers in a combined form.
//
//         |-------------------------------------------------------------|
//         | Bit     | Field Name    | Use                               |
//         |---------|---------------|-----------------------------------|
//         |   31    |     FBA       | FBA bit of FBA_1, FBA_2 register  |
//         |   30    |     PABE      | PABE bit of PABE register         |
//         | 29 to 24|               | Reserved (always 0)               |
//         | 23 to 16|     TA1       | TA1 field of TEXA register        |
//         |   15    |     AEM       | AEM bit of TEXA register          |
//         | 14 to 8 |               | Reserved (always 0)               |
//         |  7 to 0 |     TA0       | TA0 field of TEXA register        |
//         |-------------------------------------------------------------|
//
//     u_int   GsTexClut;
//         This member maintains the low-order 32 bits of the data that is set in the GS TEXCLUT register. It is valid only when CSM2 mode is specified for the CLUT storage made in the ClutType member.
//
//         |-------------------------------------------------|
//         | Bit     | Field Name    | Use                   |
//         |---------|---------------|-----------------------|
//         | 31 to 22|               | Reserved (always 0)   |
//         | 21 to 12|     COV       | CLUT offset V         |
//         | 11 to 6 |     COU       | CLUT offset U         |
//         |  5 to 0 |     CBW       | CLUT buffer width     |
//         |-------------------------------------------------|
//
//
// 3.3 MIPMAP Header
//
// A MIPMAP header exists when two or more MIPMAP textures are indicated by the MipMapTextures member in the picture header. When there is only one MIPMAP texture, no MIPMAP header exists and the user space immediately follows the picture header.
//
// When there are n MIPMAP textures, the MIPMAP header has the following structure.
//
//     typedef struct tagTIM2_MIPMAPHEADER {
//             u_long  GsMiptbp1;      // Value set in the GS MIPTBP1 register
//             u_long  GsMiptbp2;      // Value set in the GS MIPTBP2 register
//             u_int   Size[n];        // Array indicating the size in bytes of
//                                     // the texture data
//     } TIM2_MIPMAPHEADER;
//
//     The MIPMAP header has a variable length. Padding may be added to align it on a 16-byte boundary. The following table shows the relationship between the number of MIPMAP textures and the header size.
//
//         |---------------------------------------------------|
//         | Number of | MIPMAP Header Size  | Padding Size    |
//         | MIPMAP    |                     |                 |
//         | Textures  |                     |                 |
//         |-----------|---------------------|-----------------|
//         | 1         | 0 bytes             |                 |
//         | 2         | 32 bytes            | 8 bytes         |
//         | 3         | 32 bytes            | 4 bytes         |
//         | 4         | 32 bytes            | 0 bytes         |
//         | 5         | 48 bytes            | 12 bytes        |
//         | 6         | 48 bytes            | 8 bytes         |
//         | 7         | 48 bytes            | 4 bytes         |
//         |---------------------------------------------------|
//
//
// Description of Members
//     u_long  GsMiptbp1;
//         This member maintains data in the same form, as data set in the GS MIPTBP1 register.
//     |-----------------------------------------------------------------------|
//     | Bit     | Field Name    | Use                                         |
//     |---------|---------------|---------------------------------------------|
//     | 63 to 60|               | Reserved (always 0)                         |
//     | 59 to 54|     TBW3      | MIPMAP level 3 texture buffer width         |
//     | 53 to 40|     TBP3      | MIPMAP level 3 texture buffer base point    |
//     | 39 to 34|     TBW2      | MIPMAP level 2 texture buffer width         |
//     | 33 to 20|     TBP2      | MIPMAP level 2 texture buffer base point    |
//     | 19 to 14|     TBW1      | MIPMAP level 1 texture buffer width         |
//     | 13 to 0 |     TBP1      | MIPMAP level 1 texture buffer base point    |
//     |-----------------------------------------------------------------------|
//
//     u_long  GsMiptbp2;
//         This member maintains data in the same form, as data set in the GS MIPTBP2 register.
//     |-----------------------------------------------------------------------|
//     | Bit     | Field Name    | Use                                         |
//     |---------|---------------|---------------------------------------------|
//     | 63 to 60|               | Reserved (always 0)                         |
//     | 59 to 54|     TBW6      | MIPMAP level 6 texture buffer width         |
//     | 53 to 40|     TBP6      | MIPMAP level 6 texture buffer base point    |
//     | 39 to 34|     TBW5      | MIPMAP level 5 texture buffer width         |
//     | 33 to 20|     TBP5      | MIPMAP level 5 texture buffer base point    |
//     | 19 to 14|     TBW4      | MIPMAP level 4 texture buffer width         |
//     | 13 to 0 |     TBP4      | MIPMAP level 4 texture buffer base point    |
//     |-----------------------------------------------------------------------|
//
//     u_int   Size[n];
//         This member indicates the sizes in bytes of the MIPMAP textures as an array. The number of array elements is equal to the number of MIPMAP textures indicated by the MipMapTextures member of the picture header.
//         The first element of the array indicates the size of the LV0 texture, and the total size of the MIPMAP textures obtained by totaling all array elements is equal to the size of the image data part indicated by the ImageSize member of the picture header.
//         The value of each element must be a multiple of 16 to adjust the alignment of the start of the MIPMAP texture data to a 16-byte boundary.
//
//
// 3.4 User Space
//
// The user space is a variable-length data area for which no specific use has been determined. This area can be freely used.
// The user space is located immediately after the MIPMAP header or immediately after the picture header. The size of the user space can be obtained by subtracting the picture header size (always 48 bytes) and MIPMAP header size (varies according to the number of MIPMAP textures) from the entire header size indicated by the HeaderSize member of the picture header.
//
// Note:  Whether or not to save the user space information in advance when editing using a tool that supports TIM2 depends on the tool implementation. Basically, an implementation in which the user space is saved is recommended. Since the HeaderSize member of the picture header is affected by the padding for adjusting the alignment to a 16-byte or 128-byte boundary, the size of the user space may grow and exceed the intended data size when editing operations are repeatedly performed. This problem can be avoided by using the extended header that is described in Section 4.7, "Extended Header." Adding an extended header is recommended in order to clearly indicate the intended size of the user space.
//
// The recommended specification for whether or not to use an extended header when no user space is used differ according to the alignment mode. For 16-byte alignment mode, the value of the HeaderSize member of the picture header is set so that the size of the user space is zero. For 128-byte alignment mode, an extended header indicating that the intended size of the user space is zero is added.
//
//
// 3.5 Image Data
//
// Texture data is maintained in image data in a form so that it can be transferred asis to GS local memory. The image data format is defined by the ImageType member of the picture header. The start of the image data is adjusted so that it is aligned on a 16-byte or 128-byte boundary. When there are two or more MIPMAP textures, the start of each MIPMAP texture is adjusted so that it is aligned on a 16-byte boundary.
//
//
// 3.6 CLUT Data
//
// The format and pixel placement within CLUT data is defined according to the combination of the ClutType and ImageType members of the picture header. For details, see Section 4.4, "Format Specification for the CLUT Data Part" and Section 4.5, "Pixel Arrangement in CLUT Data." The start of CLUT data is adjusted so that it is aligned on a 16-byte or 128-byte boundary.
//
// The data maintained for CLUT data may not be in a form that can be directly transferred to GS local memory. For example, when the CLUT storage mode is CSM2 mode, the GS cannot use PSMCT32 as the pixel format. The pixel placement may need to be converted when the data is transferred to GS local memory.
//
//
// 4. Miscellaneous
// 4.1 General Principles Concerning Alignment Adjustment
//
// A member related to size within the header maintains an alignment-adjusted value. This means that a program that references TIM2-format data can calculate the reference address without having to specifically consider alignment. The alignment is adjusted at the stage when the TIM2-format data is created.
//
//
// 4.2 General Principles Concerning Padding and Reserved Areas
//
// 0x00 should be set for all reserved members and padding that is inserted to adjust the alignment. The reason for this is to guarantee compatibility when the version of the TIM2 format is changed in the future.
//
//
// 4.3 General Principles Concerning GS Registers
//
// Members related to GS registers that are in the picture header and MIPMAP header should be set so that they do not conflict with information indicated by other members.
//
//
// 4.4 Format Specification for the CLUT Data Part
//
// The pixel format of the CLUT data part and pixel arrangement order are determined as shown below according to the combination of the ClutType and ImageType members in the picture header. No other combinations are valid.
//
// |---------------------------------------------------------------------------|
// |   ImageType |    ClutType     | CLUT Data Pixel   | CLUT Storage  | Number|
// |             |                 | Format            | Mode and      | of    |
// |             |                 |                   | Pixel         | CLUT  |
// |             |                 |                   | Arrangement   | Colors|
// |-------------|-----------------|-------------------|---------------|-------|
// | TIM2_RGB16  | TIM2_NONE       |  None             | None          |  None |
// | TIM2_RGB24  | TIM2_NONE       |  None             | None          |  None |
// | TIM2_RGB32  | TIM2_NONE       |  None             | None          |  None |
// | TIM2_IDTEX4 | TIM2_RGB16      |  16 colors, 16-bit| CSM1, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB24      |  16 colors, 24-bit| CSM1, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB32      |  16 colors, 32-bit| CSM1, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB16|0x40 |  16 colors, 16-bit| CSM1, with    | 32n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB24|0x40 |  16 colors, 24-bit| CSM1, with    | 32n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB32|0x40 |  16 colors, 32-bit| CSM1, with    | 32n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB16|0x80 |  16 colors, 16-bit| CSM2, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB24|0x80 |  16 colors, 24-bit| CSM2, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX4 | TIM2_RGB32|0x80 |  16 colors, 32-bit| CSM2, no      | 16n   |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB16      | 256 colors, 16-bit| CSM1, with    | 256n  |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB24      | 256 colors, 24-bit| CSM1, with    | 256n  |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB32      | 256 colors, 32-bit| CSM1, with    | 256n  |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB16|0x80 | 256 colors, 16-bit| CSM2, no      | 256n  |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB24|0x80 | 256 colors, 24-bit| CSM2, no      | 256n  |
// |             |                 |  color            | rearrangement | colors|
// | TIM2_IDTEX8 | TIM2_RGB32|0x80 | 256 colors, 32-bit| CSM2, no      | 256n  |
// |             |                 |  color            | rearrangement | colors|
// |---------------------------------------------------------------------------|
//
// Note that when bit 6 of the ClutType member is set, the number of colors in the CLUT data part is set in units of 32 colors. When the number of CLUTs is odd, a padding CLUT equivalent in size to 16 colors must be added.
//
//
// 4.5 Pixel Arrangement in CLUT Data
//
// When the CLUT storage mode that is indicated by the ClutType member of the picture header is CSM1, the data may be rearranged to match the CLUT pixel layout in GS local memory. For conditions, see Section 4.4, "Format Specification for the CLUT Data Part." For information about CLUT storage modes, refer to the GS manual.
//
//     The CLUT data storage order is shown below.
//     |---------------------------------------------------------------|
//     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 16| 17| 18| 19| 20| 21| 22| 23|
//     |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
//     | 8 | 9 | 10| 11| 12| 13| 14| 15| 24| 25| 26| 27| 28| 29| 30| 31|
//     |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
//     | 32| 33| 34| 35| 36| 37| 38| 39| 48| 49| 50| 51| 52| 53| 54| 55|
//     |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
//     | 40| 41| 42| 43| 44| 45| 46| 47| 56| 57| 58| 59| 60| 61| 62| 63|
//     |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
//     |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
//
// For 256-color CLUTs, the numeric values directly indicate the placement of the color index numbers. For 16-color CLUTs, the numbers indicate the placement spanning multiple CLUTs in units of 16 colors such that 0 to 15 is the first CLUT and 16 to 31 is the second CLUT. For 16-color CLUTs, since the minimum unit is a pair of two CLUTs (32 colors), the number of color elements constituting the CLUT data part will be an integer multiple of 32.
//
// When the CLUT pixel format is RGB16, the units are 64 bytes in size and the 16- to 31-byte area and the 32- to 47-byte area are switched. When the format is RGB24, the units are 128 bytes in size and the 32- to 63-byte area and the 64- to 95-byte area are switched.
//
//
// 4.6 Format of the Image Data Part
//
// The image data part maintains the pixel data in a form that can be transferred asis to GS local memory. For details, refer to the GS manual.
//
//     Image data for TIM2_RGB32
//          One pixel is represented by 32 bits. Each of the elements for R, G, B, and A has a width of 8 bits.
//
//          bit63                 bit32                  bit0
//         |-----------------------------------------------|
//         | A1  | B1  | G1  | R1  | A0  | B0  | G0  | R0  |
//         |-----|-----|-----|-----|-----|-----|-----|-----|
//         | A3  | B3  | G3  | R3  | A2  | B2  | G2  | R2  |
//         |-----------------------------------------------|
//                                 :
//                                 :
//
//     Image data for TIM2_RGB24
//          One pixel is represented by 24 bits. Each of the elements for R, G, and B has a width of 8 bits.
//
//          bit63                 bit32                  bit0
//         |-----------------------------------------------|
//         | G2  | R2  | B1  | G1  | R1  | B0  | G0  | R0  |
//         |-----|-----|-----|-----|-----|-----|-----|-----|
//         | R5  | B4  | G4  | R4  | B3  | G3  | R3  | B2  |
//         |-----|-----|-----|-----|-----|-----|-----|-----|
//         | B7  | G7  | R7  | B6  | G6  | R6  | B5  | G5  |
//         |-----------------------------------------------|
//                                 :
//                                 :
//
//
//     Image data for TIM2_RGB16
//         One pixel is represented by 16 bits. Each of the elements for R, G, and B has a width of 5 bits and the element for A has a width of 1 bit.
//
//         bit63              bit32              bit0
//         |---------------------------------------|
//         | RGBA3   | RGBA2   | RGBA1   | RGBA0   |
//         |---------|---------|---------|---------|
//         | RGBA7   | RGBA6   | RGBA5   | RGBA4   |
//         |---------------------------------------|
//                              :
//                              :
//
//
//     Image data for TIM2_IDTEX8
//         One texel is represented by 8 bits. Each element indicates an index number within the CLUT.
//
//         bit63                  bit32                  bit0
//         |-----------------------------------------------|
//         | T7  | T6  | T5  | T4  | T3  | T2  | T1  | T0  |
//         |-----|-----|-----|-----|-----|-----|-----|-----|
//         | T15 | T14 | T13 | T12 | T11 | T10 | T9  | T8  |
//         |-----------------------------------------------|
//                                 :
//                                 :
//
//     Image data for TIM2_IDTEX4
//         One texel is represented by 4 bits. Each element indicates an index number within the CLUT.
//
//         bit63                        bit32                           bit0
//        |---------------------------------------------------------------|
//        | T | T | T | T | T | T | T | T | T | T | T | T | T | T | T | T |
//        | 1 | 1 | 1 | 1 | 1 | 1 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//        | 5 | 4 | 3 | 2 | 1 | 0 |   |   |   |   |   |   |   |   |   |   |
//        |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
//        | T | T | T | T | T | T | T | T | T | T | T | T | T | T | T | T |
//        | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 |
//        | 1 | 0 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 9 | 8 | 7 | 6 |
//        |---------------------------------------------------------------|
//                                        :
//                                        :
//
//
// 4.7 Extended Header
//
// The extended header is a recommended specification for establishing a standard method of using the user space. Using the extended header enables you to accommodate a character string comment for each picture. It also enables you to avoid the unintentional growth of the user space size due to padding for alignment adjustment.
// Even if no character string comments are required when the user space is used, the addition of an extended header is recommended to explicitly indicate the size of the user space. Also, even if neither user data nor comment character strings are required, the addition of an extended header is recommended when 128-byte alignment mode is used. In this case, the addition of an extended header will not affect the size because padding is inserted immediately after the picture header or MIPMAP header in order to align the start of the image data on a 128-byte boundary.
//
// When an extended header is used, the user space will have the following format.
//                 |---------------------------|
//                 | Extended header           |
//                 |---------------------------|
//                 | User data                 |
//                 |---------------------------|
//                 | Comment character string  |
//                 |---------------------------|
//
//     The user data and/or the comment character string may not exist.
//
//
// Extended header
//     The extended header, which has a fixed length of 16 bytes, is located at the start of the user space. Its structure is shown below.
//
// |-------------------------------------------------------------------------|
// | Offset    | Size  | Member Name   | Use                                 |
// |-----------|-------|---------------|-------------------------------------|
// | +0        | 4     | ExHeaderId    | Extended header ID                  |
// |           |       |               | ("e", "X", "t", "\x00")             |
// | +4        | 4     | UserSpaceSize | Total size of user space            |
// | +8        | 4     | UserDataSize  | Size of user data                   |
// | +12       | 4     | Reserved      | Reserved                            |
// |-------------------------------------------------------------------------|
//
//     typedef struct tagTIM2_EXHEADER {
//             char    ExHeaderId[4];  // Extended header ID
//                                     // Always ("e", "X", "t", "\x00")
//             u_int   UserSpaceSize;  // Total size of user space
//             u_int   UserDataSize;   // Size of user data
//                                     // (pointer up to comment)
//             u_int   Reserved;       // Reserved (Always 0x00000000)
//     } TIM2_EXHEADER;
//
//     Description of Members
//
//     char    ExHeaderId[4];
//         This member is a four-character string for identifying that an extended header exists in the user space. "e", "X", "t", and "\x00" are stored sequentially in the order shown.
//
//     u_int   UserSpaceSize;
//         This member indicates the size in bytes of the entire user space. The value of the UserSpaceSize member includes the 16-byte size of the extended header itself. The size that is indicated by the UserSpaceSize member may differ from the size information that is obtained by subtracting the size of the picture header and the size of the MIPMAP header from the HeaderSize member of the picture header that was described earlier. That is, the UserSpaceSize member indicates the intended size of the user space. It is not affected by padding.
//
//     u_int   UserDataSize;
//         This member indicates the size in bytes of the user data that immediately follows the extended header. This member indicates the offset up to the comment character string. For example, if the user space contains no data other than the extended header, the value of the UserDataSize member will be 0x00000000. When the total of the 16-byte size of the extended header and the size of the user data, which is indicated by the UserDataSize member, equals the intended size of the user space, which is indicated by the UserSpaceSize member, it indicates that there is no character sting comment following the user data.
//
//     u_int   Reserved;
//         This member is a reserved area for expansion. It is always 0x00000000.
//
//     User Data
//     User data having the size in bytes indicated by the UserDataSize member in the extended header is entered immediately after the extended header. The data that is used by a game program is mainly held in this part.
//
//     Comment Character String
//     This area holds a comment character string that is added to the TIM2 data. The comment character string is terminated by a single-byte NULL (0x00). The characters that can be used are ASCII codes 0x20 to 0x7f, namely the alphanumeric characters and symbols. Control codes, kanji, half-width kana, and the like cannot be used.
//
//
// -----------------------------------------------------------
// Revision History
//
// 11/18/1999  Corrected the bit value of GsTex1 in Section
// 	    3.2, "Picture
//             Header."
// 12/02/1999  Corrected the fact that TH and TW in the GS
// 	    TEX0 register had
//             been reversed.
// 02/26/2001  Modified Section 1, "Introduction."
// 	    No change was made to the specification.
//
// ------------------------------------------------------------
// Copyright(C) 1999-2001 web technology Corp.
// All rights reserved.
// http://www.webtech.co.jp/    mailto:tim2@webtech.co.jp
// -------------------------------------------------------------
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "tm2gimex.h"

static __inline int TM2readline(unsigned char *d,const unsigned char *s,int width,int bpp)
{
    int ok=1;
    int i;

    switch (bpp)
    {
        case 4:     /* 4 bit indexed image */
            for (i=0; i<width; ++i)
            {
                if (i&1)
                    *d++ = (s[i/2]>>4)&15;
                else
                    *d++ = s[i/2]&15;
            }
            break;

        case 8:     /* 8 bit indexed image */
            for (i=0; i<width; ++i)
            {
                *d++ = *s++;
            }
            break;


        case 16:    /* 16 bit direct image */
            for (i=0; i<width; ++i)
            {
                ((ARGB *) d)->a = (unsigned char)(((*(s+1)>>7)&1)*255);
                ((ARGB *) d)->b = (unsigned char)(((*(s+1)>>2)&31)*255/31);
                ((ARGB *) d)->g = (unsigned char)((((*(s+1)&3)<<3) | ((*(s+0)>>5)&7))*255/31);
                ((ARGB *) d)->r = (unsigned char)((*s&31)*255/31);
                d += 4;
                s += 2;
            }
            break;

        case 24:    /* 24 bit direct image */
            for (i=0; i<width; ++i)
            {
                ((ARGB *) d)->a = (unsigned char)(255);
                ((ARGB *) d)->r = s[0];
                ((ARGB *) d)->g = s[1];
                ((ARGB *) d)->b = s[2];
                d += 4;
                s += 3;
            }
            break;

        case 32:    /* 32 bit direct image */
            for (i=0; i<width; ++i)
            {
                ((ARGB *) d)->a = s[3];
                ((ARGB *) d)->b = s[2];
                ((ARGB *) d)->g = s[1];
                ((ARGB *) d)->r = s[0];
                d += 4;
                s += 4;
            }
            break;
    }

    return(ok);
}

void TM2_seek(GSTREAM *g, int framenum)
{
    int  frame=0;
    int  ok=1;
   // int keyoff;
    //char *chunk;

    TM2HeaderStruct tm2header;
    int alignment=16;

    gseek(g,(GPOS)0);

    if (gread(g,&tm2header,16))
    {
        int FormatId = TM2GETFIELD(tm2header.FormatId);
        if (FormatId&1) alignment = 128;
    }

    while (frame!=framenum && ok)
    {
        TIM2_PICTUREHEADER tm2info;

        gread(g,&tm2info,sizeof(TIM2_PICTUREHEADER));

        int framesize = TM2GETFIELD(tm2info.TotalSize);

        ok = gseek(g,gtell(g)-sizeof(TIM2_PICTUREHEADER)+framesize);
        ++frame;
    }
}



/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL TM2_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('T','I','M','2');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 0;    /* movie file format */
        info->file64          = 0;    /* this format supports 64 bit file sizes */
        info->mactype[0]      = GMAKEID('T','I','M','2');   /* mac file system types used */
        strcpy(info->extensions[0], ".tm2");                /* extensions */
        strcpy(info->extensions[1], ".tim2");
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
        strcpy(info->shorttypestr,  "TM2");                 /* type */
        strcpy(info->wordtypestr,   "Tim2");                /* wordtype */
        strcpy(info->longtypestr,   "Tim2 Sony Texture");   /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has 4 byte id: TIM2 */

int GCALL TM2_is(GSTREAM *g)
{
    unsigned char tm2header[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,tm2header,4))
    {
        if (ggetm(tm2header,4)==GMAKEID('T','I','M','2'))
            ok = 100;
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL TM2_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    TM2HeaderStruct tm2header;
    int  ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('T','I','M','2');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = 0;  /* keep track of which frame were pointing at */

        gseek(g,(GPOS)0);

        if (gread(g,&tm2header,16))
        {
            ok = gx->frames = TM2GETFIELD(tm2header.Pictures);
        }
    }
    return(ok);
}

GINFO * GCALL TM2_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int i;
    TIM2_PICTUREHEADER tm2info;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        TM2_seek(gx->gstream,framenum);
        gread(gx->gstream,&tm2info,sizeof(TIM2_PICTUREHEADER)/*48*/);

        int width   = TM2GETFIELD(tm2info.ImageWidth);
        int height  = TM2GETFIELD(tm2info.ImageHeight);
        int colours = TM2GETFIELD(tm2info.ClutColors);
        int imagetype = TM2GETFIELD(tm2info.ImageType)&15;
        int cluttype =  TM2GETFIELD(tm2info.ClutType)&15;
        int bpp=16;
        int clutbpp=16;
        int alphabits=1;
        int rgbbits  =5;

        switch (cluttype)
        {
            case 0:
            case 1:
            default: // 0x01 = 16-bit color (TIM2_RGB16)
                clutbpp = 16, alphabits=1, rgbbits=5; break;
            case 2:  // 0x24 = 24-bit color (TIM2_RGB24)
                clutbpp = 24, alphabits=0, rgbbits=8; break;
            case 3:  // 0x03 = 32-bit color (TIM2_RGB32)
                clutbpp = 32, alphabits=8, rgbbits=8; break;
        }

        switch (imagetype&63)
        {
            case 0:
            case 1:
            default: // 0x01 = 16-bit color (TIM2_RGB16)
                bpp = 16, alphabits=1, rgbbits=5; break;
            case 2:  // 0x02 = 24-bit color (TIM2_RGB24)
                bpp = 24, alphabits=0, rgbbits=8; break;
            case 3:  // 0x03 = 32-bit color (TIM2_RGB32)
                bpp = 32, alphabits=8, rgbbits=8; break;
            case 4:  // 0x04 = 4-bit color (TIM2_IDTEX4)
                bpp = 4; break;
            case 5:  // 0x05 = 8-bit color (TIM2_IDTEX8)
                bpp = 8; break;
        }

        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('T','I','M','2');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->width            = width;   /* Width of bitmap in pixels */
        ginfo->height           = height;  /* Height of bitmap in pixels */
        ginfo->bpp              = (bpp<=8)?8:32;    /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = bpp;              /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = colours;  /* Number of colours in original indexed palette */

        ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = alphabits;  /* Number of bits in alpha channel */
        ginfo->redbits          = rgbbits;    /* Number of bits in red channel */
        ginfo->greenbits        = rgbbits;    /* Number of bits in green channel */
        ginfo->bluebits         = rgbbits;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

        if (colours)
        {
            int clutoffset = TM2GETFIELD(tm2info.HeaderSize) + TM2GETFIELD(tm2info.ImageSize);
            int clutsize   = TM2GETFIELD(tm2info.ClutSize);

            gseek(gx->gstream,gtell(gx->gstream)+clutoffset-sizeof(TIM2_PICTUREHEADER)/*48*/);
            void *clutbuf = galloc(clutsize);
            gread(gx->gstream,clutbuf,clutsize);

            TM2readline((unsigned char *)&ginfo->colourtbl[0],(const unsigned char *)clutbuf,colours,clutbpp);

            int csm2 =  TM2GETFIELD(tm2info.ClutType)&0x80;
            if (!csm2 && bpp==8)
            {
                for (i=0; i<256; ++i)
                {
                    int j = i;
                    if ((j&0x18)==0x08)  // swap 8..15 range with 16..23
                    {
                        j ^= 0x18;
                        ARGB t = ginfo->colourtbl[i];
                        ginfo->colourtbl[i] = ginfo->colourtbl[j];
                        ginfo->colourtbl[j] = t;
                    }
                }
            }

            gfree(clutbuf);

        }
    }

    return(ginfo);
}

int GCALL TM2_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int ok=1;
    TIM2_PICTUREHEADER tm2info;

    TM2_seek(gx->gstream,ginfo->framenum);
    gread(gx->gstream,&tm2info,sizeof(TIM2_PICTUREHEADER));

    int imageoffset = TM2GETFIELD(tm2info.HeaderSize);

    gseek(gx->gstream,gtell(gx->gstream)+imageoffset-sizeof(TIM2_PICTUREHEADER)/*48*/);


    int width = ginfo->width;
    int height = ginfo->height;
    int bpp = ginfo->originalbpp;
    int bpl = ginfo->width*bpp/8;

    unsigned char *linebuf = (unsigned char *) galloc(bpl);

    if (linebuf)
    {
        unsigned char *d = (unsigned char *) dest;
        for (y=0; y<height && ok; ++y)
        {
            ok = gread(gx->gstream,linebuf,bpl);

            TM2readline(d,linebuf,width,bpp);
            d += rowbytes;
        }
        gfree(linebuf);
    }
    return(ok);
}

int GCALL TM2_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
//        if (gx->gref)
//            gfree(gx->gref);

        ok = gfree((void *) gx);
    }
    return(ok);
}

