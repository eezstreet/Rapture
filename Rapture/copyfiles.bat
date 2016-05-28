@echo off

echo Copying libraries
xcopy /s /f /i /y /v /c *.dll %RAPTURE_INSTALL%

echo Copying awesomium_process.exe
xcopy /s /f /i /y /v /c awesomium_process.exe %RAPTURE_INSTALL%

echo Copying game assets
xcopy /s /f /i /y /v /c core %RAPTURE_INSTALL%\core
