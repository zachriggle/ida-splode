
rem %PIN_HOME%\ia32\bin\pin -t %1\ida-splode.dll -python demo.py -mw demo -en "main" -- %1\demo.exe && python demo.py
rem %PIN_HOME%\ia32\bin\pin -t %1\ida-splode.dll -r "demo!main" -- %1\demo.exe && python demo.py

rem %PIN_HOME%\ia32\bin\pin -t %1\ida-splode.dll -r "demo!main" -- %1\demo.exe
%PIN_HOME%\ia32\bin\pin -t %1\ida-splode.dll -- %1\demo.exe
