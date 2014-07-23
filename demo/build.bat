rem gflags /i test.exe 
rem +htc +hfc +hpc +htg +htd +hpa

cl *.cpp /Zi /MD /Od /DYNAMICBASE:NO /GS- /link /INCREMENTAL:no
