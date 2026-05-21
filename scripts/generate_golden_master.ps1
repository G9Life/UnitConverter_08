# Generates tests/golden_master_expected.txt from UnitConverter stdout.
# Usage (from repo root):
#   .\scripts\generate_golden_master.ps1
#   .\scripts\generate_golden_master.ps1 -BuildDir build

param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Exe = Join-Path $RepoRoot (Join-Path $BuildDir "UnitConverter.exe")
$OutFile = Join-Path $RepoRoot "tests\golden_master_expected.txt"
$PromptPrefix = "Insert value for converting"
$Scenarios = @("meter:2.5", "feet:1.0", "yard:1.0", "meter:0.0")

if (-not (Test-Path $Exe)) {
    Write-Error "UnitConverter not found: $Exe`nRun: cmake -S . -B $BuildDir -DUNIT_CONVERTER_RED_PHASE=OFF; cmake --build $BuildDir"
}

function Get-ConversionStdout {
    param([string]$Scenario)
    $prevEap = $ErrorActionPreference
    $ErrorActionPreference = "SilentlyContinue"
    try {
        $lines = @($Scenario | & $Exe 2>&1 | Where-Object { $_ -is [string] })
    } finally {
        $ErrorActionPreference = $prevEap
    }
    $filtered = foreach ($line in $lines) {
        if ($line -like "ERR-*") { continue }
        if ($line -like "$PromptPrefix*") {
            $marker = "): "
            $idx = $line.IndexOf($marker)
            if ($idx -ge 0) {
                $line.Substring($idx + $marker.Length)
            }
        } else {
            $line
        }
    }
    $filtered = $filtered | Where-Object { $_ -and $_.Trim().Length -gt 0 }
    if ($filtered.Count -eq 0) {
        return ""
    }
    ($filtered -join "`n") + "`n"
}

$sb = New-Object System.Text.StringBuilder
foreach ($scenario in $Scenarios) {
    [void]$sb.AppendLine("[$scenario]")
    [void]$sb.Append((Get-ConversionStdout -Scenario $scenario))
    [void]$sb.AppendLine("---")
}

$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllText($OutFile, $sb.ToString().TrimEnd("`r", "`n") + "`n", $utf8NoBom)

Write-Host "Wrote $OutFile"
Write-Host "Stage for git: git add tests/golden_master_expected.txt"
