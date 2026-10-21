$ErrorActionPreference = 'Stop'

$files = Get-ChildItem -Path 'J:\inkubator\inkubator-new\firmware\data' -Filter *.html | Where-Object { $_.Name -ne 'index.html' }

$metaDescLine = '    <meta name="description" content="Panel sterowania inkubatorem: konfiguracja, diagnostyka i monitoring." />'
$metaThemeLine = '    <meta name="theme-color" content="#101317" />'
$bodyBlock = @"
<body>

    <noscript><div class="noscript-banner">Do poprawnego dzialania panelu wymagany jest JavaScript.</div></noscript>
    <div id="offlineBanner" class="offline-banner" role="status" aria-live="polite" aria-atomic="true"></div>
"@

foreach ($file in $files) {
    $text = Get-Content -Raw -LiteralPath $file.FullName

    if ($text -notmatch '<meta name="description"') {
        $text = [regex]::Replace(
            $text,
            '(<meta name="viewport"[^>]*>\s*)',
            "$1`n$metaDescLine`n$metaThemeLine`n",
            1
        )
    }

    $text = $text -replace '<div class="instrument">', '<main class="instrument" id="mainContent" role="main">'
    $text = $text -replace '<div class="status-bar">', '<div class="status-bar" role="status" aria-live="polite">'

    if ($text -notmatch '<body>') {
        $text = [regex]::Replace($text, '</head>\s*', "</head>`n$bodyBlock", 1)
    }

    if ($text -notmatch '<noscript><div class="noscript-banner">') {
        $text = [regex]::Replace($text, '<body>\s*', $bodyBlock, 1)
    }

    if ($text -notmatch '</main>') {
        $text = [regex]::Replace($text, '</div>\s*(<!-- ===== SKRYPTY ===== -->)', "</main>`n`n    $1", 1)
        $text = [regex]::Replace($text, '</div>\s*</body>', "</main>`n`n</body>", 1)
    }

    Set-Content -LiteralPath $file.FullName -Value $text -Encoding UTF8
}
