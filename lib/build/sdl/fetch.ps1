
# fetch.ps1
#   Downloads, extracts, and patches.

$srcVer = "1.2.14"
$src = "SDL-$srcVer"
$url = "http://www.libsdl.org/release/$src.zip"
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
