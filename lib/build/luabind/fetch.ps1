
# fetch.ps1
#   Downloads, extracts, and patches.

$srcVer = "0.9"
$src = "luabind-$srcVer"
#TODO: Make the SourceForge mirror user-selectable.
$url = "http://superb-sea2.dl.sourceforge.net/project/luabind/luabind/$srcVer/$src.zip"
$curDir = Get-Location
$destFile = "$curDir\download\$src.zip"
$srcDir = "$curDir\src"
$patchFile = "patches\0.9-nullptr.patch"

Write-Host "==> Downloading: $url"
$ua = New-Object System.Net.WebClient
$ua.DownloadFile($url, $destFile)

Write-Host "==> Extracting: $destFile"
$sh = New-Object -ComObject Shell.Application
$sh.NameSpace("$curDir").CopyHere($sh.NameSpace($destFile).ParseName($src))

Write-Host "==> Renaming to: $srcDir"
Remove-Item -Recurse $srcdir -ErrorAction SilentlyContinue
Rename-Item "$curDir\$src" $srcDir

Write-Host "==> Patching with: $patchFile"
#FIXME: Replace with user-configurable patch binary.
w-pat -p1 -d $srcDir -i "..\$patchFile"
