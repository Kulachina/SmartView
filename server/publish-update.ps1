<#
.SYNOPSIS
  Опубликовать новую версию SmartView: загрузить установщик в бакет обновлений и
  обновить version.json, чтобы установленные приложения предложили обновиться.

.EXAMPLE
  ./publish-update.ps1 -Version 1.0.1 -File "C:\build\SmartView-1.0.1-setup.exe" -Notes "Контрольные диапазоны, автообновление"

.NOTES
  Бакет обновлений (по умолчанию smartview-updates) должен быть с ПУБЛИЧНЫМ чтением объектов,
  иначе приложение не сможет скачать version.json и установщик.
  Перед сборкой релиза не забудь поднять VERSION в src/updater.h до этого же номера.
#>
param(
    [Parameter(Mandatory = $true)][string]$Version,
    [Parameter(Mandatory = $true)][string]$File,    # путь к установщику (.exe)
    [string]$Notes = "",
    [string]$Bucket = "smartview-updates"
)

$ErrorActionPreference = "Stop"
if (-not (Test-Path $File)) { throw "Файл установщика не найден: $File" }

$name = Split-Path $File -Leaf

# 1) загрузить установщик
yc storage s3api put-object --bucket $Bucket --key $name --body $File | Out-Null

# 2) собрать и залить version.json
$url = "https://storage.yandexcloud.net/$Bucket/$name"
$manifest = [ordered]@{ version = $Version; url = $url; notes = $Notes }
$tmp = Join-Path $env:TEMP "version.json"
[IO.File]::WriteAllText($tmp, ($manifest | ConvertTo-Json -Depth 5), (New-Object System.Text.UTF8Encoding($false)))
yc storage s3api put-object --bucket $Bucket --key version.json --body $tmp | Out-Null

Write-Host ""
Write-Host "Опубликовано: версия $Version" -ForegroundColor Green
Write-Host "    установщик: $url"
Write-Host "    манифест:   https://storage.yandexcloud.net/$Bucket/version.json"
Write-Host ""
Write-Host "Установленные приложения предложат обновиться при следующем запуске."
