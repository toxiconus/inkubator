(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[symulacja_legów.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  PROFILES (uproszczone)
    // ============================================================
    const PROFILES = {
        chicken: {
            common_name: 'Kura',
            incubation_days: 21,
            day_by_day: Array.from({ length: 21 }, (_, i) => {
                const d = i + 1;
                const isLockdown = d >= 19;
                return {
                    day: d,
                    shell_temp: isLockdown ? 37.5 : 37.7 + (d > 13 ? Math.min((d - 13) * 0.1, 0.7) : 0),
                    humidity_set: isLockdown ? 72 : 50,
                    humidity_range: isLockdown ? [65, 80] : [40, 60],
                    turning: !isLockdown,
                    sensitivity: d <= 3 ? 'CRITICAL' : (d <= 5 || isLockdown ? 'HIGH' : 'MEDIUM')
                };
            })
        },
        duck_indian_runner: {
            common_name: 'Kaczka Biegus',
            incubation_days: 28,
            day_by_day: Array.from({ length: 28 }, (_, i) => {
                const d = i + 1;
                const isLockdown = d >= 25;
                return {
                    day: d,
                    shell_temp: isLockdown ? 37.5 : 37.7 + (d > 12 ? Math.min((d - 12) * 0.06, 0.7) : 0),
                    humidity_set: isLockdown ? 68 : 55,
                    humidity_range: isLockdown ? [65, 75] : [50, 62],
                    turning: !isLockdown,
                    sensitivity: d <= 3 ? 'CRITICAL' : (d <= 5 || isLockdown ? 'HIGH' : 'MEDIUM')
                };
            })
        },
        goose: {
            common_name: 'Gęś',
            incubation_days: 30,
            day_by_day: Array.from({ length: 30 }, (_, i) => {
                const d = i + 1;
                const isLockdown = d >= 27;
                return {
                    day: d,
                    shell_temp: isLockdown ? 37.5 : 37.7 + (d > 12 ? Math.min((d - 12) * 0.06, 0.7) : 0),
                    humidity_set: isLockdown ? 68 : 55,
                    humidity_range: isLockdown ? [65, 75] : [50, 62],
                    turning: !isLockdown,
                    sensitivity: d <= 3 ? 'CRITICAL' : (d <= 5 || isLockdown ? 'HIGH' : 'MEDIUM')
                };
            })
        },
        quail: {
            common_name: 'Przepiórka',
            incubation_days: 17,
            day_by_day: Array.from({ length: 17 }, (_, i) => {
                const d = i + 1;
                const isLockdown = d >= 15;
                return {
                    day: d,
                    shell_temp: isLockdown ? 37.5 : 37.7 + (d > 8 ? Math.min((d - 8) * 0.08, 0.6) : 0),
                    humidity_set: isLockdown ? 65 : 50,
                    humidity_range: isLockdown ? [60, 75] : [45, 60],
                    turning: !isLockdown,
                    sensitivity: d <= 3 ? 'CRITICAL' : (d <= 5 || isLockdown ? 'HIGH' : 'MEDIUM')
                };
            })
        }
    };

    const SENSITIVITY_COLORS = {
        'CRITICAL': '#e05050',
        'HIGH': '#e8b830',
        'MEDIUM': '#70d070'
    };

    const SENSITIVITY_LABELS = {
        'CRITICAL': 'KRYTYCZNA',
        'HIGH': 'WRAŻLIWA',
        'MEDIUM': 'NORMALNA'
    };

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const species1 = document.getElementById('species1');
    const species2 = document.getElementById('species2');
    const offset1 = document.getElementById('offset1');
    const offset2 = document.getElementById('offset2');
    const daySlider = document.getElementById('daySlider');
    const dayDisplay = document.getElementById('dayDisplay');

    const tempCanvas = document.getElementById('tempChart');
    const humCanvas = document.getElementById('humChart');
    const tempLegend = document.getElementById('tempLegend');
    const humLegend = document.getElementById('humLegend');

    const updateBtn = document.getElementById('updateBtn');
    const exportBtn = document.getElementById('exportBtn');
    const resetBtn = document.getElementById('resetBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  STAN
    // ============================================================
    let currentDay = 10;
    let sim1 = { species: 'chicken', offset: 0, profile: PROFILES.chicken };
    let sim2 = { species: 'duck_indian_runner', offset: 5, profile: PROFILES.duck_indian_runner };

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getProfile(speciesId) {
        return PROFILES[speciesId] || PROFILES.chicken;
    }

    function getDayData(profile, day) {
        if (!profile || !profile.day_by_day) return null;
        const found = profile.day_by_day.find(d => d.day === day);
        return found || null;
    }

    function getTolerance(sensitivity, metric) {
        const tol = {
            temp: { 'CRITICAL': 0.3, 'HIGH': 0.6, 'MEDIUM': 1.0 },
            humidity: { 'CRITICAL': 3, 'HIGH': 5, 'MEDIUM': 8 }
        };
        return tol[metric]?.[sensitivity] || (metric === 'temp' ? 1.0 : 8);
    }

    function updateAll() {
        // Odczytaj konfigurację
        sim1.species = species1.value;
        sim2.species = species2.value;
        sim1.offset = parseInt(offset1.value) || 0;
        sim2.offset = parseInt(offset2.value) || 0;
        sim1.profile = getProfile(sim1.species);
        sim2.profile = getProfile(sim2.species);

        currentDay = parseInt(daySlider.value) || 10;
        dayDisplay.textContent = currentDay;

        updateInfoCards();
        drawCharts();
        updateComparison();
    }

    function updateInfoCards() {
        updateInfoCard(1);
        updateInfoCard(2);
    }

    function updateInfoCard(n) {
        const sim = n === 1 ? sim1 : sim2;
        const profile = sim.profile;
        const displayDay = currentDay - sim.offset;

        const prefix = 'info' + n;
        document.getElementById(prefix + 'Name').textContent = profile.common_name;

        const totalDays = profile.incubation_days;
        const dayData = getDayData(profile, Math.max(1, Math.min(displayDay, totalDays)));

        document.getElementById(prefix + 'Day').textContent = displayDay + '/' + totalDays;

        if (!dayData) {
            document.getElementById(prefix + 'Temp').textContent = '--°C';
            document.getElementById(prefix + 'Hum').textContent = '--%';
            return;
        }

        const tempTol = getTolerance(dayData.sensitivity, 'temp');
        const humTol = getTolerance(dayData.sensitivity, 'humidity');

        document.getElementById(prefix + 'Temp').textContent = dayData.shell_temp.toFixed(2) + '°C';
        document.getElementById(prefix + 'TempRange').textContent = '±' + tempTol + '°C';
        document.getElementById(prefix + 'Hum').textContent = dayData.humidity_set + '%';
        document.getElementById(prefix + 'HumRange').textContent = dayData.humidity_range.join('-') + '%';

        const sensColor = SENSITIVITY_COLORS[dayData.sensitivity] || '#70d070';
        const sensLabel = SENSITIVITY_LABELS[dayData.sensitivity] || 'NORMALNA';
        document.getElementById(prefix + 'Sens').innerHTML = '<span style="color:' + sensColor + ';">' + sensLabel + '</span>';
        document.getElementById(prefix + 'Turn').textContent = dayData.turning ? '✓ TAK' : '✗ NIE';

        // Alarm
        const alarmEl = document.getElementById('alarm' + n);
        const isLockdown = !dayData.turning;
        if (isLockdown) {
            alarmEl.textContent = '⚠ LOCKDOWN – nie otwierać!';
            alarmEl.className = 'alarm-box warning';
        } else if (dayData.sensitivity === 'CRITICAL') {
            alarmEl.textContent = '⚠ FAZA KRYTYCZNA – ostrożność!';
            alarmEl.className = 'alarm-box warning';
        } else {
            alarmEl.textContent = '✓ Wszystko OK';
            alarmEl.className = 'alarm-box ok';
        }
    }

    function updateComparison() {
        const d1 = getDayData(sim1.profile, Math.max(1, currentDay - sim1.offset));
        const d2 = getDayData(sim2.profile, Math.max(1, currentDay - sim2.offset));

        if (!d1 || !d2) {
            document.getElementById('compTemp').textContent = '-- do --°C';
            document.getElementById('compHum').textContent = '-- do --%';
            document.getElementById('compStatus').textContent = 'Brak danych';
            return;
        }

        const tMin = Math.max(
            d1.shell_temp - getTolerance(d1.sensitivity, 'temp'),
            d2.shell_temp - getTolerance(d2.sensitivity, 'temp')
        );
        const tMax = Math.min(
            d1.shell_temp + getTolerance(d1.sensitivity, 'temp'),
            d2.shell_temp + getTolerance(d2.sensitivity, 'temp')
        );

        const hMin = Math.max(
            d1.humidity_set - getTolerance(d1.sensitivity, 'humidity'),
            d2.humidity_set - getTolerance(d2.sensitivity, 'humidity')
        );
        const hMax = Math.min(
            d1.humidity_set + getTolerance(d1.sensitivity, 'humidity'),
            d2.humidity_set + getTolerance(d2.sensitivity, 'humidity')
        );

        const tempOk = tMin <= tMax;
        const humOk = hMin <= hMax;
        const compatible = tempOk && humOk;

        document.getElementById('compTemp').textContent = (tempOk ? tMin.toFixed(1) + ' do ' + tMax.toFixed(1) : '---') + '°C';
        document.getElementById('compHum').textContent = (humOk ? hMin.toFixed(0) + ' do ' + hMax.toFixed(0) : '---') + '%';
        document.getElementById('compStatus').innerHTML = compatible ?
            '<span style="color:#70d070;">✓ KOMPATYBILNE</span>' :
            '<span style="color:#e05050;">✗ KONFLIKT</span>';
        document.getElementById('compDesc').textContent = compatible ?
            'Oba gatunki mogą być hodowane razem w tych warunkach.' :
            'Zakresy warunków się nie pokrywają – rozważ osobne lęgi.';
    }

    // ============================================================
    //  WYKRESY CANVAS
    // ============================================================
    function drawCharts() {
        drawChart(tempCanvas, 'temp');
        drawChart(humCanvas, 'humidity');
    }

    function drawChart(canvas, metric) {
        const ctx = canvas.getContext('2d');
        const rect = canvas.getBoundingClientRect();
        const w = rect.width || 400;
        const h = rect.height || 200;

        canvas.width = w * 2;
        canvas.height = h * 2;
        canvas.style.width = w + 'px';
        canvas.style.height = h + 'px';
        ctx.scale(2, 2);

        ctx.clearRect(0, 0, w, h);

        const pad = { top: 16, right: 16, bottom: 24, left: 32 };
        const plotW = w - pad.left - pad.right;
        const plotH = h - pad.top - pad.bottom;

        const maxD = Math.max(sim1.profile.incubation_days, sim2.profile.incubation_days);
        const minD = 1;

        const mapX = (day) => pad.left + ((day - minD) / (maxD - minD)) * plotW;

        let minVal, maxVal;
        if (metric === 'temp') {
            minVal = 36.0;
            maxVal = 39.5;
        } else {
            minVal = 30;
            maxVal = 80;
        }
        const mapY = (val) => pad.top + plotH - ((val - minVal) / (maxVal - minVal)) * plotH;

        // Tło
        ctx.fillStyle = 'rgba(12,14,18,0.5)';
        ctx.fillRect(0, 0, w, h);

        // Grid
        ctx.strokeStyle = 'rgba(26,30,36,0.3)';
        ctx.lineWidth = 0.5;
        const step = metric === 'temp' ? 0.5 : 10;
        for (let i = minVal; i <= maxVal; i += step) {
            const y = mapY(i);
            ctx.beginPath();
            ctx.moveTo(pad.left, y);
            ctx.lineTo(w - pad.right, y);
            ctx.stroke();
        }

        // Osie
        ctx.strokeStyle = '#2a3240';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(pad.left, pad.top);
        ctx.lineTo(pad.left, h - pad.bottom);
        ctx.lineTo(w - pad.right, h - pad.bottom);
        ctx.stroke();

        // Etykiety osi Y
        ctx.fillStyle = '#5a6a7a';
        ctx.font = '8px monospace';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        for (let i = minVal; i <= maxVal; i += step * 2) {
            const y = mapY(i);
            ctx.fillText(i.toFixed(metric === 'temp' ? 1 : 0), pad.left - 4, y);
        }

        // Etykiety osi X
        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        for (let d = 1; d <= maxD; d += Math.max(1, Math.floor(maxD / 10))) {
            const x = mapX(d);
            ctx.fillText(d, x, h - pad.bottom + 4);
        }

        // Rysuj profile
        const sims = [sim1, sim2];
        const colors = ['rgba(57,170,255,', 'rgba(255,100,100,'];

        sims.forEach(function(sim, idx) {
            const profile = sim.profile;
            const color = colors[idx];

            profile.day_by_day.forEach(function(dayData) {
                const d = dayData.day;
                const displayDay = d + sim.offset;
                if (displayDay < minD || displayDay > maxD) return;

                const x = mapX(displayDay);
                const val = metric === 'temp' ? dayData.shell_temp : dayData.humidity_set;
                if (!val) return;

                const tol = getTolerance(dayData.sensitivity, metric);
                const low = val - tol;
                const high = val + tol;
                const yLow = mapY(low);
                const yHigh = mapY(high);
                const height = yLow - yHigh;

                const alpha = dayData.sensitivity === 'CRITICAL' ? '0.35)' :
                              dayData.sensitivity === 'HIGH' ? '0.2)' : '0.12)';

                ctx.fillStyle = color + alpha;
                ctx.fillRect(x - 2, yHigh, 4, height);

                // Linia idealna
                ctx.strokeStyle = color + '0.9)';
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                const yIdeal = mapY(val);
                ctx.moveTo(x - 2, yIdeal);
                ctx.lineTo(x + 2, yIdeal);
                ctx.stroke();
            });
        });

        // Linia bieżącego dnia
        const xCurrent = mapX(currentDay);
        ctx.strokeStyle = 'rgba(255,255,255,0.2)';
        ctx.lineWidth = 1;
        ctx.setLineDash([2, 3]);
        ctx.beginPath();
        ctx.moveTo(xCurrent, pad.top);
        ctx.lineTo(xCurrent, pad.top + plotH);
        ctx.stroke();
        ctx.setLineDash([]);

        // Legenda
        const d1 = getDayData(sim1.profile, Math.max(1, currentDay - sim1.offset));
        const d2 = getDayData(sim2.profile, Math.max(1, currentDay - sim2.offset));
        const val1 = d1 ? (metric === 'temp' ? d1.shell_temp : d1.humidity_set) : '--';
        const val2 = d2 ? (metric === 'temp' ? d2.shell_temp : d2.humidity_set) : '--';
        const legend = 'SIM1: ' + val1 + (metric === 'temp' ? '°C' : '%') + ' | SIM2: ' + val2 + (metric === 'temp' ? '°C' : '%');
        const legendEl = metric === 'temp' ? tempLegend : humLegend;
        if (legendEl) legendEl.textContent = legend;
    }

    // ============================================================
    //  FUNKCJE AKCJI
    // ============================================================
    function exportCSV() {
        let csv = 'Dzień,Kura(Temp),Kura(RH%),Kaczka(Temp),Kaczka(RH%)\n';
        const maxD = Math.max(sim1.profile.incubation_days, sim2.profile.incubation_days);

        for (let d = 1; d <= maxD; d++) {
            const d1 = getDayData(sim1.profile, d);
            const d2 = getDayData(sim2.profile, d);
            csv += d + ',' +
                   (d1 ? d1.shell_temp.toFixed(2) : '--') + ',' +
                   (d1 ? d1.humidity_set : '--') + ',' +
                   (d2 ? d2.shell_temp.toFixed(2) : '--') + ',' +
                   (d2 ? d2.humidity_set : '--') + '\n';
        }

        const blob = new Blob([csv], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'analiza_legow_' + Date.now() + '.csv';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        ink.setStatus('Pobrano plik CSV', 'ok');
    }

    function resetAll() {
        species1.value = 'chicken';
        species2.value = 'duck_indian_runner';
        offset1.value = 0;
        offset2.value = 5;
        daySlider.value = 10;
        dayDisplay.textContent = 10;
        currentDay = 10;
        updateAll();
        ink.setStatus('Reset ustawień', 'info');
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono', 'ok');
            return;
        }
        // Ta strona nie wymaga aktywnej komunikacji z ESP
    });

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        // Event listenery
        species1.addEventListener('change', updateAll);
        species2.addEventListener('change', updateAll);
        offset1.addEventListener('change', updateAll);
        offset2.addEventListener('change', updateAll);
        daySlider.addEventListener('input', function() {
            dayDisplay.textContent = this.value;
            updateAll();
        });

        if (updateBtn) updateBtn.addEventListener('click', updateAll);
        if (exportBtn) exportBtn.addEventListener('click', exportCSV);
        if (resetBtn) resetBtn.addEventListener('click', resetAll);
        if (backBtn) backBtn.addEventListener('click', goBack);

        // Inicjalizacja
        updateAll();

        // Obsługa resize dla wykresów
        let resizeTimeout;
        window.addEventListener('resize', function() {
            clearTimeout(resizeTimeout);
            resizeTimeout = setTimeout(function() {
                drawCharts();
            }, 200);
        });

        console.log('[symulacja_legow.js] Zainicjalizowano');
    });

})();