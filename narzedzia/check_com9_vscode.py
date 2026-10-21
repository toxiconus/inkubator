import serial
import time
import serial.tools.list_ports as lp

print('ports:')
for p in lp.comports():
    print(f'{p.device} - {p.description} - {p.hwid}')

try:
    with serial.Serial('COM9', 115200, timeout=3) as ser:
        print('opened COM9')
        time.sleep(1)
        ser.reset_input_buffer()
        ser.write(b'\n')
        start = time.time()
        while time.time() - start < 10:
            line = ser.readline()
            if line:
                print(repr(line))
                print(line.decode('utf-8', errors='replace').rstrip('\r\n'))
        print('done')
except Exception as e:
    print('ERR', type(e).__name__, e)
