// Użycie: node gen_nonbird_profiles.js
// Odtwarza w incubation_profiles.json programy nasion i fermentacji z danych zapisanych w panelu (index.html).
// Generuje profile programów nie-ptasich (nasiona, fermentacja) w formacie incubation_profiles.json z danych panelu.
const fs = require('fs'), vm = require('vm');
const path = require('path');
const HTML = path.join(__dirname, '..', 'firmware', 'data', 'index.html');
const F = path.join(__dirname, '..', 'firmware', 'data', 'incubation_profiles.json');
const html = fs.readFileSync(HTML, 'utf8').replace(/\r\n/g, '\n');
const a = html.indexOf('            const SPECIES = {};');
const b = html.indexOf('            const CATEGORY_LABELS = {');
if (a < 0 || b < 0) throw new Error('markers');
const code = html.slice(a, b) + '\n;SPECIES;';
const SPECIES = vm.runInNewContext(code, {});

const KEYS = { LETTUCE: 'lettuce', TOMATO: 'tomato', PEPPER: 'pepper', CUCUMBER: 'cucumber', BASIL: 'basil', MICROGREENS: 'microgreens', SPROUTS: 'sprouts', YOGURT: 'yogurt', TEMPEH: 'tempeh', DOUGH: 'dough', SOURDOUGH: 'sourdough' };
const IDS = { lettuce: 'lactuca_sativa', tomato: 'solanum_lycopersicum', pepper: 'capsicum_annuum', cucumber: 'cucumis_sativus', basil: 'ocimum_basilicum', microgreens: 'raphanus_sativus', sprouts: 'medicago_sativa', yogurt: 'yogurt', tempeh: 'tempeh', dough: 'dough', sourdough: 'sourdough' };

let txt = fs.readFileSync(F, 'utf8');
const crlf = txt.includes('\r\n');
txt = txt.replace(/\r\n/g, '\n');
const doc = JSON.parse(txt);
Object.keys(doc.species).forEach(k => { if (doc.species[k].non_bird) delete doc.species[k]; });

function profile(key, sp) {
  const days = [];
  for (let d = 1; d <= sp.days; d++) {
    const ph = sp.phases.find(p => d >= p.start && d <= p.end) || sp.phases[sp.phases.length - 1];
    const sensitivity = sp.sensitivity.critical.includes(d) ? 'CRITICAL' : sp.sensitivity.high.includes(d) ? 'HIGH' : 'MEDIUM';
    days.push({
      day: d, shell_temp: ph.temp, air_temp_target: ph.temp, humidity_set: ph.rh == null ? 0 : ph.rh,
      humidity_range: ph.rh == null ? [0, 0] : [ph.rhmin, ph.rhmax], turning: false, misting: false, sensitivity,
      status_color: sensitivity === 'CRITICAL' ? '#ff4444' : sensitivity === 'HIGH' ? '#ff8844' : '#70d070', notes: ''
    });
  }
  return {
    id: IDS[key], common_name: sp.name, incubation_days: sp.days, non_bird: true,
    description: sp.shortName + ': program ' + (sp.category === 'nasiona' ? 'kiełkowania' : 'fermentacji') + ' (' + (sp.duration || sp.days + ' dni') + ')',
    reference: sp.source || '', turning_frequency_per_day: 0, day_by_day: days
  };
}

Object.keys(KEYS).forEach(k => {
  const sp = SPECIES[k];
  if (!sp) throw new Error('brak ' + k);
  doc.species[KEYS[k]] = profile(KEYS[k], sp);
});

// zapis: jeden dzień w jednej linii, jak w pozostałych profilach
function fmt(o) {
  const lines = ['{'];
  lines.push('  "version": ' + JSON.stringify(doc.version) + ',');
  lines.push('  "sensitivity_colors": ' + JSON.stringify(doc.sensitivity_colors) + ',');
  lines.push('  "species": {');
  const keys = Object.keys(doc.species);
  keys.forEach((k, i) => {
    const sp = doc.species[k];
    const head = Object.assign({}, sp); delete head.day_by_day; delete head.critical_phases;
    const rest = Object.keys(head).map(h => '      ' + JSON.stringify(h) + ': ' + JSON.stringify(head[h]));
    let body = '    ' + JSON.stringify(k) + ': {\n' + rest.join(',\n');
    if (sp.critical_phases) body += ',\n      "critical_phases": ' + JSON.stringify(sp.critical_phases);
    body += ',\n      "day_by_day": [\n' + sp.day_by_day.map(d => '        ' + JSON.stringify(d)).join(',\n') + '\n      ]\n    }';
    lines.push(body + (i < keys.length - 1 ? ',' : ''));
  });
  lines.push('  },');
  lines.push('  "alarm_system": ' + JSON.stringify(doc.alarm_system, null, 2).replace(/\n/g, '\n  '));
  lines.push('}');
  return lines.join('\n') + '\n';
}
const out = fmt(doc);
JSON.parse(out);
fs.writeFileSync(F, crlf ? out.replace(/\n/g, '\r\n') : out, 'utf8');
console.log('ok', Object.keys(doc.species).length, 'programów,', out.length, 'B');
