
# fetch.ps1
#   Downloads, extracts, and patches.

$srcVer = "5.1.4"
$srcNormVer = "5_1_4"
$src = "lua5.1"
#TODO: Make the SourceForge mirror user-selectable.
$url = "http://superb-sea2.dl.sourceforge.net/project/luabinaries/$srcVer/Docs%20and%20Sources/lua$srcNormVer`_Sources.zip"
$curDir = Get-Location
$destFile = "$curDir\download\$src.zip"
$srcDir = "$curDir\src"

Write-Host "==> Downloading: $url"
$ua = New-Object System.Net.WebClient
$ua.DownloadFile($url, $destFile)

Write-Host "==> Extracting: $destFile"
$sh = New-Object -ComObject Shell.Application
$sh.NameSpace("$curDir").CopyHere($sh.NameSpace($destFile).ParseName($src))

Write-Host "==> Renaming to: $srcDir"
Remove-Item -Recurse $srcdir -ErrorAction SilentlyContinue
Rename-Item "$curDir\$src" $srcDir
