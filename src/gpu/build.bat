@SET "compileshadersEXECDIR=%CD%"

@REM move to where this file is located
@CD %~dp0

@SET "compileshadersTEMPDIR=..\..\temp\"
@SET "compileshadersEMBED=%compileshadersTEMPDIR%embed.exe"
@SET "compileshadersOUTPUTDIR=..\gen\"

@SET "compileshadersGODOTMINORVERSION=-DGODOT_VERSION_MINOR=5"
@SET "compileshadersGLSLANGFLAGS=-gV -I. --target-env vulkan1.3"
@SET "compileshadersSTAGEVERT=-DSTAGE_VERT -S vert"
@SET "compileshadersSTAGEFRAG=-DSTAGE_FRAG -S frag"

@ECHO OFF

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\test\dummy.comp           -o %compileshadersTEMPDIR%dummy.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\test\dummy_commander.comp -o %compileshadersTEMPDIR%dummy_commander.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\test\dummy_debug.comp     -o %compileshadersTEMPDIR%dummy_debug.spv

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\face_orientation.comp -o %compileshadersTEMPDIR%cr__ced__face_orientation.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\detection.comp        -o %compileshadersTEMPDIR%cr__ced__detection.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\allocation.comp       -o %compileshadersTEMPDIR%cr__ced__allocation.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\contour_edge_detection\scatter.comp          -o %compileshadersTEMPDIR%cr__ced__scatter.spv

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\fragment_generation\first_commander.comp -o %compileshadersTEMPDIR%cr__fg__first_commander.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\fragment_generation\frag_counts.comp     -o %compileshadersTEMPDIR%cr__fg__frag_counts.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\fragment_generation\allocation.comp      -o %compileshadersTEMPDIR%cr__fg__allocation.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_rasterization\fragment_generation\scatter.comp         -o %compileshadersTEMPDIR%cr__fg__scatter.spv

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\first_commander.comp  -o %compileshadersTEMPDIR%cr__cpg__first_commander.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\soft_depth_test.comp  -o %compileshadersTEMPDIR%cr__cpg__soft_depth_test.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\pre_alloc.comp        -o %compileshadersTEMPDIR%cr__cpg__pre_alloc.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\allocation.comp       -o %compileshadersTEMPDIR%cr__cpg__allocation.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\scatter.comp          -o %compileshadersTEMPDIR%cr__cpg__scatter.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\second_commander.comp -o %compileshadersTEMPDIR%cr__cpg__second_commander.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% %compileshadersSTAGEVERT% -V .\contour_rasterization\contour_pixel_generation\hard_depth_test.glsl  -o %compileshadersTEMPDIR%cr__cpg__hard_depth_test.vert.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% %compileshadersSTAGEFRAG% -V .\contour_rasterization\contour_pixel_generation\hard_depth_test.glsl  -o %compileshadersTEMPDIR%cr__cpg__hard_depth_test.frag.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS%                           -V .\contour_rasterization\contour_pixel_generation\decode.comp           -o %compileshadersTEMPDIR%cr__cpg__decode.spv

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_chaining\pixel_edge_generation\first_commander.comp -o %compileshadersTEMPDIR%cc__peg__first_commander.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\contour_chaining\pixel_edge_generation\generation.comp      -o %compileshadersTEMPDIR%cc__peg__generation.spv

	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\debug\display_contour_fragments.comp  -o %compileshadersTEMPDIR%debug__display_contour_fragments.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\debug\display_contour_pixels.comp     -o %compileshadersTEMPDIR%debug__display_contour_pixels.spv
	glslang %compileshadersGODOTMINORVERSION% %compileshadersGLSLANGFLAGS% -V .\debug\display_sparse_pixel_edges.comp -o %compileshadersTEMPDIR%debug__display_sparse_pixel_edges.spv

gcc embed.c -o %compileshadersEMBED%

	%compileshadersEMBED% %compileshadersTEMPDIR%dummy.spv           %compileshadersOUTPUTDIR%dummy.spv.h           SHADER_SPV_dummy
	%compileshadersEMBED% %compileshadersTEMPDIR%dummy_commander.spv %compileshadersOUTPUTDIR%dummy_commander.spv.h SHADER_SPV_dummy_commander
	%compileshadersEMBED% %compileshadersTEMPDIR%dummy_debug.spv     %compileshadersOUTPUTDIR%dummy_debug.spv.h     SHADER_SPV_dummy_debug

	%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__face_orientation.spv %compileshadersOUTPUTDIR%cr__ced__face_orientation.spv.h SHADER_SPV_cr__ced__face_orientation
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__detection.spv        %compileshadersOUTPUTDIR%cr__ced__detection.spv.h        SHADER_SPV_cr__ced__detection
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__allocation.spv       %compileshadersOUTPUTDIR%cr__ced__allocation.spv.h       SHADER_SPV_cr__ced__allocation
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__ced__scatter.spv          %compileshadersOUTPUTDIR%cr__ced__scatter.spv.h          SHADER_SPV_cr__ced__scatter

	%compileshadersEMBED% %compileshadersTEMPDIR%cr__fg__first_commander.spv %compileshadersOUTPUTDIR%cr__fg__first_commander.spv.h SHADER_SPV_cr__fg__first_commander
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__fg__frag_counts.spv     %compileshadersOUTPUTDIR%cr__fg__frag_counts.spv.h     SHADER_SPV_cr__fg__frag_counts
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__fg__allocation.spv      %compileshadersOUTPUTDIR%cr__fg__allocation.spv.h      SHADER_SPV_cr__fg__allocation
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__fg__scatter.spv         %compileshadersOUTPUTDIR%cr__fg__scatter.spv.h         SHADER_SPV_cr__fg__scatter

	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__first_commander.spv      %compileshadersOUTPUTDIR%cr__cpg__first_commander.spv.h      SHADER_SPV_cr__cpg__first_commander
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__soft_depth_test.spv      %compileshadersOUTPUTDIR%cr__cpg__soft_depth_test.spv.h      SHADER_SPV_cr__cpg__soft_depth_test
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__pre_alloc.spv            %compileshadersOUTPUTDIR%cr__cpg__pre_alloc.spv.h            SHADER_SPV_cr__cpg__pre_alloc
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__allocation.spv           %compileshadersOUTPUTDIR%cr__cpg__allocation.spv.h           SHADER_SPV_cr__cpg__allocation
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__scatter.spv              %compileshadersOUTPUTDIR%cr__cpg__scatter.spv.h              SHADER_SPV_cr__cpg__scatter
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__second_commander.spv     %compileshadersOUTPUTDIR%cr__cpg__second_commander.spv.h     SHADER_SPV_cr__cpg__second_commander
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__hard_depth_test.vert.spv %compileshadersOUTPUTDIR%cr__cpg__hard_depth_test.vert.spv.h SHADER_SPV_cr__cpg__hard_depth_test__vert
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__hard_depth_test.frag.spv %compileshadersOUTPUTDIR%cr__cpg__hard_depth_test.frag.spv.h SHADER_SPV_cr__cpg__hard_depth_test__frag
	%compileshadersEMBED% %compileshadersTEMPDIR%cr__cpg__decode.spv               %compileshadersOUTPUTDIR%cr__cpg__decode.spv.h               SHADER_SPV_cr__cpg__decode

	%compileshadersEMBED% %compileshadersTEMPDIR%cc__peg__first_commander.spv %compileshadersOUTPUTDIR%cc__peg__first_commander.spv.h SHADER_SPV_cc__peg__first_commander
	%compileshadersEMBED% %compileshadersTEMPDIR%cc__peg__generation.spv      %compileshadersOUTPUTDIR%cc__peg__generation.spv.h      SHADER_SPV_cc__peg__generation

	%compileshadersEMBED% %compileshadersTEMPDIR%debug__display_contour_fragments.spv  %compileshadersOUTPUTDIR%debug__display_contour_fragments.spv.h  SHADER_SPV_debug__display_contour_fragments
	%compileshadersEMBED% %compileshadersTEMPDIR%debug__display_contour_pixels.spv     %compileshadersOUTPUTDIR%debug__display_contour_pixels.spv.h     SHADER_SPV_debug__display_contour_pixels
	%compileshadersEMBED% %compileshadersTEMPDIR%debug__display_sparse_pixel_edges.spv %compileshadersOUTPUTDIR%debug__display_sparse_pixel_edges.spv.h SHADER_SPV_debug__display_sparse_pixel_edges

@ECHO ON

@REM return to the execution directory
@CD %compileshadersEXECDIR%