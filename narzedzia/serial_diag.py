import serial, time, sys
port = 'COM9'
try:
    ser = serial.Serial(port, 115200, timeout=1)
    print('OPENED', port)
    ser.setRTS(True)
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(0.5)
    print('reset done')
    start = time.time()
    while time.time() - start < 5:
        line = ser.readline()
        if line:
            sys.stdout.write(line.decode('utf-8', 'replace'))
    ser.close()
except Exception as e:
    print('ERROR', e)
