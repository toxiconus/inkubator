$files = @('read_com9_once.ps1','check_mpu_com.ps1')
foreach ($f in $files) {
    Write-Host "=== $f ==="
    Get-Content $f | Select-Object -First 30 | ForEach-Object { Write-Host $_ }
    Write-Host '--- parse ---'
    $errors = [ref]@()
    [System.Management.Automation.Language.Parser]::ParseFile($f, [ref]$null, $errors)
    if ($errors.Value.Count -eq 0) { Write-Host 'Parser: OK' } else { $errors.Value | ForEach-Object { Write-Host $_.ToString() } }
    Write-Host
}
