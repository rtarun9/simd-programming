call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cl /DSIMD_DEBUG=0 /TC /Fe"array_sum.exe" /O2   src/array_sum.c 
array_sum.exe
