$portName = 'COM9'
$baud = 115200
$timeoutMs = 2000

Write-Host "Sprawdzam port: $portName"
$devicePath = "\.\$portName"
if (-not (Test-Path $devicePath)) {
    Write-Host "PORT NIE ISTNIEJE: $portName"
    exit 1
}

$port = New-Object System.IO.Ports.SerialPort($portName, $baud, 'None', 8, 'One')
$port.DtrEnable = $false
$port.RtsEnable = $false
try {
    $port.Open()
    $port.ReadTimeout = $timeoutMs
    Write-Host "OTWARTO: $portName baud=$baud"
    $port.WriteLine('')
    for ($i = 0; $i -lt 5; $i++) {
        try {
            $line = $port.ReadLine()
            if ($line -ne $null) {
                Write-Host "ODPOWIEDZ: $line"
                break
            }
        } catch [System.TimeoutException] {
            Write-Host "TIMEOUT odczytu"
            break
        }
    }
} catch {
    Write-Host "BŁĄD: $($_.Exception.Message)"
} finally {
    if ($port -and $port.IsOpen) {
        $port.Close()
        Write-Host "ZAMKNIETO: $portName"
    }
}
