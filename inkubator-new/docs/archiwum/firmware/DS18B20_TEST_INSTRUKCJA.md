## 🚀 Mini Program DS18B20 - Instrukcja Wgrania

Plik `main.cpp` został zastąpiony mini programem do testowania czujnika DS18B20.

### ✅ Co program robi:
- Inicjuje OneWire dla GPIO6 i GPIO7
- Czyta temperaturę co 2 sekundy
- Wyświetla wynik: **czujnik OK** (temperatura) lub **DISCONNECTED** (-99)

### 📥 Jak wgrać:

**Opcja 1: VSCode + PlatformIO Extension**
1. W VSCode naciśnij `Ctrl+Shift+P`
2. Wpisz: `PlatformIO: Build`
3. Po kompilacji: `PlatformIO: Upload`
4. System wgra kod na ESP32-S3

**Opcja 2: Ręcznie przez Terminal**
```bash
cd j:\inkubator
pio run -e esp32s3 -t upload --upload-port COM5
```

### 📊 Oczekiwane wyniki:

**Jeśli czujnik jest PODŁĄCZONY i rezystor OK:**
```
[  2000] S0: 24.56C | S1: DISC
[  4000] S0: 24.56C | S1: DISC
[  6000] S0: 24.57C | S1: DISC
```

**Jeśli czujnik NIE jest podłączony (brak rezystora lub brak połączenia):**
```
[  2000] S0: DISC | S1: DISC
[  4000] S0: DISC | S1: DISC
[  6000] S0: DISC | S1: DISC
```

### 🔍 Debug Info:
Czytaj logi przez serial monitor (COM5 @ 115200 baud).

### ⏮️ Przywrócenie v2:
Aby przywrócić pełny Inkubator:
```bash
Copy-Item j:\inkubator\firmware\src\main.cpp.v2.backup j:\inkubator\firmware\src\main.cpp -Force
```

---

**Backup pełnej wersji:** `main.cpp.v2.backup` (już zapisany)
