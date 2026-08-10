# WebIntoApp Packaging Tool
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir = Join-Path $ScriptDir "..\LAB5_Dev\solution\data"
$OutputZip = Join-Path $ScriptDir "iot_dashboard_package.zip"

Write-Host "=================================================="
Write-Host "WebIntoApp Packaging Tool for IoT Dashboard"
Write-Host "=================================================="

if (Test-Path $SourceDir) {
    Write-Host "Found source directory: $SourceDir"
    if (Test-Path $OutputZip) {
        Remove-Item $OutputZip -Force
        Write-Host "Removed previous zip file."
    }
    Write-Host "Compressing files into $OutputZip..."
    Compress-Archive -Path "$SourceDir\*" -DestinationPath $OutputZip -Force
    Write-Host "Compression complete! Ready to upload to WebIntoApp.com"
} else {
    Write-Host "Source directory not found: $SourceDir"
}
