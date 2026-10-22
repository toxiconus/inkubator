(function() {
    'use strict';

    const SPECIES = {
        CHICKEN: {
            name: 'Kura',
            days: 21,
            lockdown: 19,
            noRotate: [10, 15, 18],
            phases: [
                { start: 1, end: 18, temp: 37.8, rh: 55, turn: true, notes: 'Normalna inkubacja' },
                { start: 19, end: 21, temp: 37.2, rh: 70, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 16, 17, 18],
                high: [4, 5, 15, 19, 20],
                medium: [6, 7, 8, 9, 10, 11, 12, 13, 14]
            }
        },
        DUCK: {
            name: 'Kaczka',
            days: 28,
            lockdown: 25,
            noRotate: [14, 21, 25],
            phases: [
                { start: 1, end: 24, temp: 37.8, rh: 60, turn: true, notes: 'Normalna inkubacja' },
                { start: 25, end: 28, temp: 37.2, rh: 75, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 21, 22, 23, 24],
                high: [4, 5, 17, 18, 19, 20, 25, 26, 27],
                medium: [6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
            }
        },
        GOOSE: {
            name: 'Gęś',
            days: 30,
            lockdown: 27,
            noRotate: [14, 21, 27],
            phases: [
                { start: 1, end: 26, temp: 37.6, rh: 60, turn: true, notes: 'Normalna inkubacja' },
                { start: 27, end: 30, temp: 37.2, rh: 75, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 23, 24, 25, 26],
                high: [4, 5, 18, 19, 20, 21, 22, 27, 28, 29],
                medium: [6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]
            }
        },
        QUAIL: {
            name: 'Przepiórka',
            days: 17,
            lockdown: 15,
            noRotate: [8, 12, 15],
            phases: [
                { start: 1, end: 14, temp: 37.8, rh: 50, turn: true, notes: 'Normalna inkubacja' },
                { start: 15, end: 17, temp: 37.3, rh: 70, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 14, 15],
                high: [4, 5, 12, 13, 16, 17],
                medium: [6, 7, 8, 9, 10, 11]
            }
        },
        GUINEA: {
            name: 'Perliczka',
            days: 26,
            lockdown: 23,
            noRotate: [12, 18, 23],
            phases: [
                { start: 1, end: 22, temp: 37.7, rh: 55, turn: true, notes: 'Normalna inkubacja' },
                { start: 23, end: 26, temp: 37.2, rh: 70, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 20, 21, 22, 23],
                high: [4, 5, 15, 16, 17, 18, 19, 24, 25, 26],
                medium: [6, 7, 8, 9, 10, 11, 12, 13, 14]
            }
        },
        PHEASANT: {
            name: 'Bażant',
            days: 24,
            lockdown: 21,
            noRotate: [10, 16, 21],
            phases: [
                { start: 1, end: 20, temp: 37.8, rh: 55, turn: true, notes: 'Normalna inkubacja' },
                { start: 21, end: 24, temp: 37.2, rh: 70, turn: false, notes: '🔴 LOCKDOWN – bez obrotów' }
            ],
            sensitivity: {
                critical: [1, 2, 3, 18, 19, 20, 21],
                high: [4, 5, 14, 15, 16, 17, 22, 23, 24],
                medium: [6, 7, 8, 9, 10, 11, 12, 13]
            }
        }
    };

    const Hatch = window.Hatch || (window.Hatch = {});

    Hatch.SPECIES = SPECIES;

    Hatch.getSpeciesInfo = function(speciesId) {
        return SPECIES[speciesId] || SPECIES.CHICKEN;
    };

    Hatch.getPhaseForDay = function(speciesId, day) {
        const info = Hatch.getSpeciesInfo(speciesId);
        for (const phase of info.phases) {
            if (day >= phase.start && day <= phase.end) {
                return phase;
            }
        }
        return info.phases[info.phases.length - 1] || { temp: 37.7, rh: 60, turn: true, notes: '' };
    };

    Hatch.getSensitivity = function(speciesId, day) {
        const info = Hatch.getSpeciesInfo(speciesId);
        if (!info.sensitivity) return 'MEDIUM';
        if (info.sensitivity.critical.includes(day)) return 'CRITICAL';
        if (info.sensitivity.high.includes(day)) return 'HIGH';
        return 'MEDIUM';
    };

    Hatch.getSensitivityColor = function(sensitivity) {
        if (sensitivity === 'CRITICAL') return '#ff4444';
        if (sensitivity === 'HIGH') return '#ff8844';
        return '#70d070';
    };

    Hatch.renderSchedule = function(speciesId, day) {
        const info = Hatch.getSpeciesInfo(speciesId);
        const title = document.getElementById('scheduleTitle');
        const badge = document.getElementById('scheduleBadge');
        const tbody = document.getElementById('scheduleBody');
        if (!tbody || !title || !badge) return;

        title.textContent = 'Harmonogram · ' + info.name;
        badge.textContent = 'Dzień ' + day + '/' + info.days;

        tbody.innerHTML = '';
        for (let d = 1; d <= info.days; d++) {
            let phase = null;
            for (const p of info.phases) {
                if (d >= p.start && d <= p.end) {
                    phase = p;
                    break;
                }
            }
            if (!phase) continue;

            const isActive = d === day;
            const isLockdown = d >= info.lockdown;
            const noRotate = (info.noRotate || []).includes(d);
            const sens = Hatch.getSensitivity(speciesId, d);
            const sensColor = Hatch.getSensitivityColor(sens);
            const tr = document.createElement('tr');
            if (isActive) tr.className = 'active';
            if (isLockdown) tr.className = 'lockdown';
            if (sens === 'CRITICAL') tr.style.background = 'rgba(255,68,68,0.08)';
            else if (sens === 'HIGH') tr.style.background = 'rgba(255,136,68,0.06)';

            let turnHtml = phase.turn ? '✓' : '✗';
            if (noRotate && isLockdown) {
                turnHtml = '<span class="no-rotate">✕</span>';
            }
            const sensLabel = sens === 'CRITICAL' ? '🔴' : (sens === 'HIGH' ? '🟠' : '🟢');

            tr.innerHTML =
                `<td>${isActive ? '▶ ' : ''}${d}</td><td style="color:${sensColor};font-weight:${sens === 'CRITICAL' ? 'bold' : 'normal'}">${phase.temp.toFixed(1)}</td><td>${phase.rh.toFixed(0)}</td><td>${turnHtml}</td><td>${isLockdown ? '<span class="lock">LOCK</span>' : '—'}</td><td style="font-size:9px;color:#5a6a7a;">${sensLabel} ${phase.notes || ''}</td>`;
            tbody.appendChild(tr);
        }

        const wrap = tbody.closest('.schedule-table-wrap');
        if (wrap) {
            const activeRow = tbody.querySelector('tr.active');
            if (activeRow) {
                const rowTop = activeRow.offsetTop;
                wrap.scrollTop = Math.max(0, rowTop - 60);
            }
        }

        Hatch.updatePreview(speciesId, day);
    };

    Hatch.updatePreview = function(speciesId, day) {
        const info = Hatch.getSpeciesInfo(speciesId);
        let phase = null;
        for (const p of info.phases) {
            if (day >= p.start && day <= p.end) {
                phase = p;
                break;
            }
        }
        const tbody = document.getElementById('previewBody');
        if (!tbody) return;
        if (!phase) {
            tbody.innerHTML = '<tr><td style="color:#5a6a7a;font-size:11px;">Brak danych dla tego dnia</td></tr>';
            return;
        }

        const isLockdown = day >= (info.lockdown || info.days);
        const noRotate = (info.noRotate || []).includes(day);
        const sens = Hatch.getSensitivity(speciesId, day);
        const sensColor = Hatch.getSensitivityColor(sens);

        tbody.innerHTML = `
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Gatunek</td>
                <td style="text-align:right;color:#b0c8d8;font-size:12px;">${info.name}</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Dzień</td>
                <td style="text-align:right;color:#b0c8d8;font-size:12px;">${day} / ${info.days}</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Temp. zadana</td>
                <td style="text-align:right;color:${sensColor};font-size:12px;font-weight:${sens === 'CRITICAL' ? 'bold' : 'normal'}">${phase.temp.toFixed(1)} °C</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Wilgotność</td>
                <td style="text-align:right;color:#70d070;font-size:12px;">${phase.rh.toFixed(0)} %</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Obrót</td>
                <td style="text-align:right;color:${noRotate ? '#884444' : (phase.turn ? '#70d070' : '#a070d0')};font-size:12px;">${noRotate ? '✕ BRAK' : (phase.turn ? '✓ TAK' : '✗ NIE')}</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;border-bottom:1px solid rgba(26,30,36,0.3);">Faza</td>
                <td style="text-align:right;color:${sensColor};font-size:12px;font-weight:${sens === 'CRITICAL' ? 'bold' : 'normal'}">${sens} ${isLockdown ? '🔒' : ''}</td></tr>
            <tr><td style="color:#5a6a7a;font-size:11px;padding:2px 4px;">Uwagi</td>
                <td style="text-align:right;color:#6a7a8a;font-size:11px;">${phase.notes || ''}</td></tr>
        `;
    };

    Hatch.renderScheduleTables = function() {
        const data = window.Hatch.getTrayData ? window.Hatch.getTrayData() : window.trayData;
        if (!data || !data.trays) return;
        for (let i = 0; i < 2; i++) {
            const tray = data.trays[i] || {};
            const speciesId = tray.species_id || 'CHICKEN';
            const currentDay = tray.current_day || 1;
            const info = Hatch.getSpeciesInfo(speciesId);
            const body = document.getElementById('schedBody' + i);
            if (!body) continue;
            body.innerHTML = '';
            for (let d = 1; d <= info.days; d++) {
                let phase = null;
                for (const p of info.phases) {
                    if (d >= p.start && d <= p.end) {
                        phase = p;
                        break;
                    }
                }
                if (!phase) continue;
                const isLock = d >= info.lockdown;
                const sens = Hatch.getSensitivity(speciesId, d);
                const sensColor = Hatch.getSensitivityColor(sens);
                const tr = document.createElement('tr');
                if (d === currentDay) tr.className = 'active';
                if (isLock) tr.className = 'lockdown';
                if (sens === 'CRITICAL') tr.style.background = 'rgba(255,68,68,0.06)';
                tr.innerHTML =
                    `<td style="font-weight:${sens === 'CRITICAL' ? 'bold' : 'normal'};color:${sensColor};">${d}</td><td>${phase.temp.toFixed(1)}</td><td>${phase.rh.toFixed(0)}</td><td>${phase.turn ? '✓' : '✗'}</td><td>${isLock ? '🔒' : '—'}</td>`;
                body.appendChild(tr);
            }
        }
    };

    Hatch.init = function(options) {
        this.getTrayData = (options && typeof options.getTrayData === 'function') ? options.getTrayData : function() { return window.trayData; };
        this.sendCommand = (options && typeof options.sendCommand === 'function') ? options.sendCommand : window.sendCommand;
        this.addLog = (options && typeof options.addLog === 'function') ? options.addLog : function() {};
    };

    window.SPECIES = window.SPECIES || SPECIES;
    window.getSpeciesInfo = window.getSpeciesInfo || Hatch.getSpeciesInfo;
    window.getPhaseForDay = window.getPhaseForDay || Hatch.getPhaseForDay;
    window.getSensitivity = window.getSensitivity || Hatch.getSensitivity;
    window.getSensitivityColor = window.getSensitivityColor || Hatch.getSensitivityColor;
    window.renderSchedule = window.renderSchedule || function(speciesId, day) { return Hatch.renderSchedule(speciesId, day); };
    window.updatePreview = window.updatePreview || function(speciesId, day) { return Hatch.updatePreview(speciesId, day); };
    window.renderScheduleTables = window.renderScheduleTables || function() { return Hatch.renderScheduleTables(); };
})();