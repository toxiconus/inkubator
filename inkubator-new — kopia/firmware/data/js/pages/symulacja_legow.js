(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[symulacja_legow.js] Brak wspolnego modulu Inkubator');
        return;
    }

    const STORAGE_KEY = 'inkubator.simLegow.v1';

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
            common_name: 'Ges',
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
            common_name: 'Przepiorka',
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
        CRITICAL: '#e05050',
        HIGH: '#e8b830',
        MEDIUM: '#70d070'
    };

    const SENSITIVITY_LABELS = {
        CRITICAL: 'KRYTYCZNA',
        HIGH: 'WRAZLIWA',
        MEDIUM: 'NORMALNA'
    };

    const SCENARIOS = {
        normal: { temp: 0, hum: 0 },
        heat_stress: { temp: 0.6, hum: -6 },
        cold_stress: { temp: -0.6, hum: 4 },
        dry: { temp: 0.1, hum: -10 },
        wet: { temp: -0.1, hum: 10 }
    };

    const DEFAULT_Y_RANGES = {
        temp: { min: 35.5, max: 40.5 },
        humidity: { min: 25, max: 90 }
    };

    const species1 = document.getElementById('species1');
    const species2 = document.getElementById('species2');
    const offset1 = document.getElementById('offset1');
    const offset2 = document.getElementById('offset2');
    const daySlider = document.getElementById('daySlider');
    const dayDisplay = document.getElementById('dayDisplay');

    const tempCanvas = document.getElementById('tempChart');
    const humCanvas = document.getElementById('humChart');
    const tempChartBox = document.getElementById('tempChartBox');
    const humChartBox = document.getElementById('humChartBox');
    const chartContainer = document.querySelector('.chart-container');
    const tempLegend = document.getElementById('tempLegend');
    const humLegend = document.getElementById('humLegend');
    const tempYRange = document.getElementById('tempYRange');
    const humYRange = document.getElementById('humYRange');

    const updateBtn = document.getElementById('updateBtn');
    const exportBtn = document.getElementById('exportBtn');
    const resetBtn = document.getElementById('resetBtn');
    const backBtn = document.getElementById('backBtn');
    const playBtn = document.getElementById('playBtn');

    const scenarioSelect = document.getElementById('scenarioSelect');
    const noiseLevel = document.getElementById('noiseLevel');
    const noiseLevelVal = document.getElementById('noiseLevelVal');
    const tempBias = document.getElementById('tempBias');
    const tempBiasVal = document.getElementById('tempBiasVal');
    const humBias = document.getElementById('humBias');
    const humBiasVal = document.getElementById('humBiasVal');
    const chartHeightRange = document.getElementById('chartHeightRange');
    const chartHeightValue = document.getElementById('chartHeightValue');
    const expandTempBtn = document.getElementById('expandTempBtn');
    const expandHumBtn = document.getElementById('expandHumBtn');
    const resetZoomBtn = document.getElementById('resetZoomBtn');
    const tempZoomInBtn = document.getElementById('tempZoomInBtn');
    const tempZoomOutBtn = document.getElementById('tempZoomOutBtn');
    const tempZoomResetBtn = document.getElementById('tempZoomResetBtn');
    const humZoomInBtn = document.getElementById('humZoomInBtn');
    const humZoomOutBtn = document.getElementById('humZoomOutBtn');
    const humZoomResetBtn = document.getElementById('humZoomResetBtn');

    let currentDay = 10;
    let sim1 = { species: 'chicken', offset: 0, profile: PROFILES.chicken };
    let sim2 = { species: 'duck_indian_runner', offset: 5, profile: PROFILES.duck_indian_runner };

    const simConfig = {
        scenario: 'normal',
        noise: 0.15,
        tempBias: 0,
        humBias: 0,
        autoRun: false,
        chartHeight: 220,
        expandedChart: 'none',
        yRanges: {
            temp: { ...DEFAULT_Y_RANGES.temp },
            humidity: { ...DEFAULT_Y_RANGES.humidity }
        }
    };

    let autoTimer = null;

    function clamp(value, min, max) {
        return Math.min(max, Math.max(min, value));
    }

    function getProfile(speciesId) {
        return PROFILES[speciesId] || PROFILES.chicken;
    }

    function getDayData(profile, day) {
        if (!profile || !profile.day_by_day) return null;
        return profile.day_by_day.find(d => d.day === day) || null;
    }

    function getTolerance(sensitivity, metric) {
        const tol = {
            temp: { CRITICAL: 0.3, HIGH: 0.6, MEDIUM: 1.0 },
            humidity: { CRITICAL: 3, HIGH: 5, MEDIUM: 8 }
        };
        return tol[metric]?.[sensitivity] || (metric === 'temp' ? 1.0 : 8);
    }

    function pseudoNoise(day, seed) {
        const x = Math.sin((day * 12.9898) + (seed * 78.233)) * 43758.5453;
        return (x - Math.floor(x)) * 2 - 1;
    }

    function getScenarioOffsets() {
        return SCENARIOS[simConfig.scenario] || SCENARIOS.normal;
    }

    function getEffectiveDayData(sim, targetDay) {
        const profile = sim.profile;
        const totalDays = profile.incubation_days;
        const clampedDay = clamp(targetDay, 1, totalDays);
        const base = getDayData(profile, clampedDay);
        if (!base) return null;

        const scenario = getScenarioOffsets();
        const noiseAmpTemp = simConfig.noise;
        const noiseAmpHum = simConfig.noise * 8;
        const noiseT = pseudoNoise(clampedDay, sim.offset + (sim.species === 'chicken' ? 1 : 2));
        const noiseH = pseudoNoise(clampedDay, sim.offset + (sim.species === 'goose' ? 7 : 11));

        const simulatedTemp = clamp(
            base.shell_temp + scenario.temp + simConfig.tempBias + (noiseT * noiseAmpTemp),
            35.0,
            40.5
        );
        const simulatedHum = clamp(
            base.humidity_set + scenario.hum + simConfig.humBias + (noiseH * noiseAmpHum),
            20,
            95
        );

        const lower = clamp(simulatedHum - getTolerance(base.sensitivity, 'humidity'), 20, 95);
        const upper = clamp(simulatedHum + getTolerance(base.sensitivity, 'humidity'), 20, 95);

        return {
            ...base,
            shell_temp: simulatedTemp,
            humidity_set: Math.round(simulatedHum),
            humidity_range: [Math.round(lower), Math.round(upper)]
        };
    }

    function setScenarioValueLabels() {
        noiseLevelVal.textContent = Number(simConfig.noise).toFixed(2);
        tempBiasVal.textContent = Number(simConfig.tempBias).toFixed(1) + ' C';
        humBiasVal.textContent = Number(simConfig.humBias).toFixed(0) + ' %';
        if (chartHeightValue) chartHeightValue.textContent = String(simConfig.chartHeight) + ' px';
    }

    function updateYAxisLabels() {
        if (tempYRange) {
            tempYRange.textContent = 'Y: ' + simConfig.yRanges.temp.min.toFixed(1) + '-' + simConfig.yRanges.temp.max.toFixed(1) + ' C';
        }
        if (humYRange) {
            humYRange.textContent = 'Y: ' + Math.round(simConfig.yRanges.humidity.min) + '-' + Math.round(simConfig.yRanges.humidity.max) + ' %';
        }
    }

    function applyChartLayout() {
        if (!chartContainer) return;

        chartContainer.style.setProperty('--chart-height', String(simConfig.chartHeight) + 'px');

        chartContainer.classList.remove('focus-temp', 'focus-humidity');
        if (tempChartBox) tempChartBox.classList.remove('is-expanded');
        if (humChartBox) humChartBox.classList.remove('is-expanded');

        if (simConfig.expandedChart === 'temp') {
            chartContainer.classList.add('focus-temp');
            if (tempChartBox) tempChartBox.classList.add('is-expanded');
        } else if (simConfig.expandedChart === 'humidity') {
            chartContainer.classList.add('focus-humidity');
            if (humChartBox) humChartBox.classList.add('is-expanded');
        }

        if (expandTempBtn) expandTempBtn.classList.toggle('active', simConfig.expandedChart === 'temp');
        if (expandHumBtn) expandHumBtn.classList.toggle('active', simConfig.expandedChart === 'humidity');
        updateYAxisLabels();
    }

    function syncControlsFromState() {
        species1.value = sim1.species;
        species2.value = sim2.species;
        offset1.value = sim1.offset;
        offset2.value = sim2.offset;
        daySlider.value = currentDay;
        dayDisplay.textContent = currentDay;

        scenarioSelect.value = simConfig.scenario;
        noiseLevel.value = simConfig.noise;
        tempBias.value = simConfig.tempBias;
        humBias.value = simConfig.humBias;
        if (chartHeightRange) chartHeightRange.value = simConfig.chartHeight;
        setScenarioValueLabels();
        applyChartLayout();

        playBtn.textContent = simConfig.autoRun ? 'Auto-run ON' : 'Auto-run OFF';
    }

    function persistConfig() {
        try {
            localStorage.setItem(STORAGE_KEY, JSON.stringify({
                currentDay,
                sim1Species: sim1.species,
                sim1Offset: sim1.offset,
                sim2Species: sim2.species,
                sim2Offset: sim2.offset,
                scenario: simConfig.scenario,
                noise: simConfig.noise,
                tempBias: simConfig.tempBias,
                humBias: simConfig.humBias,
                autoRun: simConfig.autoRun,
                chartHeight: simConfig.chartHeight,
                expandedChart: simConfig.expandedChart,
                yRanges: simConfig.yRanges
            }));
        } catch (err) {
            console.warn('[symulacja_legow.js] localStorage save failed', err);
        }
    }

    function loadConfig() {
        try {
            const raw = localStorage.getItem(STORAGE_KEY);
            if (!raw) return;
            const parsed = JSON.parse(raw);

            currentDay = clamp(Number(parsed.currentDay) || currentDay, 1, 30);
            sim1.species = parsed.sim1Species || sim1.species;
            sim2.species = parsed.sim2Species || sim2.species;
            sim1.offset = clamp(Number(parsed.sim1Offset) || 0, 0, 10);
            sim2.offset = clamp(Number(parsed.sim2Offset) || 0, 0, 10);
            sim1.profile = getProfile(sim1.species);
            sim2.profile = getProfile(sim2.species);

            simConfig.scenario = parsed.scenario && SCENARIOS[parsed.scenario] ? parsed.scenario : 'normal';
            simConfig.noise = clamp(Number(parsed.noise), 0, 0.8);
            simConfig.tempBias = clamp(Number(parsed.tempBias), -1.5, 1.5);
            simConfig.humBias = clamp(Number(parsed.humBias), -15, 15);
            simConfig.autoRun = Boolean(parsed.autoRun);
            simConfig.chartHeight = clamp(Number(parsed.chartHeight), 160, 420);
            simConfig.expandedChart = parsed.expandedChart === 'temp' || parsed.expandedChart === 'humidity'
                ? parsed.expandedChart
                : 'none';
            if (parsed.yRanges && parsed.yRanges.temp && parsed.yRanges.humidity) {
                simConfig.yRanges.temp.min = Number(parsed.yRanges.temp.min);
                simConfig.yRanges.temp.max = Number(parsed.yRanges.temp.max);
                simConfig.yRanges.humidity.min = Number(parsed.yRanges.humidity.min);
                simConfig.yRanges.humidity.max = Number(parsed.yRanges.humidity.max);
            }

            if (!Number.isFinite(simConfig.noise)) simConfig.noise = 0.15;
            if (!Number.isFinite(simConfig.tempBias)) simConfig.tempBias = 0;
            if (!Number.isFinite(simConfig.humBias)) simConfig.humBias = 0;
            if (!Number.isFinite(simConfig.chartHeight)) simConfig.chartHeight = 220;
            if (!Number.isFinite(simConfig.yRanges.temp.min) || !Number.isFinite(simConfig.yRanges.temp.max)) {
                simConfig.yRanges.temp = { ...DEFAULT_Y_RANGES.temp };
            }
            if (!Number.isFinite(simConfig.yRanges.humidity.min) || !Number.isFinite(simConfig.yRanges.humidity.max)) {
                simConfig.yRanges.humidity = { ...DEFAULT_Y_RANGES.humidity };
            }
        } catch (err) {
            console.warn('[symulacja_legow.js] localStorage load failed', err);
        }
    }

    function resetChartRanges() {
        simConfig.yRanges.temp = { ...DEFAULT_Y_RANGES.temp };
        simConfig.yRanges.humidity = { ...DEFAULT_Y_RANGES.humidity };
    }

    function resetChartRange(metric) {
        if (metric === 'temp') {
            simConfig.yRanges.temp = { ...DEFAULT_Y_RANGES.temp };
        } else if (metric === 'humidity') {
            simConfig.yRanges.humidity = { ...DEFAULT_Y_RANGES.humidity };
        }
        updateYAxisLabels();
        persistConfig();
        drawCharts();
    }

    function zoomYAxis(metric, deltaY, pointerRatio) {
        const range = simConfig.yRanges[metric];
        if (!range) return;

        const cfg = metric === 'temp'
            ? { absMin: 34, absMax: 42, minSpan: 0.8, maxSpan: 10 }
            : { absMin: 0, absMax: 100, minSpan: 10, maxSpan: 100 };

        const span = range.max - range.min;
        const zoomFactor = deltaY < 0 ? 0.9 : 1.1;
        let newSpan = clamp(span * zoomFactor, cfg.minSpan, cfg.maxSpan);

        const ratio = clamp(pointerRatio, 0, 1);
        const anchorValue = range.max - (ratio * span);

        let newMin = anchorValue - ((1 - ratio) * newSpan);
        let newMax = anchorValue + (ratio * newSpan);

        if (newMin < cfg.absMin) {
            const shift = cfg.absMin - newMin;
            newMin += shift;
            newMax += shift;
        }
        if (newMax > cfg.absMax) {
            const shift = newMax - cfg.absMax;
            newMin -= shift;
            newMax -= shift;
        }

        if (newMax - newMin < cfg.minSpan) {
            const mid = (newMax + newMin) / 2;
            newMin = mid - (cfg.minSpan / 2);
            newMax = mid + (cfg.minSpan / 2);
        }

        range.min = clamp(newMin, cfg.absMin, cfg.absMax);
        range.max = clamp(newMax, cfg.absMin, cfg.absMax);

        updateYAxisLabels();
        persistConfig();
        drawCharts();
    }

    function zoomChartStep(metric, zoomIn) {
        zoomYAxis(metric, zoomIn ? -1 : 1, 0.5);
    }

    function setExpandedChart(mode) {
        if (mode !== 'temp' && mode !== 'humidity' && mode !== 'none') return;
        simConfig.expandedChart = simConfig.expandedChart === mode ? 'none' : mode;
        applyChartLayout();
        persistConfig();
        drawCharts();
    }

    function setChartHeight(value) {
        simConfig.chartHeight = clamp(Number(value) || 220, 160, 420);
        if (chartHeightRange) chartHeightRange.value = simConfig.chartHeight;
        setScenarioValueLabels();
        applyChartLayout();
        persistConfig();
        drawCharts();
    }

    function updateInfoCards() {
        updateInfoCard(1);
        updateInfoCard(2);
    }

    function updateInfoCard(n) {
        const sim = n === 1 ? sim1 : sim2;
        const profile = sim.profile;
        const displayDay = currentDay - sim.offset;
        const clampedDay = clamp(displayDay, 1, profile.incubation_days);

        const prefix = 'info' + n;
        document.getElementById(prefix + 'Name').textContent = profile.common_name;
        document.getElementById(prefix + 'Day').textContent = displayDay + '/' + profile.incubation_days;

        const dayData = getEffectiveDayData(sim, clampedDay);
        if (!dayData) {
            document.getElementById(prefix + 'Temp').textContent = '-- C';
            document.getElementById(prefix + 'TempRange').textContent = '--';
            document.getElementById(prefix + 'Hum').textContent = '-- %';
            document.getElementById(prefix + 'HumRange').textContent = '--';
            document.getElementById(prefix + 'Sens').textContent = '--';
            document.getElementById(prefix + 'Turn').textContent = '--';
            return;
        }

        const tempTol = getTolerance(dayData.sensitivity, 'temp');
        const sensColor = SENSITIVITY_COLORS[dayData.sensitivity] || '#70d070';
        const sensLabel = SENSITIVITY_LABELS[dayData.sensitivity] || 'NORMALNA';

        document.getElementById(prefix + 'Temp').textContent = dayData.shell_temp.toFixed(2) + ' C';
        document.getElementById(prefix + 'TempRange').textContent = '+/- ' + tempTol.toFixed(1) + ' C';
        document.getElementById(prefix + 'Hum').textContent = dayData.humidity_set + ' %';
        document.getElementById(prefix + 'HumRange').textContent = dayData.humidity_range.join('-') + ' %';
        document.getElementById(prefix + 'Sens').innerHTML = '<span style="color:' + sensColor + ';">' + sensLabel + '</span>';
        document.getElementById(prefix + 'Turn').textContent = dayData.turning ? 'TAK' : 'NIE';

        const alarmEl = document.getElementById('alarm' + n);
        const isLockdown = !dayData.turning;
        if (isLockdown) {
            alarmEl.textContent = 'LOCKDOWN - nie otwieraj komory bez potrzeby';
            alarmEl.className = 'alarm-box warning';
        } else if (dayData.sensitivity === 'CRITICAL') {
            alarmEl.textContent = 'Faza krytyczna - trzymaj stabilne warunki';
            alarmEl.className = 'alarm-box warning';
        } else {
            alarmEl.textContent = 'Warunki stabilne';
            alarmEl.className = 'alarm-box ok';
        }
    }

    function updateComparison() {
        const day1 = clamp(currentDay - sim1.offset, 1, sim1.profile.incubation_days);
        const day2 = clamp(currentDay - sim2.offset, 1, sim2.profile.incubation_days);
        const d1 = getEffectiveDayData(sim1, day1);
        const d2 = getEffectiveDayData(sim2, day2);

        if (!d1 || !d2) {
            document.getElementById('compTemp').textContent = '-- C do -- C';
            document.getElementById('compHum').textContent = '--% do --%';
            document.getElementById('compStatus').textContent = 'Brak danych';
            document.getElementById('compDesc').textContent = 'Brak danych do porownania.';
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

        document.getElementById('compTemp').textContent = (tempOk ? tMin.toFixed(1) + ' do ' + tMax.toFixed(1) : 'BRAK') + ' C';
        document.getElementById('compHum').textContent = (humOk ? hMin.toFixed(0) + ' do ' + hMax.toFixed(0) : 'BRAK') + ' %';
        document.getElementById('compStatus').innerHTML = compatible ?
            '<span style="color:#70d070;">KOMPATYBILNE</span>' :
            '<span style="color:#e05050;">KONFLIKT</span>';
        document.getElementById('compDesc').textContent = compatible ?
            'Zakresy wspolne sa poprawne. Mozliwa koinkubacja.' :
            'Zakresy warunkow sie rozjezdzaja. Rozwaz osobne cykle.';
    }

    function drawCharts() {
        drawChart(tempCanvas, 'temp');
        drawChart(humCanvas, 'humidity');
    }

    function drawChart(canvas, metric) {
        if (!canvas) return;

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

        const maxD = Math.max(sim1.profile.incubation_days + sim1.offset, sim2.profile.incubation_days + sim2.offset);
        const minD = 1;
        const mapX = day => pad.left + ((day - minD) / Math.max(1, maxD - minD)) * plotW;

        const minVal = simConfig.yRanges[metric].min;
        const maxVal = simConfig.yRanges[metric].max;
        const mapY = val => pad.top + plotH - ((val - minVal) / (maxVal - minVal)) * plotH;

        ctx.fillStyle = 'rgba(12,14,18,0.5)';
        ctx.fillRect(0, 0, w, h);

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

        ctx.strokeStyle = '#2a3240';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(pad.left, pad.top);
        ctx.lineTo(pad.left, h - pad.bottom);
        ctx.lineTo(w - pad.right, h - pad.bottom);
        ctx.stroke();

        ctx.fillStyle = '#5a6a7a';
        ctx.font = '8px monospace';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        for (let i = minVal; i <= maxVal; i += step * 2) {
            ctx.fillText(i.toFixed(metric === 'temp' ? 1 : 0), pad.left - 4, mapY(i));
        }

        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        const xStep = Math.max(1, Math.floor(maxD / 10));
        for (let d = 1; d <= maxD; d += xStep) {
            ctx.fillText(String(d), mapX(d), h - pad.bottom + 4);
        }

        const sims = [sim1, sim2];
        const colors = ['rgba(57,170,255,', 'rgba(255,100,100,'];

        sims.forEach((sim, idx) => {
            const color = colors[idx];
            sim.profile.day_by_day.forEach(dayData => {
                const displayDay = dayData.day + sim.offset;
                if (displayDay < minD || displayDay > maxD) return;

                const effective = getEffectiveDayData(sim, dayData.day);
                if (!effective) return;

                const x = mapX(displayDay);
                const val = metric === 'temp' ? effective.shell_temp : effective.humidity_set;
                const tol = getTolerance(effective.sensitivity, metric);
                const yLow = mapY(val - tol);
                const yHigh = mapY(val + tol);
                const alpha = effective.sensitivity === 'CRITICAL' ? '0.35)' : effective.sensitivity === 'HIGH' ? '0.2)' : '0.12)';

                ctx.fillStyle = color + alpha;
                ctx.fillRect(x - 2, yHigh, 4, yLow - yHigh);

                ctx.strokeStyle = color + '0.9)';
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                const yIdeal = mapY(val);
                ctx.moveTo(x - 2, yIdeal);
                ctx.lineTo(x + 2, yIdeal);
                ctx.stroke();
            });
        });

        const xCurrent = mapX(currentDay);
        ctx.strokeStyle = 'rgba(255,255,255,0.25)';
        ctx.lineWidth = 1;
        ctx.setLineDash([2, 3]);
        ctx.beginPath();
        ctx.moveTo(xCurrent, pad.top);
        ctx.lineTo(xCurrent, pad.top + plotH);
        ctx.stroke();
        ctx.setLineDash([]);

        const d1 = getEffectiveDayData(sim1, clamp(currentDay - sim1.offset, 1, sim1.profile.incubation_days));
        const d2 = getEffectiveDayData(sim2, clamp(currentDay - sim2.offset, 1, sim2.profile.incubation_days));
        const val1 = d1 ? (metric === 'temp' ? d1.shell_temp.toFixed(2) + ' C' : d1.humidity_set + ' %') : '--';
        const val2 = d2 ? (metric === 'temp' ? d2.shell_temp.toFixed(2) + ' C' : d2.humidity_set + ' %') : '--';
        const legendEl = metric === 'temp' ? tempLegend : humLegend;
        if (legendEl) legendEl.textContent = 'SIM1: ' + val1 + ' | SIM2: ' + val2;
    }

    function updateAll(showStatus) {
        sim1.species = species1.value;
        sim2.species = species2.value;
        sim1.offset = clamp(parseInt(offset1.value, 10) || 0, 0, 10);
        sim2.offset = clamp(parseInt(offset2.value, 10) || 0, 0, 10);
        sim1.profile = getProfile(sim1.species);
        sim2.profile = getProfile(sim2.species);

        currentDay = clamp(parseInt(daySlider.value, 10) || 10, 1, 30);
        dayDisplay.textContent = currentDay;

        simConfig.scenario = scenarioSelect.value;
        simConfig.noise = clamp(Number(noiseLevel.value), 0, 0.8);
        simConfig.tempBias = clamp(Number(tempBias.value), -1.5, 1.5);
        simConfig.humBias = clamp(Number(humBias.value), -15, 15);
        setScenarioValueLabels();

        updateInfoCards();
        drawCharts();
        updateComparison();
        persistConfig();

        if (showStatus) {
            ink.setStatus('Symulacja zaktualizowana', 'ok');
        }
    }

    function exportCSV() {
        const lines = [];
        lines.push('Dzien,Gatunek1,Temp1_C,RH1_pct,Gatunek2,Temp2_C,RH2_pct,Scenariusz,Noise,TempBias,HumBias');

        const maxD = Math.max(sim1.profile.incubation_days + sim1.offset, sim2.profile.incubation_days + sim2.offset);
        for (let d = 1; d <= maxD; d++) {
            const d1 = getEffectiveDayData(sim1, clamp(d - sim1.offset, 1, sim1.profile.incubation_days));
            const d2 = getEffectiveDayData(sim2, clamp(d - sim2.offset, 1, sim2.profile.incubation_days));

            lines.push([
                d,
                sim1.profile.common_name,
                d1 ? d1.shell_temp.toFixed(2) : '--',
                d1 ? d1.humidity_set : '--',
                sim2.profile.common_name,
                d2 ? d2.shell_temp.toFixed(2) : '--',
                d2 ? d2.humidity_set : '--',
                simConfig.scenario,
                simConfig.noise.toFixed(2),
                simConfig.tempBias.toFixed(1),
                simConfig.humBias.toFixed(0)
            ].join(','));
        }

        const blob = new Blob([lines.join('\n')], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'analiza_legow_' + Date.now() + '.csv';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        ink.setStatus('Pobrano CSV z symulacji', 'ok');
    }

    function resetAll() {
        sim1 = { species: 'chicken', offset: 0, profile: PROFILES.chicken };
        sim2 = { species: 'duck_indian_runner', offset: 5, profile: PROFILES.duck_indian_runner };
        currentDay = 10;
        simConfig.scenario = 'normal';
        simConfig.noise = 0.15;
        simConfig.tempBias = 0;
        simConfig.humBias = 0;
        simConfig.autoRun = false;
        simConfig.chartHeight = 220;
        simConfig.expandedChart = 'none';
        resetChartRanges();
        stopAutoRun();
        syncControlsFromState();
        updateAll(true);
    }

    function goBack() {
        window.location.href = 'index.html';
    }

    function startAutoRun() {
        if (autoTimer) return;
        simConfig.autoRun = true;
        playBtn.textContent = 'Auto-run ON';
        autoTimer = setInterval(() => {
            const next = Number(daySlider.value) >= 30 ? 1 : Number(daySlider.value) + 1;
            daySlider.value = next;
            updateAll(false);
        }, 1200);
        persistConfig();
    }

    function stopAutoRun() {
        simConfig.autoRun = false;
        playBtn.textContent = 'Auto-run OFF';
        if (autoTimer) {
            clearInterval(autoTimer);
            autoTimer = null;
        }
        persistConfig();
    }

    function toggleAutoRun() {
        if (simConfig.autoRun) {
            stopAutoRun();
            ink.setStatus('Auto-run zatrzymany', 'info');
        } else {
            startAutoRun();
            ink.setStatus('Auto-run uruchomiony', 'ok');
        }
    }

    ink.onWsEvent(function(type) {
        if (type === 'open') {
            ink.setStatus('Połączono', 'ok');
        }
    });

    document.addEventListener('DOMContentLoaded', function() {
        loadConfig();
        syncControlsFromState();

        ink.setStatus('Tryb lokalny symulacji - ESP nie jest wymagane', 'info');

        species1.addEventListener('change', () => updateAll(true));
        species2.addEventListener('change', () => updateAll(true));
        offset1.addEventListener('change', () => updateAll(true));
        offset2.addEventListener('change', () => updateAll(true));
        scenarioSelect.addEventListener('change', () => updateAll(true));

        noiseLevel.addEventListener('input', () => updateAll(false));
        tempBias.addEventListener('input', () => updateAll(false));
        humBias.addEventListener('input', () => updateAll(false));

        daySlider.addEventListener('input', () => updateAll(false));

        if (updateBtn) updateBtn.addEventListener('click', () => updateAll(true));
        if (exportBtn) exportBtn.addEventListener('click', exportCSV);
        if (resetBtn) resetBtn.addEventListener('click', resetAll);
        if (backBtn) backBtn.addEventListener('click', goBack);
        if (playBtn) playBtn.addEventListener('click', toggleAutoRun);
        if (chartHeightRange) chartHeightRange.addEventListener('input', () => setChartHeight(chartHeightRange.value));
        if (expandTempBtn) expandTempBtn.addEventListener('click', () => setExpandedChart('temp'));
        if (expandHumBtn) expandHumBtn.addEventListener('click', () => setExpandedChart('humidity'));
        if (resetZoomBtn) {
            resetZoomBtn.addEventListener('click', () => {
                simConfig.expandedChart = 'none';
                resetChartRanges();
                setChartHeight(220);
            });
        }

        if (tempZoomInBtn) tempZoomInBtn.addEventListener('click', () => zoomChartStep('temp', true));
        if (tempZoomOutBtn) tempZoomOutBtn.addEventListener('click', () => zoomChartStep('temp', false));
        if (tempZoomResetBtn) tempZoomResetBtn.addEventListener('click', () => resetChartRange('temp'));
        if (humZoomInBtn) humZoomInBtn.addEventListener('click', () => zoomChartStep('humidity', true));
        if (humZoomOutBtn) humZoomOutBtn.addEventListener('click', () => zoomChartStep('humidity', false));
        if (humZoomResetBtn) humZoomResetBtn.addEventListener('click', () => resetChartRange('humidity'));

        if (tempCanvas) {
            tempCanvas.addEventListener('wheel', (event) => {
                event.preventDefault();
                const rect = tempCanvas.getBoundingClientRect();
                const ratio = (event.clientY - rect.top) / Math.max(1, rect.height);
                zoomYAxis('temp', event.deltaY, ratio);
            }, { passive: false });
        }

        if (humCanvas) {
            humCanvas.addEventListener('wheel', (event) => {
                event.preventDefault();
                const rect = humCanvas.getBoundingClientRect();
                const ratio = (event.clientY - rect.top) / Math.max(1, rect.height);
                zoomYAxis('humidity', event.deltaY, ratio);
            }, { passive: false });
        }

        updateAll(false);

        if (simConfig.autoRun) {
            startAutoRun();
        }

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

