@SET "compileshadersEXECDIR=%CD%"

@REM move to where this file is located
@CD %~dp0

@SET "compileshadersTEMPDIR=..\..\temp\"

g++ shaders_build_main.cpp -std=c++20 -o %compileshadersTEMPDIR%shaders_build_main.exe
%compileshadersTEMPDIR%shaders_build_main.exe

@REM return to the execution directory
@CD %compileshadersEXECDIR%