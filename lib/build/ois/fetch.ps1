
# fetch.ps1
#   Downloads, extracts, and patches.

$srcVer = "1.2.0"
$srcVerShort = "1.2"
$src = "ois"
#TODO: Make the SourceForge mirror user-selectable.
$url = "http://superb-sea2.dl.sourceforge.net/project/wgois/Source%20Release/$srcVerShort/ois_$srcVer.zip"
$curDir = Get-Location
$destFile = "$curDir\download\$src.zip"
$srcDir = "$curDir\src"

Write-Host "==> Downloading: $url"
$ua = New-Object System.Net.WebClient
try {
    $ua.DownloadFile($url, $destFile)
}
catch [Exception] {
    Write-Error "Failed to download: $url"
    break
}

Write-Host "==> Extracting: $destFile"
$sh = New-Object -ComObject Shell.Application
$sh.NameSpace("$curDir").CopyHere($sh.NameSpace($destFile).ParseName($src))

Write-Host "==> Renaming to: $srcDir"
Remove-Item -Recurse $srcdir -ErrorAction SilentlyContinue
Rename-Item "$curDir\$src" $srcDir
