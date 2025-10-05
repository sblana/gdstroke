@SET "compileshadersEXECDIR=%CD%"

@REM move to where this file is located
@CD %~dp0

@SET "compileshadersTEMPDIR=..\..\temp\"
@SET "compileshadersEMBED=%compileshadersTEMPDIR%embed.exe"
@SET "compileshadersOUTPUTDIR=..\gen\"
@REM @ECHO ON
glslang -I. -DGODOT_VERSION_MINOR=5 --target-env vulkan1.3 -V .\test\buffer_sizes.comp -o %compileshadersTEMPDIR%test_buffer_sizes.spv
gcc embed.c -o %compileshadersEMBED%
%compileshadersEMBED% %compileshadersTEMPDIR%test_buffer_sizes.spv %compileshadersOUTPUTDIR%test_buffer_sizes.spv.h SHADER_SPV_test_buffer_sizes
@REM @ECHO OFF

@REM return to the execution directory
@CD %compileshadersEXECDIR%