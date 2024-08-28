call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cl /DSIMD_DEBUG=0 /TC /Fe"array_sum.exe" /O2 /Oi /arch:AVX2 src/array_sum.c 
REM cl /DSIMD_DEBUG=0 /TC /Fe"array_reduce.exe" /O2 /Oi /arch:AVX2 src/array_reduce.c 

array_sum.exe
REM array_reduce.exe
