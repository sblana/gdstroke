@SET "compileshadersEXECDIR=%CD%"

@REM move to where this file is located
@CD %~dp0

@SET "compileshadersTEMPDIR=..\..\temp\"
@SET "compileshadersEMBED=%compileshadersTEMPDIR%embed.exe"
@SET "compileshadersOUTPUTDIR=..\gen\"

@SET "compileshadersGODOTMINORVERSION=-DGODOT_VERSION_MINOR=5"
@SET "compileshadersGLSLANGFLAGS=-gV -I. --target-env vulkan1.3"
@ECHO OFF
glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\test\dummy.comp                                                    -o %compileshadersTEMPDIR%dummy.spv
glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\face_orientation.comp -o %compileshadersTEMPDIR%cr__ced__face_orientation.spv
glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\detection.comp        -o %compileshadersTEMPDIR%cr__ced__detection.spv
glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\allocation.comp       -o %compileshadersTEMPDIR%cr__ced__allocation.spv
glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\scatter.comp          -o %compileshadersTEMPDIR%cr__ced__scatter.spv
gcc embed.c -o %compileshadersEMBED%
%compileshadersEMBED% %compileshadersTEMPDIR%dummy.spv               %compileshadersOUTPUTDIR%dummy.spv.h               SHADER_SPV_dummy
%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__face_orientation.spv %compileshadersOUTPUTDIR%cr__ced__face_orientation.spv.h SHADER_SPV_cr__ced__face_orientation
%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__detection.spv        %compileshadersOUTPUTDIR%cr__ced__detection.spv.h        SHADER_SPV_cr__ced__detection
%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__allocation.spv       %compileshadersOUTPUTDIR%cr__ced__allocation.spv.h       SHADER_SPV_cr__ced__allocation
%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__scatter.spv          %compileshadersOUTPUTDIR%cr__ced__scatter.spv.h          SHADER_SPV_cr__ced__scatter
@ECHO ON

@REM return to the execution directory
@CD %compileshadersEXECDIR%