@echo off

echo Compiling file using assettool
cd ../../Rapture/manifest
"%~dp0..\..\tools\assettool\assettool\Debug\assettool.exe" -project "__rapture.json" -outdir "../core"