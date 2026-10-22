import serial
import time
import serial.tools.list_ports as lp

print('ports:', [p.device + ' ' + p.description for p in lp.comports()])

try:
    with serial.Serial('COM9', 115200, timeout=1) as ser:
        print('opened COM9')
        start = time.time()
        while time.time() - start < 12:
            line = ser.readline()
            if line:
                print(repr(line))
                print(line.decode('utf-8', errors='replace').rstrip('\r\n'))
            else:
                time.sleep(0.1)
except Exception as e:
    print('ERROR:', e)
