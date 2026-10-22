<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Topbar + Tabs · Lab</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        :root {
            --bg: #0f120f;
            --panel: #1c211c;
            --panel2: #252a25;
            --text: #c0c4c0;
            --text-dim: #7a807a;
            --accent: #c8a85a;
            --green: #5a9a4a;
            --green-glow: #7aba6a;
            --red: #a04a3a;
            --brass: #b0905a;
            --mono: "Courier New", "Cascadia Code", monospace;
            --sans: "Arial Narrow", Arial, sans-serif;
        }

        body {
            background: radial-gradient(ellipse at 50% -10%, #1a221a 0%, #0f120f 75%);
            font-family: var(--mono);
            color: var(--text);
            min-height: 100vh;
            padding: 10px;
            display: flex;
            align-items: flex-start;
            justify-content: center;
        }

        .wrapper {
            width: 100%;
            max-width: 1280px;
            margin: 0 auto;
        }

        /* ═══ TOPBAR ═══ */
        .topbar {
            display: flex;
            align-items: center;
            gap: 6px;
            padding: 4px 14px;
            min-height: 40px;
            background: rgba(11, 20, 11, .96);
            border: 1px solid rgba(31, 58, 37, .6);
            border-radius: 8px 8px 0 0;
            backdrop-filter: blur(10px);
            flex-wrap: wrap;
        }

        .topbar-logo {
            font-family: var(--mono);
            font-weight: 600;
            font-size: 11px;
            letter-spacing: .12em;
            color: var(--accent);
            white-space: nowrap;
            padding-right: 4px;
        }

        .topbar-sep {
            width: 1px;
            height: 22px;
            background: rgba(47, 90, 58, .4);
            flex-shrink: 0;
        }

        .topbar-stat {
            display: flex;
            align-items: center;
            gap: 4px;
            font-size: 9px;
            color: var(--text-dim);
            font-family: var(--mono);
            white-space: nowrap;
        }

        .topbar-stat .dot {
            width: 6px;
            height: 6px;
            border-radius: 50%;
            display: inline-block;
        }
        .topbar-stat .dot.ok {
            background: var(--green);
            box-shadow: 0 0 6px var(--green-glow);
        }
        .topbar-stat .dot.err {
            background: var(--red);
            box-shadow: 0 0 6px var(--red);
        }

        .topbar-stat strong {
            color: var(--text);
            font-size: 10px;
            min-width: 20px;
        }

        /* mini bębenki */
        .topbar-drum {
            display: inline-flex;
            gap: 1px;
            background: #050705;
            padding: 1px 3px;
            border-radius: 3px;
            box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.8), 0 0 0 1px #1a1a14;
            align-items: center;
            height: 18px;
        }

        .topbar-drum .d {
            width: 10px;
            height: 14px;
            background: linear-gradient(145deg, #1c1c1c, #0f0f0f 40%, #1c1c1c 80%);
            border: 1px solid #2a2a2a;
            border-radius: 2px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 8px;
            font-weight: 700;
            color: #f0f2e2;
            text-shadow: 0 1px 2px #000, 0 -1px 1px rgba(255, 255, 255, 0.3);
            box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.8), 0 1px 0 rgba(255, 255, 255, 0.15);
        }

        .topbar-drum .dot-d {
            width: 3px;
            background: transparent;
            border: none;
            box-shadow: none;
            font-size: 7px;
            color: #ddd;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .topbar-drum .unit {
            font-size: 6px;
            color: var(--text-dim);
            background: transparent;
            border: none;
            box-shadow: none;
            padding: 0 2px;
        }

        .topbar-time {
            font-family: var(--mono);
            font-size: 10px;
            color: var(--text);
            letter-spacing: .04em;
        }

        .topbar-esp {
            font-size: 8px;
            color: var(--text-dim);
            font-family: var(--mono);
            margin-left: auto;
            white-space: nowrap;
        }
        .topbar-esp span {
            color: var(--text);
        }

        /* ═══ TABS ═══ */
        .tabs {
            display: flex;
            gap: 3px;
            padding: 4px 14px 0;
            background: rgba(11, 20, 11, .95);
            border-bottom: 1px solid rgba(31, 58, 37, .6);
            border-radius: 0 0 8px 8px;
            flex-wrap: wrap;
        }

        .tab {
            padding: 5px 12px;
            font-size: 9px;
            font-weight: 600;
            color: var(--text-dim);
            border: none;
            background: rgba(255, 255, 255, .02);
            cursor: pointer;
            border-radius: 5px 5px 0 0;
            transition: all .15s ease;
            font-family: var(--mono);
            letter-spacing: .04em;
            display: flex;
            align-items: center;
            gap: 4px;
        }

        .tab:hover {
            color: var(--text);
            background: rgba(255, 255, 255, .04);
        }

        .tab.active {
            color: var(--accent);
            background: rgba(57, 211, 95, .06);
            box-shadow: inset 0 -2px 0 var(--accent);
        }

        .tab .sym {
            font-size: 12px;
            opacity: 0.7;
        }

        /* ─── MINIMALNA ZAWARTOŚĆ ─── */
        .content-placeholder {
            margin-top: 20px;
            padding: 20px;
            background: rgba(0,0,0,0.2);
            border-radius: 8px;
            text-align: center;
            color: var(--text-dim);
            font-family: var(--mono);
            font-size: 11px;
            border: 1px solid rgba(255,255,255,0.04);
        }

        /* ─── RESPONSYWNOŚĆ ─── */
        @media (max-width: 700px) {
            .topbar { gap: 4px; padding: 4px 8px; }
            .topbar-logo { font-size: 9px; }
            .topbar-stat { font-size: 8px; }
            .topbar-drum .d { width: 8px; height: 12px; font-size: 7px; }
            .topbar-drum .unit { font-size: 5px; }
            .topbar-time { font-size: 9px; }
            .tab { font-size: 8px; padding: 4px 6px; }
            .tab .sym { font-size: 10px; }
        }

        @media (max-width: 480px) {
            .topbar { flex-wrap: wrap; gap: 2px; }
            .topbar-sep { display: none; }
            .topbar-esp { margin-left: 0; width: 100%; text-align: right; }
        }
    </style>
</head>
<body>

<div class="wrapper">

    <!-- ═══ TOPBAR ═══ -->
    <header class="topbar">
        <span class="topbar-logo">INCUBATOR</span>
        <span class="topbar-sep"></span>

        <!-- API -->
        <span class="topbar-stat">
            <span class="dot ok" id="apiDot"></span>
            <span>API</span>
            <strong id="apiStatusTop">OK</strong>
        </span>
        <span class="topbar-sep"></span>

        <!-- DS1 -->
        <span class="topbar-stat">
            <span style="font-size:7px;color:var(--text-dim);">DS1</span>
            <span class="topbar-drum">
                <span class="d" id="t-ds1-1">3</span>
                <span class="d" id="t-ds1-2">7</span>
                <span class="dot-d">.</span>
                <span class="d" id="t-ds1-3">8</span>
                <span class="d" id="t-ds1-4">2</span>
                <span class="unit">°C</span>
            </span>
        </span>
        <span class="topbar-sep"></span>

        <!-- DS2 -->
        <span class="topbar-stat">
            <span style="font-size:7px;color:var(--text-dim);">DS2</span>
            <span class="topbar-drum">
                <span class="d" id="t-ds2-1">3</span>
                <span class="d" id="t-ds2-2">7</span>
                <span class="dot-d">.</span>
                <span class="d" id="t-ds2-3">7</span>
                <span class="d" id="t-ds2-4">6</span>
                <span class="unit">°C</span>
            </span>
        </span>
        <span class="topbar-sep"></span>

        <!-- RH (DHT11) -->
        <span class="topbar-stat">
            <span style="font-size:7px;color:var(--text-dim);">RH</span>
            <span class="topbar-drum">
                <span class="d" id="t-rh-1">5</span>
                <span class="d" id="t-rh-2">2</span>
                <span class="dot-d">.</span>
                <span class="d" id="t-rh-3">3</span>
                <span class="unit">%</span>
            </span>
        </span>
        <span class="topbar-sep"></span>

        <!-- Czas -->
        <span class="topbar-stat">
            <span style="font-size:7px;color:var(--text-dim);">TIME</span>
            <strong class="topbar-time" id="topClock">--:--:--</strong>
        </span>

        <!-- ESP temp -->
        <span class="topbar-esp">
            ESP <span id="espTempTop">--°C</span>
        </span>
    </header>

    <!-- ═══ TABS ═══ -->
    <nav class="tabs">
        <button class="tab active" data-tab="status" onclick="switchTab('status')">
            <span class="sym">⊞</span> STATUS
        </button>
        <button class="tab" data-tab="wykresy" onclick="switchTab('wykresy')">
            <span class="sym">⌇</span> WYKRESY
        </button>
        <button class="tab" data-tab="harmonogram" onclick="switchTab('harmonogram')">
            <span class="sym">⊠</span> HARMONOGRAM
        </button>
        <button class="tab" data-tab="dziennik" onclick="switchTab('dziennik')">
            <span class="sym">◰</span> DZIENNIK
        </button>
        <button class="tab" data-tab="logi" onclick="switchTab('logi')">
            <span class="sym">◉</span> LOGI
        </button>
        <button class="tab" data-tab="kalibracja" onclick="switchTab('kalibracja')">
            <span class="sym">◷</span> KALIBRACJA
        </button>
    </nav>

    <!-- ─── PRZYKŁADOWA ZAWARTOŚĆ ─── -->
    <div class="content-placeholder" id="content">
        Wybrano zakładkę: <strong id="activeTabLabel">STATUS</strong>
    </div>

</div>

<script>
    // ─── PRZEŁĄCZANIE ZAKŁADEK ───
    function switchTab(name) {
        // Aktualizacja przycisków
        document.querySelectorAll('.tab').forEach(t => {
            t.classList.toggle('active', t.dataset.tab === name);
        });
        // Aktualizacja etykiety
        const labels = {
            status: 'STATUS',
            wykresy: 'WYKRESY',
            harmonogram: 'HARMONOGRAM',
            dziennik: 'DZIENNIK',
            logi: 'LOGI',
            kalibracja: 'KALIBRACJA'
        };
        document.getElementById('activeTabLabel').textContent = labels[name] || name.toUpperCase();
    }

    // ─── SYMULACJA ZEGARA ───
    function updateClock() {
        const now = new Date();
        document.getElementById('topClock').textContent = now.toLocaleTimeString('pl-PL', { hour12: false });
    }
    setInterval(updateClock, 1000);
    updateClock();

    // ─── SYMULACJA DANYCH BĘBENKÓW (dla przykładu) ───
    function randomizeDrums() {
        const v1 = (37 + Math.random() * 0.8).toFixed(2);
        document.getElementById('t-ds1-1').textContent = v1[0] || '0';
        document.getElementById('t-ds1-2').textContent = v1[1] || '0';
        document.getElementById('t-ds1-3').textContent = v1[3] || '0';
        document.getElementById('t-ds1-4').textContent = v1[4] || '0';

        const v2 = (37 + Math.random() * 0.6).toFixed(2);
        document.getElementById('t-ds2-1').textContent = v2[0] || '0';
        document.getElementById('t-ds2-2').textContent = v2[1] || '0';
        document.getElementById('t-ds2-3').textContent = v2[3] || '0';
        document.getElementById('t-ds2-4').textContent = v2[4] || '0';

        const rh = (45 + Math.random() * 15).toFixed(1);
        document.getElementById('t-rh-1').textContent = rh[0] || '0';
        document.getElementById('t-rh-2').textContent = rh[1] || '0';
        document.getElementById('t-rh-3').textContent = rh[3] || '0';

        document.getElementById('espTempTop').textContent = (38 + Math.random() * 4).toFixed(1) + '°C';

        // Losowa zmiana statusu API
        const ok = Math.random() > 0.2;
        const dot = document.getElementById('apiDot');
        dot.className = 'dot ' + (ok ? 'ok' : 'err');
        document.getElementById('apiStatusTop').textContent = ok ? 'OK' : 'ERR';
    }

    setInterval(randomizeDrums, 1500);
    randomizeDrums();

    // Eksport funkcji do global
    window.switchTab = switchTab;
</script>

</body>
</html>