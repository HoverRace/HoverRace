@echo off

:: gen-mo.cmd
::   Generates binary .mo files from .po files.

if not exist hoverrace.sln (
	echo This script must be run from the base directory.
	pause
	exit /b
)

for %%F in (locale\*.po) do (
	echo Generating %%~nF.mo...
	md share\locale\%%~nF\LC_MESSAGES 2> NUL
	msgfmt --statistics -o share\locale\%%~nF\LC_MESSAGES\hoverrace.mo "%%F"
)
