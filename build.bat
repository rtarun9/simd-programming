call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM cl /DSIMD_DEBUG=0 /TC /Fe"array_sum.exe" /O2 /Oi /arch:AVX2 src/array_sum.c 
cl /DSIMD_DEBUG=0 /TC /Fe"array_reduce.exe" /O2 /Oi /Zi /arch:AVX2 src/array_reduce.c 

REM array_sum.exe
array_reduce.exe
