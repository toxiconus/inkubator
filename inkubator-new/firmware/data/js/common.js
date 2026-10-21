/**
 * common.js - modular shared runtime for Inkubator HMI
 */
(function() {
    'use strict';

    const Inkubator = {
        Config: {
            wsPort: 81,
            wsReconnect: 3000,
            maxRetries: 10,
            debug: false,
            apiTimeoutMs: 5000,
            defaultHost: '192.168.5.1'
        },

        State: {
            temperature: null,
            humidity: null,
            heater: { ch0: 0, ch1: 0 },
            fan: 0,
            alarm: { active: false, messages: [] },
            water: { level: 100, status: 'ok' },
            motor: { running: false, turns: 0 },
            rtc: { ok: true, time: null },
            wifi: { connected: false, rssi: 0 },
            sensors: { ds1: null, ds2: null, dht: null },
            esp: { temp: null, free_ram: null, cpu: null },
            lastUpdateTs: null,
            offline: false
        },

        Events: {
            _listeners: {},
            emit: function(event, data) {
                const listeners = this._listeners[event];
                if (!listeners || !listeners.length) return;
                listeners.forEach(function(cb) {
                    try {
                        cb(data);
                    } catch (err) {
                        if (Inkubator.Config.debug) {
                            console.warn('Event listener failed for', event, err);
                        }
                    }
                });
            },
            subscribe: function(event, callback) {
                if (!this._listeners[event]) this._listeners[event] = [];
                this._listeners[event].push(callback);
                return () => this.unsubscribe(event, callback);
            },
            unsubscribe: function(event, callback) {
                if (!this._listeners[event]) return;
                this._listeners[event] = this._listeners[event].filter(function(cb) {
                    return cb !== callback;
                });
            }
        },

        Parser: {
            decode: function(message) {
                if (typeof message !== 'string') {
                    return this.normalize(message || {});
                }
                try {
                    return this.normalize(JSON.parse(message));
                } catch (err) {
                    return this.parseLegacy(message);
                }
            },

            normalize: function(data) {
                const normalized = {
                    type: data.type || 'update',
                    payload: {
                        temperature: data.temp_chamber ?? data.temp ?? null,
                        humidity: data.humidity ?? null,
                        heater: {
                            ch0: data.heater_pwm_ch0 ?? 0,
                            ch1: data.heater_pwm_ch1 ?? 0
                        },
                        fan: data.fan_speed ?? 0,
                        motor: {
                            running: data.motor_running ?? false,
                            turns: data.motor_turns ?? 0
                        },
                        water: {
                            level: data.water_level ?? 100,
                            status: (data.water_level ?? 100) < 20 ? 'low' : 'ok'
                        },
                        wifi: {
                            connected: data.wifi_connected ?? true,
                            rssi: data.wifi_rssi ?? -50
                        },
                        rtc: {
                            ok: data.rtc_ok ?? true,
                            time: data.rtc_time ?? null
                        },
                        sensors: {
                            ds1: data.temp_chamber ?? null,
                            ds2: data.temp_aquarium ?? null,
                            dht: data.humidity ?? null
                        },
                        esp: {
                            temp: data.esp_internal_temp ?? null,
                            free_ram: data.free_ram ?? null,
                            cpu: data.cpu_load ?? null
                        },
                        raw: data
                    },
                    timestamp: data.timestamp || Date.now()
                };
                return normalized;
            },

            parseLegacy: function(message) {
                const parsed = {};
                message.split('\n').forEach(function(line) {
                    const pair = line.split('=');
                    if (pair.length < 2) return;
                    const key = pair[0].trim();
                    const raw = pair.slice(1).join('=').trim();
                    const num = Number(raw);
                    parsed[key] = Number.isFinite(num) ? num : raw;
                });
                return this.normalize(parsed);
            }
        },

        WS: window.Inkubator && window.Inkubator.WS ? window.Inkubator.WS : {},

        API: {
            fetchJson: function(url, options) {
                const cfg = options || {};
                const controller = new AbortController();
                const timer = setTimeout(function() {
                    controller.abort();
                }, Inkubator.Config.apiTimeoutMs);

                const query = new URLSearchParams(window.location.search);
                const queryHost = query.get('espHost');
                const baseHost = queryHost || Inkubator.Storage.load('inkubator.wsHost', null) || (window.location.hostname || window.location.host);
                const baseUrl = baseHost ? (window.location.protocol + '//' + baseHost) : null;
                const targetUrl = (baseUrl && url.startsWith('/')) ? (baseUrl + url) : url;

                return fetch(targetUrl, { ...cfg, signal: controller.signal })
                    .then(function(resp) {
                        clearTimeout(timer);
                        if (!resp.ok) {
                            throw new Error('HTTP ' + resp.status + ' for ' + targetUrl);
                        }
                        return resp.json();
                    })
                    .catch(function(err) {
                        clearTimeout(timer);
                        throw err;
                    });
            },

            fetchStatus: function() {
                return this.fetchJson('/api/status');
            },

            fetchTrays: function() {
                return this.fetchJson('/api/trays');
            },

            sendCommand: function(command, data) {
                return this.fetchJson('/api/command', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ command: command, data: data || {} })
                });
            }
        },

        UI: {
            updateMetric: function(id, value, unit) {
                const el = document.getElementById(id);
                if (!el) return;
                const text = unit ? String(value) + ' ' + unit : String(value);
                if (el.dataset.value !== text) {
                    el.textContent = text;
                    el.dataset.value = text;
                }
            },

            updateBadge: function(id, status) {
                const el = document.getElementById(id);
                if (!el) return;
                const cls = status === 'ok' ? 'ok' : status === 'warn' ? 'warn' : status === 'err' ? 'err' : '';
                el.classList.remove('ok', 'warn', 'err');
                if (cls) el.classList.add(cls);
            },

            updateHealth: function(id, ok) {
                const el = document.getElementById(id);
                if (!el) return;
                el.classList.remove('ok', 'err');
                el.classList.add(ok ? 'ok' : 'err');
            },

            showToast: function(message, type) {
                const toast = document.createElement('div');
                toast.className = 'hmi-toast ' + (type || 'info');
                toast.textContent = message;
                document.body.appendChild(toast);
                setTimeout(function() {
                    toast.classList.add('show');
                }, 10);
                setTimeout(function() {
                    toast.classList.remove('show');
                    setTimeout(function() {
                        toast.remove();
                    }, 200);
                }, 2200);
            },

            setStatus: function(text, type) {
                const statusMsg = document.getElementById('statusMsg');
                if (!statusMsg) return;
                statusMsg.textContent = text;
                statusMsg.className = 'msg' + (type ? ' ' + type : '');
            },

            setConnectionState: function(state) {
                const wsDot = document.getElementById('wsDot');
                const wsStatus = document.getElementById('wsStatus');
                const wsDetail = document.getElementById('wsDetail');

                if (wsDot) {
                    wsDot.classList.remove('online', 'offline', 'warn');
                    if (state === 'connected') wsDot.classList.add('online');
                    if (state === 'disconnected') wsDot.classList.add('offline');
                    if (state === 'connecting') wsDot.classList.add('warn');
                }

                if (wsStatus) {
                    wsStatus.textContent = state === 'connected' ? 'Polaczono' : state === 'connecting' ? 'Laczenie' : 'Rozlaczono';
                }

                if (wsDetail) {
                    wsDetail.textContent = state === 'connected' ? 'WebSocket OK' : 'WebSocket ' + state;
                }
            }
        },

        Storage: {
            save: function(key, data) {
                try {
                    localStorage.setItem(key, JSON.stringify(data));
                    return true;
                } catch (err) {
                    return false;
                }
            },
            load: function(key, fallback) {
                try {
                    const raw = localStorage.getItem(key);
                    return raw ? JSON.parse(raw) : fallback;
                } catch (err) {
                    return fallback;
                }
            },
            clear: function() {
                try {
                    localStorage.clear();
                    return true;
                } catch (err) {
                    return false;
                }
            }
        },

        Utils: {
            formatTime: function(ms) {
                const totalSec = Math.max(0, Math.round(ms / 1000));
                const h = Math.floor(totalSec / 3600);
                const m = Math.floor((totalSec % 3600) / 60);
                const s = totalSec % 60;
                if (h > 0) return String(h).padStart(2, '0') + ':' + String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0');
                return String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0');
            },
            clamp: function(value, min, max) {
                return Math.max(min, Math.min(max, value));
            },
            round: function(value, decimals) {
                const p = Math.pow(10, decimals || 0);
                return Math.round(value * p) / p;
            },
            parseFloatSafe: function(value, fallback) {
                const parsed = parseFloat(value);
                return Number.isFinite(parsed) ? parsed : fallback;
            },
            parseIntSafe: function(value, fallback) {
                const parsed = parseInt(value, 10);
                return Number.isFinite(parsed) ? parsed : fallback;
            }
        },

        Offline: {
            _timer: null,
            _lastUpdate: null,
            show: function() {
                const instrument = document.querySelector('.instrument');
                const banner = document.getElementById('offlineBanner');
                if (instrument) instrument.classList.add('offline');
                if (banner) {
                    banner.classList.add('visible');
                    const stamp = this._lastUpdate ? this._lastUpdate.toLocaleTimeString() : 'never';
                    banner.textContent = 'OFFLINE MODE - last update: ' + stamp;
                }
                this._startCountdown();
            },
            hide: function() {
                const instrument = document.querySelector('.instrument');
                const banner = document.getElementById('offlineBanner');
                if (instrument) instrument.classList.remove('offline');
                if (banner) banner.classList.remove('visible');
                clearInterval(this._timer);
                this._timer = null;
            },
            markUpdate: function(timestamp) {
                this._lastUpdate = timestamp ? new Date(timestamp) : new Date();
            },
            _startCountdown: function() {
                clearInterval(this._timer);
                let seconds = 0;
                this._timer = setInterval(function() {
                    seconds += 1;
                    const banner = document.getElementById('offlineBanner');
                    if (banner) {
                        banner.textContent = 'OFFLINE MODE - ' + seconds + 's since last update';
                    }
                }, 1000);
            }
        },

        setStateFromPayload: function(payload, timestamp) {
            if (!payload) return;
            this.State.temperature = payload.temperature;
            this.State.humidity = payload.humidity;
            this.State.heater = payload.heater || this.State.heater;
            this.State.fan = payload.fan ?? this.State.fan;
            this.State.motor = payload.motor || this.State.motor;
            this.State.water = payload.water || this.State.water;
            this.State.wifi = payload.wifi || this.State.wifi;
            this.State.rtc = payload.rtc || this.State.rtc;
            this.State.sensors = payload.sensors || this.State.sensors;
            this.State.esp = payload.esp || this.State.esp;
            this.State.lastUpdateTs = timestamp || Date.now();
            this.State.offline = false;
            this.Offline.markUpdate(this.State.lastUpdateTs);
            this.Offline.hide();
        }
    };

    Inkubator.wsSend = function(command, data) { return Inkubator.WS.send(command, data); };
    Inkubator.wsConnect = function(path) { return Inkubator.WS.connect(path); };
    Inkubator.wsConnected = function() {
        return Inkubator.WS._ws && Inkubator.WS._ws.readyState === WebSocket.OPEN;
    };
    Inkubator.setStatus = Inkubator.UI.setStatus;
    Inkubator.setWsStatus = Inkubator.UI.setConnectionState;
    Inkubator.setWsHost = function(host) {
        const value = (host || '').trim();
        if (!value) return false;
        return Inkubator.Storage.save('inkubator.wsHost', value);
    };
    Inkubator.getWsHost = function() {
        return Inkubator.Storage.load('inkubator.wsHost', null) || Inkubator.WS.resolveHost();
    };
    Inkubator.reconnectManually = function(path) {
        if (Inkubator.WS._ws) {
            try { Inkubator.WS._ws.close(); } catch (err) {}
        }
        Inkubator.WS._retryCount = 0;
        Inkubator.WS.connect(path || '/');
    };
    Inkubator.onWsEvent = function(callback) {
        if (typeof callback !== 'function') return function() {};

        const unsubOpen = Inkubator.Events.subscribe('ws.open', function(event) {
            callback('open', event.ws);
        });
        const unsubMsg = Inkubator.Events.subscribe('ws.message', function(event) {
            callback('message', Inkubator.WS._ws, event.raw);
        });
        const unsubClose = Inkubator.Events.subscribe('ws.close', function() {
            callback('close', Inkubator.WS._ws);
        });

        return function() {
            unsubOpen();
            unsubMsg();
            unsubClose();
        };
    };
    Inkubator.parseFloatSafe = Inkubator.Utils.parseFloatSafe;
    Inkubator.parseIntSafe = Inkubator.Utils.parseIntSafe;
    Inkubator.clamp = Inkubator.Utils.clamp;
    Inkubator.getWsUrl = Inkubator.WS.getUrl.bind(Inkubator.WS);

    document.addEventListener('DOMContentLoaded', function() {
        const body = document.body;
        const query = new URLSearchParams(window.location.search);
        const forceOfflineMode = query.get('offline') === '1';
        const isFileProtocol = window.location.protocol === 'file:';
        const disableAutoWs = forceOfflineMode || isFileProtocol || (body && body.dataset && body.dataset.wsAutoconnect === 'false');

        const wsStatus = document.getElementById('wsStatus');
        const wsDot = document.getElementById('wsDot');

        if (wsStatus) wsStatus.addEventListener('click', function() { Inkubator.reconnectManually('/'); });
        if (wsDot) wsDot.addEventListener('click', function() { Inkubator.reconnectManually('/'); });

        if (disableAutoWs) {
            Inkubator.UI.setConnectionState('disconnected');
            if (isFileProtocol && !forceOfflineMode) {
                Inkubator.UI.setStatus('Tryb plikowy - dodaj ?offline=0&espHost=IP_ESP aby laczyc online', 'info');
            } else {
                Inkubator.UI.setStatus('Tryb offline - bez ESP', 'info');
            }
            Inkubator.Offline.show();
            return;
        }

        Inkubator.WS.connect('/');

        setInterval(function() {
            if (!Inkubator.wsConnected()) {
                Inkubator.reconnectManually('/');
            }
        }, 30000);
    });

    window.addEventListener('beforeunload', function() {
        if (Inkubator.WS && typeof Inkubator.WS.close === 'function') {
            try { Inkubator.WS.close(); } catch (err) {}
        }
        if (Inkubator.WS && Inkubator.WS._reconnectTimer) {
            clearTimeout(Inkubator.WS._reconnectTimer);
        }
    });

    if (Inkubator.WS && typeof Inkubator.WS.subscribe === 'function') {
        Inkubator.WS.subscribe('status.update', function(payload) {
            Inkubator.Events.emit('status.update', payload);
        });
        Inkubator.WS.subscribe('trays.update', function(payload) {
            Inkubator.Events.emit('trays.update', payload);
        });
        Inkubator.WS.subscribe('ws.open', function(data) {
            Inkubator.Events.emit('ws.open', data);
        });
        Inkubator.WS.subscribe('ws.close', function(data) {
            Inkubator.Events.emit('ws.close', data);
        });
        Inkubator.WS.subscribe('ws.error', function(data) {
            Inkubator.Events.emit('ws.error', data);
        });
    }

    window.Inkubator = Inkubator;
})();

(function() {
    'use strict';

    const Inkubator = window.Inkubator || {};

    function extend(target, source) {
        if (!target || !source) return;
        Object.keys(source).forEach(function(key) {
            if (target[key] === undefined) {
                target[key] = source[key];
            }
        });
    }

    Inkubator.Events = Inkubator.Events || {
        _listeners: {},
        emit: function(event, data) {
            const list = this._listeners[event];
            if (!list || !list.length) return;
            list.forEach(function(cb) {
                try { cb(data); } catch (err) {}
            });
        },
        subscribe: function(event, callback) {
            if (!this._listeners[event]) this._listeners[event] = [];
            this._listeners[event].push(callback);
            return function() { Inkubator.Events.unsubscribe(event, callback); };
        },
        unsubscribe: function(event, callback) {
            if (!this._listeners[event]) return;
            this._listeners[event] = this._listeners[event].filter(function(cb) {
                return cb !== callback;
            });
        },
        once: function(event, callback) {
            const wrapper = function(data) {
                callback(data);
                Inkubator.Events.unsubscribe(event, wrapper);
            };
            this.subscribe(event, wrapper);
        }
    };

    Inkubator.State = Inkubator.State || {};
    extend(Inkubator.State, {
        get: function(key) {
            return key ? this[key] : this;
        },
        set: function(key, value) {
            if (typeof key === 'object' && key !== null) {
                Object.assign(this, key);
            } else {
                this[key] = value;
            }
            this.lastUpdateTs = Date.now();
            Inkubator.Events.emit('state.changed', this);
            if (typeof Inkubator.Events.emit === 'function') {
                Inkubator.Events.emit('state.' + key, value);
            }
        },
        update: function(data) {
            if (data && typeof data === 'object') {
                Object.assign(this, data);
            }
            this.lastUpdateTs = Date.now();
            this.offline = false;
            Inkubator.Events.emit('state.changed', this);
            if (Inkubator.Offline && typeof Inkubator.Offline.markUpdate === 'function') {
                Inkubator.Offline.markUpdate(this.lastUpdateTs);
            }
            if (Inkubator.Offline && typeof Inkubator.Offline.hide === 'function') {
                Inkubator.Offline.hide();
            }
        }
    });

    Inkubator.UI = Inkubator.UI || {};
    extend(Inkubator.UI, {
        setText: function(id, value) {
            const el = document.getElementById(id);
            if (el) el.textContent = value;
        },
        setValue: function(id, value, unit) {
            const el = document.getElementById(id);
            if (!el) return;
            el.textContent = value + (unit ? ' ' + unit : '');
        },
        setHealth: function(id, ok, label) {
            const el = document.getElementById(id);
            if (!el) return;
            el.className = 'health-item ' + (ok ? 'ok' : 'err');
            if (label) el.textContent = label;
        },
        setDot: function(id, state) {
            const el = document.getElementById(id);
            if (!el) return;
            el.className = 'dot';
            if (state === 'ok') el.classList.add('ok');
            else if (state === 'warn') el.classList.add('warn');
            else if (state === 'err') el.classList.add('err');
            else if (state === 'loading') el.classList.add('loading');
            else el.classList.add('offline');
        },
        show: function(id) {
            const el = document.getElementById(id);
            if (el) el.style.display = '';
        },
        hide: function(id) {
            const el = document.getElementById(id);
            if (el) el.style.display = 'none';
        },
        toggle: function(id) {
            const el = document.getElementById(id);
            if (!el) return;
            el.style.display = el.style.display === 'none' ? '' : 'none';
        }
    });

    Inkubator.Storage = Inkubator.Storage || {};
    extend(Inkubator.Storage, {
        remove: function(key) {
            try {
                localStorage.removeItem(key);
                return true;
            } catch (e) {
                return false;
            }
        }
    });

    Inkubator.Utils = Inkubator.Utils || {};
    extend(Inkubator.Utils, {
        clamp: function(v, min, max) {
            return Math.max(min, Math.min(max, v));
        },
        round: function(v, d) {
            const p = Math.pow(10, d || 0);
            return Math.round(v * p) / p;
        },
        formatTime: function(ms) {
            const s = Math.floor(ms / 1000);
            const m = Math.floor(s / 60);
            const h = Math.floor(m / 60);
            return String(h).padStart(2, '0') + ':' + String(m % 60).padStart(2, '0') + ':' + String(s % 60).padStart(2, '0');
        },
        formatDate: function(ts) {
            const d = new Date(ts);
            return d.toLocaleTimeString('pl-PL', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
        },
        parseFloat: function(v, fallback) {
            const n = parseFloat(v);
            return isNaN(n) ? fallback : n;
        },
        parseInt: function(v, fallback) {
            const n = parseInt(v, 10);
            return isNaN(n) ? fallback : n;
        }
    });

    Inkubator.Config = Inkubator.Config || {};
    extend(Inkubator.Config, {
        load: function() {
            try {
                const saved = Inkubator.Storage.load('inkubator.config', {});
                Object.assign(this, saved);
            } catch (e) {}
            return this;
        },
        save: function() {
            Inkubator.Storage.save('inkubator.config', {
                wsPort: this.wsPort,
                wsReconnect: this.wsReconnect,
                apiTimeoutMs: this.apiTimeoutMs,
                refreshInterval: this.refreshInterval,
                maxRetries: this.maxRetries,
                debug: this.debug
            });
            return this;
        }
    });

    if (typeof Inkubator.Config.load === 'function') {
        Inkubator.Config.load();
    }

    if (typeof window.setText !== 'function') {
        window.setText = function(id, value) {
            const el = document.getElementById(id);
            if (el) el.textContent = value;
        };
    }

    if (typeof window.setHtml !== 'function') {
        window.setHtml = function(id, html) {
            const el = document.getElementById(id);
            if (el) el.innerHTML = html;
        };
    }

    if (typeof window.fetchJson !== 'function') {
        window.fetchJson = function(url, options = {}, timeoutMs = 5000) {
            if (window.Inkubator && Inkubator.API && typeof Inkubator.API.fetchJson === 'function') {
                return Inkubator.API.fetchJson(url, options);
            }
            const controller = new AbortController();
            const timer = setTimeout(function() {
                controller.abort();
            }, timeoutMs);
            return fetch(url, { ...options, signal: controller.signal })
                .then(function(response) {
                    clearTimeout(timer);
                    if (!response.ok) {
                        throw new Error('HTTP ' + response.status + ' for ' + url);
                    }
                    return response.json();
                })
                .catch(function(err) {
                    clearTimeout(timer);
                    throw err;
                });
        };
    }

    if (typeof window.addLog !== 'function') {
        window.addLog = function(type, msg) {
            if (typeof console !== 'undefined' && typeof console.log === 'function') {
                console.log('[' + type + ']', msg);
            }
        };
    }

    window.Inkubator = Inkubator;
})();
