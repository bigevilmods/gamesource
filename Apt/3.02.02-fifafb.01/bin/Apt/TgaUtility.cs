/*H************************************************************************************************/
/*!

    \file    TgaUtility.cs

    \brief   Performs low-level image file operations needed by the Apt pipeline.

    (c) 2009 Electronic Arts Inc.

    \version    1.00.00    08/27/09 (RB)  First revision
    \version    1.00.01    09/02/09 (RB)  Added using() to let Bitmaps be disposed earlier.
    \version    1.00.02    09/10/09 (RB)  Check for 8-bit bpp in CheckTgaSanity.
    \version    1.01.00    09/10/09 (RB)  Added -combinealpha_unpremultiply
    \version    1.01.01    11/10/09 (RB)  Fixed bug loading 8-bit paletted TGA files
*/
/************************************************************************************************H*/

using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;

namespace EA {
    namespace Apt {
        namespace Pipeline {

            /// <summary>
            /// A utility class that performs low-level image file operations needed by the Apt pipeline.
            /// </summary>
            static class TgaUtility
            {
                private static readonly string MYNAME = "tgautility";           // name to display in PrintUsage
                private static readonly string VERSION = "1.01.01";             // version number
                private static readonly string ERR_PREFIX = MYNAME;             // prefix for error messages
                private static readonly string UPSIZE = "-upsize";              // upsize argument name
                private static readonly string DOWNSIZE = "-downsize";          // downsize argument name
                private static readonly string COMBINEALPHA = "-combinealpha";  // combinealpha argument name
                private static readonly string COMBINEALPHA_DIV = "-combinealpha_unpremultiply";  // combinealpha_div argument name
                private static readonly int    MAX_TRIES = 100;                 // max. number of tries for failsafe operations


                /// <summary>
                /// Main entry point into the program
                /// </summary>
                /// <param name="args">Command line arguments</param>
                /// <returns>0 on success, nonzero on failure</returns>
                public static int Main(string[] args)
                {
                    try
                    {
                        if (args.Length < 1)
                        {
                            throw new Exception("You must specify one of the options -upsize, -downsize or -combinealpha.");
                        }
                        if (args[0] == UPSIZE || args[0] == DOWNSIZE)
                        {
                            if (args.Length != 5)
                            {
                                throw new Exception("Wrong number of arguments");
                            }
                            int width = Convert.ToInt32(args[1]);
                            int height = Convert.ToInt32(args[2]);
                            if (args[0] == UPSIZE)
                            {
                                UpSizeAndConvertToTga(args[3], args[4], width, height);
                            }
                            else
                            {
                                DownSizeAndConvertToTga(args[3], args[4], width, height);
                            }
                        }
                        if (args[0] == COMBINEALPHA || args[0] == COMBINEALPHA_DIV)
                        {
                            if (args.Length != 4)
                            {
                                throw new Exception("Wrong number of arguments for " + COMBINEALPHA);
                            }
                            ConvertJpgWithAlphaToTga(args[2], args[1], args[3], (args[0] == COMBINEALPHA_DIV));
                        }
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.Message);
                        PrintUsage();
                        return 1;
                    }
                    return 0;
                }


                /// <summary>
                /// Upsizes an image to the given width and height, and converts it to a 32-bit unpaletted TGA file.
                /// The upsize operation extrudes (clamps) pixels to the edge.
                /// </summary>
                /// <param name="inputFileName">input filename; can be JPG or TGA</param>
                /// <param name="outputFileName">output filename; should be a .tga file</param>
                /// <param name="width">new width; should be no smaller than input file's width</param>
                /// <param name="height">new height; should be no smaller than input file's height</param>
                public static void UpSizeAndConvertToTga(string inputFileName, string outputFileName, int width, int height)
                {
                    using (Bitmap inputBitmap = LoadImage(inputFileName))
                    {
                        if (inputBitmap.Width > width && inputBitmap.Height > height)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + inputFileName + " is already larger than " + width + "x" + height);
                        }
                        else if (inputBitmap.Width == width && inputBitmap.Height == height)
                        {
                            SaveTga(inputBitmap, outputFileName);
                            return;
                        }

                        using (Bitmap outputBitmap = new Bitmap(width, height, PixelFormat.Format32bppArgb))
                        {
                            // copy input image into output image, and extrude (clamp) to edge
                            BitmapData inData = inputBitmap.LockBits(new Rectangle(0, 0, inputBitmap.Width, inputBitmap.Height), ImageLockMode.ReadOnly, inputBitmap.PixelFormat);
                            BitmapData outData = outputBitmap.LockBits(new Rectangle(0, 0, outputBitmap.Width, outputBitmap.Height), ImageLockMode.ReadWrite, outputBitmap.PixelFormat);

                            byte[] outBytes = new byte[outData.Stride];

                            for (int j = 0; j < outputBitmap.Height; ++j)
                            {
                                // for each row

                                if (j < inputBitmap.Height)
                                {
                                    // copy into outBytes
                                    IntPtr inPtr = new IntPtr(inData.Scan0.ToInt32() + (j * inData.Stride));
                                    System.Runtime.InteropServices.Marshal.Copy(inPtr, outBytes, 0, inData.Stride);

                                    // extrude horizontally
                                    for (int i = inData.Stride; i < outData.Stride; i += 4)
                                    {
                                        outBytes[i + 0] = outBytes[i - 4];
                                        outBytes[i + 1] = outBytes[i - 3];
                                        outBytes[i + 2] = outBytes[i - 2];
                                        outBytes[i + 3] = outBytes[i - 1];
                                    }
                                }

                                // copy from managed array outBytes to unmanaged pointer outPtr
                                IntPtr outPtr = new IntPtr(outData.Scan0.ToInt32() + (j * outData.Stride));
                                System.Runtime.InteropServices.Marshal.Copy(outBytes, 0, outPtr, outData.Stride);
                            }

                            outputBitmap.UnlockBits(outData);
                            inputBitmap.UnlockBits(inData);

                            SaveTga(outputBitmap, outputFileName);
                        }
                    }
                }

                /// <summary>
                /// Downsizes an image to the given width and height, and converts it to a 32-bit unpaletted TGA file.
                /// </summary>
                /// <param name="inputFileName">input filename; can be JPG or TGA</param>
                /// <param name="outputFileName">>output filename; should be a .tga file</param>
                /// <param name="width">new width; should be no larger than input file's width</param>
                /// <param name="height">new height; should be no larger than input file's height</param>
                public static void DownSizeAndConvertToTga(string inputFileName, string outputFileName, int width, int height)
                {
                    using (Bitmap inputBitmap = LoadImage(inputFileName))
                    {
                        if (inputBitmap.Width < width && inputBitmap.Height < height)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + inputFileName + " is already smaller than " + width + "x" + height);
                        }
                        using (Bitmap outputBitmap = new Bitmap(inputBitmap, width, height))
                        {
                            SaveTga(outputBitmap, outputFileName);
                        }
                    }
                }


                /// <summary>
                /// Takes a JPG and an 8-bit TGA and combines them into a 32-bit unpaletted TGA with alpha.
                /// </summary>
                /// <param name="jpgFileName">JPG file; 24-bit RGB color input</param>
                /// <param name="tgaFileName">8-bit TGA file; taken as alpha channel</param>
                /// <param name="outputFileName">output filename; should be a .tga</param>
                public static void ConvertJpgWithAlphaToTga(string jpgFileName, string tgaFileName, string outputFileName, bool divideByAlpha)
                {
                    if ( Path.GetExtension(jpgFileName.ToLower()) != ".jpg" || Path.GetExtension(tgaFileName.ToLower()) != ".tga" )
                    {
                        throw new Exception( ERR_PREFIX + "Expected " + jpgFileName + " to be a .jpg file, and " + tgaFileName + " to be a .tga.");
                    }

                    using (Bitmap jpgBitmap = CreateBitmap(jpgFileName))
                    {
                        using (Bitmap tgaBitmap = LoadTga(tgaFileName))
                        {

                            if (jpgBitmap.Width != tgaBitmap.Width || jpgBitmap.Height != tgaBitmap.Height)
                            {
                                throw new Exception(ERR_PREFIX + "Expected " + jpgFileName + " to be the same width and height as " + tgaFileName);
                            }

                            using (Bitmap outputBitmap = Convert24To32BitImage(jpgBitmap, tgaBitmap, divideByAlpha))
                            {
                                SaveTga(outputBitmap, outputFileName);
                            }
                        }
                    }
                }

                /// <summary>
                /// Prints command-line arguments
                /// </summary>
                private static void PrintUsage()
                {
                    Console.WriteLine(MYNAME + " " + VERSION + ": A TGA processing utility for the Apt pipeline");
                    Console.WriteLine("(c) 2009 Electronic Arts");
                    Console.WriteLine();
                    Console.WriteLine("Usage:");
                    Console.WriteLine(MYNAME + " <options> <inputfilename> <outputfilename>");
                    Console.WriteLine("options:");
                    Console.WriteLine("  " + UPSIZE + " <width> <height>       : Upsize image to be size <width> and <height>.");
                    Console.WriteLine("  " + DOWNSIZE + " <width> <height>     : Downsize image to be size <width> and <height>.");
                    Console.WriteLine("  " + COMBINEALPHA + " <alphafile>      : Combine <alphafile> with <inputfilename>, where");
                    Console.WriteLine("                                   <alphafile> is an 8-bit TGA file that will be");
                    Console.WriteLine("                                   used as the alpha channel of <inputfilename>,");
                    Console.WriteLine("                                   and <inputfilename> is a 24-bit JPG file.");
                    Console.WriteLine("  " + COMBINEALPHA_DIV + " <alphafile>  : Same as " + COMBINEALPHA + " except RGB is multiplied ");
                    Console.WriteLine("                                   by 255 and divided by alpha for each pixel.");
                    Console.WriteLine("inputfilename:");
                    Console.WriteLine("  Input file; a JPG or TGA file. For TGA, it can be 8-bit paletted or 32-bit");
                    Console.WriteLine("  unpaletted, and must be uncompressed, not RLE.");
                    Console.WriteLine("outputfilename:");
                    Console.WriteLine("  Output file. Will be an uncompressed, 32-bit unpaletted TGA file.");

                }

                /// <summary>
                /// Saves a Bitmap as a 32-bit unpaletted, uncompressed TGA file
                /// </summary>
                /// <param name="outputBitmap">bitmap to output</param>
                /// <param name="fileName">output filename</param>
                private static void SaveTga(Bitmap outputBitmap, string fileName)
                {
                    if (Path.GetExtension(fileName) != ".tga")
                    {
                        throw new Exception(ERR_PREFIX + "Output filename " + fileName + " must have '.tga' extension.");
                    }

                    if (outputBitmap.PixelFormat != PixelFormat.Format32bppArgb)
                    {
                        throw new Exception( ERR_PREFIX + "Output image " + fileName + " has invalid pixel format " + outputBitmap.PixelFormat.ToString());
                    }

                    using (BinaryWriter writer = CreateBinaryWriter(fileName))
                    {
                        WriteTgaHeader(writer, outputBitmap.Width, outputBitmap.Height);

                        BitmapData outData = outputBitmap.LockBits(new Rectangle(0, 0, outputBitmap.Width, outputBitmap.Height), ImageLockMode.ReadOnly, outputBitmap.PixelFormat);
                        int size = outData.Stride * outputBitmap.Height;
                        byte[] imageData = new byte[size];
                        IntPtr ptr = outData.Scan0;
                        System.Runtime.InteropServices.Marshal.Copy(ptr, imageData, 0, size);
                        outputBitmap.UnlockBits(outData);
                        writer.Write(imageData);
                        writer.Close();
                    }
                }

                /// <summary>
                /// Loads an image file and returns a 32-bit ARGB format Bitmap
                /// </summary>
                /// <param name="fileName">Either a TGA file (8-bit indexed, or 24-bit RGB, or 32-bit ARGB) or a JPG file (24- or 32-bit)</param>
                /// <returns>Bitmap that has 32-bit ARGB pixel format</returns>
                private static Bitmap LoadImage(string fileName)
                {
                    string ext = Path.GetExtension(fileName.ToLower());
                    if (ext == ".tga")
                    {
                        return LoadTga(fileName);
                    }

                    Bitmap inputBitmap = CreateBitmap(fileName);
                    if (inputBitmap.PixelFormat == PixelFormat.Format32bppArgb)
                    {
                        return inputBitmap;
                    }
                    else if (inputBitmap.PixelFormat == PixelFormat.Format24bppRgb)
                    {
                        return Convert24To32BitImage(inputBitmap);
                    }

                    throw new Exception( ERR_PREFIX + "Input image " + fileName + " has invalid pixel format " + inputBitmap.PixelFormat.ToString());
                }

                /// <summary>
                /// Loads a file as a TGA
                /// </summary>
                /// <param name="fileName">TGA filename; can be 8-bit paletted or 32-bit unpaletted; must be uncompressed (not RLE)</param>
                /// <returns>bew Bitmap containing contents of given file</returns>
                private static Bitmap LoadTga(string fileName)
                {
                    Bitmap outputBitmap = null;

                    byte[] imageData = null;
                    int width = 0;
                    int height = 0;
                    int imageDescriptor = 0;

                    using (BinaryReader reader = CreateBinaryReader(fileName))
                    {
                        int idSize = Convert.ToInt32( reader.ReadByte() );           // num chars in id field
                        bool isPaletted = Convert.ToBoolean( reader.ReadByte() );    // colour map type 0 no palette or colour map type 1 paletted
                        int imageType = Convert.ToInt32(reader.ReadByte());          // image type 1: uncompressed colour mapped image or 2: rgb image or 3: grey scale
                        int colorMapStart = Convert.ToInt32( reader.ReadUInt16() );  // base of colour map indices
                        int colorMapLength = Convert.ToInt32( reader.ReadUInt16() ); // length of colour map indices
                        int bppPalette = Convert.ToInt32( reader.ReadByte() );       // bpp for colour map entries
                        int xOrigin = Convert.ToInt32( reader.ReadUInt16() );        // x origin
                        int yOrigin = Convert.ToInt32( reader.ReadUInt16() );        // y origin
                        width = Convert.ToInt32( reader.ReadUInt16() );          // width
                        height = Convert.ToInt32( reader.ReadUInt16() );         // height
                        int bppPixel = Convert.ToInt32( reader.ReadByte() );         // bpp of pixel format
                        imageDescriptor = Convert.ToInt32( reader.ReadByte() );  // image descriptor byte (32 == origin in top left)

                        CheckTgaSanity(fileName, isPaletted, imageType, colorMapStart, bppPalette, xOrigin, yOrigin, bppPixel, imageDescriptor);

                        reader.ReadBytes(idSize);

                        if (!isPaletted)
                        {
                            imageData = reader.ReadBytes(width * height * 4);
                        }
                        else
                        {
                            byte[] palette = reader.ReadBytes(colorMapLength * 4);
                            byte[] indices = reader.ReadBytes(width * height);
                            imageData = new byte[width * height * 4];
                            for( int i=0; i<width*height; ++i )
                            {
                                int index = Convert.ToInt32(indices[i]);

                                imageData[i * 4 + 0] = palette[index * 4 + 0];
                                imageData[i * 4 + 1] = palette[index * 4 + 1];
                                imageData[i * 4 + 2] = palette[index * 4 + 2];
                                imageData[i * 4 + 3] = palette[index * 4 + 3];
                            }
                        }
                    }

                    outputBitmap = new Bitmap( width, height, PixelFormat.Format32bppArgb );
                    BitmapData outData = outputBitmap.LockBits( new Rectangle(0,0,width,height), ImageLockMode.ReadWrite, outputBitmap.PixelFormat );


                    // copy from managed array to unmanaged pointer

                    if (imageDescriptor == 32)
                    {
                        IntPtr ptr = outData.Scan0;
                        System.Runtime.InteropServices.Marshal.Copy(imageData, 0, ptr, outData.Stride * outputBitmap.Height);
                    }
                    else if (imageDescriptor == 8)
                    {
                        // image is upside down
                        for (int j = 0; j < outputBitmap.Height; ++j)
                        {
                            IntPtr ptr = new IntPtr(outData.Scan0.ToInt32() + (outputBitmap.Height - j - 1) * outData.Stride);
                            System.Runtime.InteropServices.Marshal.Copy(imageData, j * outData.Stride, ptr, outData.Stride);
                        }
                    }

                    outputBitmap.UnlockBits(outData);
                    return outputBitmap;
                }

                /// <summary>
                /// Checks that the given file is the kind of TGA file that we know how to process.
                /// </summary>
                /// <param name="fileName">TGA filename</param>
                /// <param name="isPaletted">true if image is paletted</param>
                /// <param name="imageType">image type byte from TGA header</param>
                /// <param name="colorMapStart">index at which palette starts</param>
                /// <param name="bppPalette">bits per pixel in palette data</param>
                /// <param name="xOrigin">x origin</param>
                /// <param name="yOrigin">y origin</param>
                /// <param name="bppPixel">bits per pixel in image data</param>
                /// <param name="imageDescriptor">image descriptor byte from TGA header</param>
                private static void CheckTgaSanity(string fileName, bool isPaletted, int imageType, int colorMapStart, int bppPalette, int xOrigin, int yOrigin, int  bppPixel, int imageDescriptor)
                {
                    if (imageDescriptor != 32 && imageDescriptor != 8)
                    {
                        throw new Exception(ERR_PREFIX + "Input image " + fileName + " has image descriptor byte = " + imageDescriptor + " instead of 8 or 32");
                    }

                    if ( xOrigin != 0 || yOrigin != 0 )
                    {
                        throw new Exception(ERR_PREFIX + "Input image " + fileName + " has nonzero xorigin or yorigin");
                    }
                    if (imageType < 1 || imageType > 2)
                    {
                        throw new Exception(ERR_PREFIX + "Input image " + fileName + " has image type byte = " + imageType + " instead of 1 or 2");
                    }
                    if (bppPixel != 8 && bppPixel != 32)
                    {
                        throw new Exception(ERR_PREFIX + "Input image " + fileName + " has bpp = " + bppPixel + " instead of 8 or 32");
                    }

                    if (!isPaletted)
                    {
                        if (imageType != 2)
                        {
                            throw new Exception( ERR_PREFIX + "Input image " + fileName + ": TGA image type '" + imageType + "' doesn't match palette flag '" + isPaletted + "'");
                        }
                        if (bppPixel != 32)
                        {
                            throw new Exception( ERR_PREFIX + "Input image " + fileName + ": Non-paletted TGA image must be 32 bpp.");
                        }
                    }
                    else
                    {
                        if (imageType != 1)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + fileName + ": TGA image type '" + imageType + "' doesn't match palette flag '" + isPaletted + "'");
                        }
                        if (bppPixel != 8)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + fileName + ": Paletted TGA image must be have 8-bit pixel data, not " + bppPixel);
                        }
                        if (bppPalette != 32)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + fileName + ": Paletted TGA image must be have 32-bit palette data, not " + bppPalette);
                        }

                        if (colorMapStart != 0)
                        {
                            throw new Exception(ERR_PREFIX + "Input image " + fileName + ": Paletted TGA image must start at palette index 0, not " + colorMapStart);
                        }
                    }
                }


                /// <summary>
                /// Writes a TGA header block to the given BinaryWriter
                /// </summary>
                /// <param name="writer">BinaryWriter to write to</param>
                /// <param name="width">image width</param>
                /// <param name="height">image height</param>
                private static void WriteTgaHeader(BinaryWriter writer, int width, int height)
                {
                    writer.Write((byte)0);        // num chars in id field
                    writer.Write((byte)0);        // colour map type 0 no palette or colour map type 1 paletted
                    writer.Write((byte)2);        // image type 1: uncompressed colour mapped image or 2: rgb image or 3: grey scale
                    writer.Write((ushort)0);      // base of colour map indices
                    writer.Write((ushort)0);      // length of colour map indices
                    writer.Write((byte)0);        // bpp for colour map entries
                    writer.Write((ushort)0);      // x origin
                    writer.Write((ushort)0);      // y origin
                    writer.Write((ushort)width);  // width
                    writer.Write((ushort)height); // height
                    writer.Write((byte)32);       // bpp of pixel format
                    writer.Write((byte)32);       // image descriptor byte (32 == origin in top left)
                }


                /// <summary>
                /// Converts given Bitmap to a 32-bit Bitmap
                /// </summary>
                /// <param name="inputBitmap">Bitmap to convert</param>
                /// <returns>new 32-bit Bitmap</returns>
                private static Bitmap Convert24To32BitImage(Bitmap inputBitmap)
                {
                    int width = inputBitmap.Width;
                    int height = inputBitmap.Height;

                    Bitmap outputBitmap = new Bitmap(width, height, PixelFormat.Format32bppArgb);

                    BitmapData inData = inputBitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadOnly, inputBitmap.PixelFormat);
                    BitmapData outData = outputBitmap.LockBits(new Rectangle(0, 0, outputBitmap.Width, outputBitmap.Height), ImageLockMode.WriteOnly, outputBitmap.PixelFormat);

                    byte[] inBytes = new byte[inData.Stride];
                    byte[] outBytes = new byte[outData.Stride];

                    for (int j = 0; j < height; ++j)
                    {
                        // for each row

                        // copy from unmanaged pointer inPtr to managed array inBytes
                        IntPtr inPtr = new IntPtr(inData.Scan0.ToInt32() + (j * inData.Stride));
                        System.Runtime.InteropServices.Marshal.Copy(inPtr, inBytes, 0, inData.Stride);

                        // take inBytes and convert each pixel from RGB to ARGB, and write result to outBytes
                        for (int i = 0; i < width; ++i)
                        {
                            outBytes[i * 4 + 0] = inBytes[i * 3 + 0];
                            outBytes[i * 4 + 1] = inBytes[i * 3 + 1];
                            outBytes[i * 4 + 2] = inBytes[i * 3 + 2];
                            outBytes[i * 4 + 3] = 255;
                        }

                        // copy from managed array outBytes to unmanaged pointer outPtr
                        IntPtr outPtr = new IntPtr(outData.Scan0.ToInt32() + (j * outData.Stride));
                        System.Runtime.InteropServices.Marshal.Copy(outBytes, 0, outPtr, outData.Stride);
                    }

                    outputBitmap.UnlockBits(outData);
                    inputBitmap.UnlockBits(inData);

                    return outputBitmap;
                }


                /// <summary>
                /// Combines a 24-bit Bitmap and a 32-bit alpha Bitmap into a new 32-bit Bitmap.
                /// Uses the alpha bitmap as the alpha channel of the new Bitmap.
                /// </summary>
                /// <param name="rgbBitmap">24-bit Bitmap</param>
                /// <param name="alphaBitmap">32-bit greyscale alpha Bitmap</param>
                /// <returns>new 32-bit Bitmap</returns>
                private static Bitmap Convert24To32BitImage(Bitmap rgbBitmap, Bitmap alphaBitmap, bool divideByAlpha)
                {
                    Bitmap outputBitmap = new Bitmap(rgbBitmap.Width, rgbBitmap.Height, PixelFormat.Format32bppArgb);

                    BitmapData rgbData = rgbBitmap.LockBits(new Rectangle(0, 0, rgbBitmap.Width, rgbBitmap.Height), ImageLockMode.ReadOnly, rgbBitmap.PixelFormat);
                    BitmapData alphaData = alphaBitmap.LockBits(new Rectangle(0, 0, alphaBitmap.Width, alphaBitmap.Height), ImageLockMode.ReadOnly, alphaBitmap.PixelFormat);
                    BitmapData outData = outputBitmap.LockBits(new Rectangle(0, 0, outputBitmap.Width, outputBitmap.Height), ImageLockMode.ReadWrite, outputBitmap.PixelFormat);

                    byte[] rgbBytes = new byte[rgbData.Stride];
                    byte[] alphaBytes = new byte[alphaData.Stride];
                    byte[] outBytes = new byte[outData.Stride];

                    for (int j = 0; j < outputBitmap.Height; ++j)
                    {
                        // for each row

                        // copy from unmanaged pointer inPtr to managed array inBytes
                        IntPtr rgbPtr = new IntPtr(rgbData.Scan0.ToInt32() + (j * rgbData.Stride));
                        System.Runtime.InteropServices.Marshal.Copy(rgbPtr, rgbBytes, 0, rgbData.Stride);

                        IntPtr alphaPtr = new IntPtr(alphaData.Scan0.ToInt32() + (j * alphaData.Stride));
                        System.Runtime.InteropServices.Marshal.Copy(alphaPtr, alphaBytes, 0, alphaData.Stride);

                        // take rgbBytes and convert each pixel from RGB to ARGB, and write result to outBytes

                        if (divideByAlpha)
                        {
                            // undo the premultiply-alpha operation that Flash
                            // performed when the image was added to the fla.
                            // see: http://www.quasimondo.com/archives/000665.php
                            //      http://home.comcast.net/~tom_forsyth/blog.wiki.html

                            for (int i = 0; i < rgbBitmap.Width; ++i)
                            {
                                byte alphaByte = alphaBytes[i * 4 + 0];
                                if (alphaByte == 0)
                                {
                                    alphaByte = 1;
                                }
                                outBytes[i * 4 + 0] = (byte) Math.Round(Math.Min(rgbBytes[i * 3 + 0] * 255.0f / alphaByte, 255.0f));
                                outBytes[i * 4 + 1] = (byte) Math.Round(Math.Min(rgbBytes[i * 3 + 1] * 255.0f / alphaByte, 255.0f));
                                outBytes[i * 4 + 2] = (byte) Math.Round(Math.Min(rgbBytes[i * 3 + 2] * 255.0f / alphaByte, 255.0f));
                                outBytes[i * 4 + 3] = alphaBytes[i * 4 + 0];
                            }
                        }
                        else
                        {
                            for (int i = 0; i < rgbBitmap.Width; ++i)
                            {
                                outBytes[i * 4 + 0] = rgbBytes[i * 3 + 0];
                                outBytes[i * 4 + 1] = rgbBytes[i * 3 + 1];
                                outBytes[i * 4 + 2] = rgbBytes[i * 3 + 2];
                                outBytes[i * 4 + 3] = alphaBytes[i * 4 + 0];
                            }
                        }
                        // copy from managed array outBytes to unmanaged pointer outPtr
                        IntPtr outPtr = new IntPtr(outData.Scan0.ToInt32() + (j * outData.Stride));
                        System.Runtime.InteropServices.Marshal.Copy(outBytes, 0, outPtr, outData.Stride);
                    }

                    outputBitmap.UnlockBits(outData);
                    alphaBitmap.UnlockBits(alphaData);
                    rgbBitmap.UnlockBits(rgbData);

                    return outputBitmap;
                }


                /// <summary>
                /// Delay a short time (before retrying the operation).
                /// </summary>
                private static void Delay()
                {
                    Random generator = new Random();
                    int time = generator.Next(100, 500); //Generate a random number between 100 and 500.
                    System.Threading.Thread.Sleep(time); //Sleep for that many milliseconds.
                }

                /// <summary>
                /// Fail-safe BinaryReader factory method
                /// </summary>
                /// <param name="fileName">file to open for reading</param>
                /// <returns>new BinaryReader, if successful</returns>
                private static BinaryReader CreateBinaryReader(string fileName)
                {
                    Exception lastException = null;
                    for (int i = 0; i < MAX_TRIES; ++i)
                    {
                        try
                        {
                            return new BinaryReader(new FileStream(fileName, FileMode.Open, FileAccess.Read));
                        }
                        catch (Exception e)
                        {
                            lastException = e;
                            Delay();
                        }
                    }
                    throw new Exception(ERR_PREFIX + "CreateBinaryReader() failed: filename=" + fileName + "\nPrevious exception was: " + lastException.Message, lastException);
                }


                /// <summary>
                /// Fail-safe BinaryWriter factory method
                /// </summary>
                /// <param name="fileName">file to open for writing</param>
                /// <returns>new BinaryWriter, if successful</returns>
                private static BinaryWriter CreateBinaryWriter(string fileName)
                {
                    Exception lastException = null;
                    for (int i = 0; i < MAX_TRIES; ++i)
                    {
                        try
                        {
                            return new BinaryWriter(new FileStream(fileName, FileMode.Create));
                        }
                        catch (Exception e)
                        {
                            lastException = e;
                            Delay();
                        }
                    }
                    throw new Exception(ERR_PREFIX + "CreateBinaryWriter() failed: filename=" + fileName + "\nPrevious exception was: " + lastException.Message, lastException);
                }


                /// <summary>
                /// Fail-safe Bitmap factory method
                /// </summary>
                /// <param name="fileName">Image file to open</param>
                /// <returns>new Bitmap, on success</returns>
                private static Bitmap CreateBitmap(string fileName)
                {
                    Exception lastException = null;
                    for (int i = 0; i < MAX_TRIES; ++i)
                    {
                        try
                        {
                            return new Bitmap(fileName);
                        }
                        catch (Exception e)
                        {
                            lastException = e;
                            Delay();
                        }
                    }
                    throw new Exception(ERR_PREFIX + "CreateBitmap() failed: filename=" + fileName + "\nPrevious exception was: " + lastException.Message, lastException);
                }
            }
        }
    }
}
