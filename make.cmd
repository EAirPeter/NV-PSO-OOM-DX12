@echo off

echo.dxc -Fo CS.cso CS.hlsl -E CSMain -T cs_6_0
dxc -Fo CS.cso CS.hlsl -E CSMain -T cs_6_0

if "%1" == "" (
  echo.cl main.cpp /EHsc /nologo
  cl main.cpp /EHsc /nologo
) else (
  echo.cl main.cpp /EHsc /nologo "/DUSE_NVIDIA=%1"
  cl main.cpp /EHsc /nologo "/DUSE_NVIDIA=%1"
)
