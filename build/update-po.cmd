@echo off

:: update-po.cmd
::   Extracts and merges translatable strings.

if not exist hoverrace.sln (
	echo This script must be run from the base directory.
	pause
	exit /b
)

echo Generating template...
:: We use this method of making the file list so that it roughly
:: matches what gets generated via the shell script.
type nul > srcfiles.tmp
for %%F in (client\Game2\*.cpp) do echo ./client/Game2/%%~nxF >> srcfiles.tmp
xgettext -f srcfiles.tmp -o locale/hoverrace.pot --sort-by-file --c++ --boost -k_
del srcfiles.tmp

for /R locale %%F in (*.po) do (
 	echo Merging %%~nF...
 	msgmerge --sort-by-file --update "%%F" locale/hoverrace.pot
)
