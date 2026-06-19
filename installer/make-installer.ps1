<#
.SYNOPSIS
  Собрать офлайн-установщик SmartView (один setup.exe) из готовой папки dist\.

.EXAMPLE
  # Сначала: Release-сборка + windeployqt в папку dist\ (см. docs)
  ./make-installer.ps1 -Version 0.9.2

.NOTES
  Требуется Qt Installer Framework (binarycreator.exe).
  Версию ставит в config.xml и package.xml автоматически — следи, чтобы она
  совпадала с #define VERSION в src/updater.h и с тем, что публикуешь в version.json.
#>
param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$DistDir = "",
    [string]$Ifw = "C:\Qt\Tools\QtInstallerFramework\4.8\bin\binarycreator.exe"
)

$ErrorActionPreference = "Stop"
$here    = $PSScriptRoot
$pkg     = Join-Path $here "packages\com.kulachina.smartview"
$data    = Join-Path $pkg  "data"
$config  = Join-Path $here "config\config.xml"
$pkgxml  = Join-Path $pkg  "meta\package.xml"
$enc     = New-Object System.Text.UTF8Encoding($false)

if (-not $DistDir) { $DistDir = Join-Path (Split-Path $here -Parent) "dist" }
if (-not (Test-Path "$DistDir\SmartView.exe")) {
    throw "Не найден $DistDir\SmartView.exe. Сначала собери Release и сделай windeployqt в папку dist\."
}
if (-not (Test-Path $Ifw)) { throw "Не найден binarycreator: $Ifw (укажи путь через -Ifw)" }

# 1) проставить версию в config.xml и package.xml
[IO.File]::WriteAllText($config, ((Get-Content $config -Raw) -replace '<Version>.*?</Version>', "<Version>$Version</Version>"), $enc)
[IO.File]::WriteAllText($pkgxml, ((Get-Content $pkgxml -Raw) -replace '<Version>.*?</Version>', "<Version>$Version</Version>"), $enc)

# 2) положить файлы программы (из dist\) в data\
if (Test-Path $data) { Remove-Item $data -Recurse -Force }
New-Item -ItemType Directory $data | Out-Null
Copy-Item "$DistDir\*" $data -Recurse

# 3) собрать офлайн-установщик
$out = Join-Path (Split-Path $here -Parent) "SmartView-$Version-setup.exe"
& $Ifw -c $config -p (Join-Path $here "packages") $out

Write-Host ""
Write-Host "Готово: $out" -ForegroundColor Green
Write-Host "Дальше опубликуй: server\publish-update.ps1 -Version $Version -File `"$out`" -Notes `"...`""
