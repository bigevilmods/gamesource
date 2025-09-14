cd Data-TemporaryBag-0.08
attrib -r *.* /s 
perl Makefile.pl 
"c:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\nmake.exe"
"c:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\nmake.exe" install
cd ..

cd SWF-File-0.36
attrib -r *.* /s 
perl Makefile.pl 
"c:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\nmake.exe"
"c:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\nmake.exe" install
cd ..
