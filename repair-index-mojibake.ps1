$path = 'J:\inkubator\inkubator-new\firmware\data\index.html'
$backup = 'J:\inkubator\inkubator-new\firmware\data\index.before-mojibake-fix.html'

Copy-Item -LiteralPath $path -Destination $backup -Force

$cp1250 = [System.Text.Encoding]::GetEncoding(1250)
$utf8 = [System.Text.Encoding]::UTF8

$raw = Get-Content -Raw -LiteralPath $path
$lines = $raw -split "`r?`n"

$badChars = @([char]194, [char]226, [char]258, [char]196, [char]313, [char]273)
$changed = 0

for ($i = 0; $i -lt $lines.Length; $i++) {
    $line = $lines[$i]
    if ($line.IndexOfAny($badChars) -ge 0) {
        $bytes = $cp1250.GetBytes($line)
        $fixed = $utf8.GetString($bytes)
        if ($fixed -ne $line) {
            $lines[$i] = $fixed
            $changed++
        }
    }
}

$updated = ($lines -join "`r`n")
Set-Content -LiteralPath $path -Value $updated -Encoding UTF8
Write-Output ("MOJIBAKE_FIXED_LINES=" + $changed)
Write-Output ("BACKUP=" + $backup)
