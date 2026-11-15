$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & g++ -std=c++17 -Wall -Wextra -Wpedantic -static -I include tests/regression.cpp -o build/regression.exe
    if ($LASTEXITCODE -ne 0) { throw 'Build tests failed.' }
    & .\build\regression.exe
    if ($LASTEXITCODE -ne 0) { throw 'Regression tests failed.' }
} finally {
    Pop-Location
}
