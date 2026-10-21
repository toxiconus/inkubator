param(
    [int]$Baud = 115200,
    [int]$TimeoutMs = 3000,
    [int]$Interval = 2
)

$ports = @('COM7', 'COM9')

function Test-Port {
    param(
        [string]$PortName
    )

    Write-Host '============================='
    Write-Host "Sprawdzam $PortName"

    $devicePath = "\.\$PortName"
    if (-not (Test-Path $devicePath)) {
        Write-Host "PORT NIE ISTNIEJE: $PortName"
        return
    }

    $port = New-Object System.IO.Ports.SerialPort($PortName, $Baud, 'None', 8, 'One')
    $port.DtrEnable = $false
    $port.RtsEnable = $false

    try {
        $port.Open()
        $port.ReadTimeout = $TimeoutMs
        Write-Host "OTWARTO port: $PortName baud: $Baud"
        $port.WriteLine('')

        $start = Get-Date
        while (((Get-Date) - $start).TotalMilliseconds -lt $TimeoutMs) {
            try {
                $line = $port.ReadLine()
                if ($line -ne $null) {
                    Write-Host "ODPOWIEDZ: $line"
                    break
                }
            } catch [System.TimeoutException] {
                # ignoruj timeout i spróbuj dalej aż minie limit
            }
        }

        Write-Host "KONIEC okresu odczytu $TimeoutMs ms"
    } catch {
        Write-Host "BŁĄD portu: $($_.Exception.Message)"
    } finally {
        if ($port -and $port.IsOpen) {
            $port.Close()
        }
        if ($port) {
            $port.Dispose()
        }
    }
}

Write-Host "Uruchamiam monitor COM7 i COM9. Przerwij Ctrl+C."

while ($true) {
    foreach ($portName in $ports) {
        Test-Port -PortName $portName
        Write-Host ''
    }
    Write-Host "Czekam $Interval sekund przed kolejnym skanem..."
    Start-Sleep -Seconds $Interval
}
