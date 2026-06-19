<#
.SYNOPSIS
  Выдать новый лицензионный ключ SmartView.
  Генерирует ключ, СКАЧИВАЕТ актуальный state.json из бакета (чтобы не затереть
  привязки устройств), добавляет запись и заливает state.json обратно.

.EXAMPLE
  ./new-key.ps1 -Owner "Иван Петров"
  ./new-key.ps1 -Owner "Пётр" -MaxDevices 2 -Expires "2026-12-31T23:59:59Z"

.NOTES
  Требуется настроенный yc CLI (yc init).
  Если в твоей версии yc флаги s3api отличаются — поправь две функции Get-State/Put-State ниже
  (либо правь state.json вручную через консоль Object Storage).
#>
param(
    [Parameter(Mandatory = $true)][string]$Owner,
    [int]$MaxDevices = 1,
    [string]$Expires = "",
    [string]$Bucket = "smartview-licenses",
    [string]$Key = ""
)

$ErrorActionPreference = "Stop"
$tmp = Join-Path $env:TEMP "smartview-state.json"

function New-LicenseKey {
    $c = '23456789ABCDEFGHJKLMNPQRSTUVWXYZ'   # без похожих 0/O/1/I/L
    (1..4 | ForEach-Object { -join (1..4 | ForEach-Object { $c[(Get-Random -Maximum $c.Length)] }) }) -join '-'
}

# --- доступ к Object Storage (при необходимости поправь флаги под свою версию yc) ---
function Get-State {
    yc storage s3api get-object --bucket $Bucket --key state.json --file $tmp | Out-Null
}
function Put-State {
    yc storage s3api put-object --bucket $Bucket --key state.json --body $tmp | Out-Null
}

if ([string]::IsNullOrWhiteSpace($Key)) { $Key = New-LicenseKey }

# 1) Скачать текущее состояние. Если не вышло — ОБРЫВАЕМ, чтобы не затереть привязки устройств.
try {
    Get-State
    $state = Get-Content $tmp -Raw -Encoding UTF8 | ConvertFrom-Json
} catch {
    throw "Не удалось скачать state.json из бакета '$Bucket'. Прерываю, чтобы не затереть привязки. ($($_.Exception.Message))"
}

if (-not $state.licenses) { $state | Add-Member -NotePropertyName licenses -NotePropertyValue ([pscustomobject]@{}) -Force }
if (-not $state.devices)  { $state | Add-Member -NotePropertyName devices  -NotePropertyValue ([pscustomobject]@{}) -Force }

# 2) Проверить, что ключа ещё нет
if ($state.licenses.PSObject.Properties.Name -contains $Key) {
    throw "Ключ $Key уже существует в state.json"
}

# 3) Добавить запись
$entry = [pscustomobject]@{ owner = $Owner; status = "active"; max_devices = $MaxDevices; expires = $Expires }
$state.licenses | Add-Member -NotePropertyName $Key -NotePropertyValue $entry

# 4) Записать и залить обратно
$json = $state | ConvertTo-Json -Depth 10
[IO.File]::WriteAllText($tmp, $json, (New-Object System.Text.UTF8Encoding($false)))
Put-State

Write-Host ""
Write-Host "Готово. Выдан ключ:" -ForegroundColor Green
Write-Host "    $Key" -ForegroundColor Cyan
$srok = if ($Expires) { $Expires } else { "бессрочно" }
Write-Host "    владелец: $Owner; устройств: $MaxDevices; срок: $srok"
Write-Host ""
Write-Host "Отдай человеку программу SmartView и этот ключ."
