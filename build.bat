@echo off
@set SOURCE=src/main.c
@set OUT_EXE=main
@set OUT_DIR=build
@set LIBS=user32.lib gdi32.lib

IF NOT EXIST %OUT_DIR%\ MKDIR %OUT_DIR% 

cl -Zi %SOURCE% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% 