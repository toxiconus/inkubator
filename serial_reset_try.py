import serial, time, sys
port = 'COM9'
try:
    ser = serial.Serial(port, 115200, timeout=1)
    print('OPENED', port)
    print('STATE', 'DTR', ser.dtr, 'RTS', ser.rts)
    print('TRY normal boot sequence 1')
    ser.setRTS(True)
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(0.5)
    start = time.time()
    while time.time() - start < 5:
        line = ser.readline()
        if line:
            sys.stdout.write(line.decode('utf-8', 'replace'))
    print('\nTRY normal boot sequence 2')
    ser.setDTR(True)
    ser.setRTS(False)
    time.sleep(0.1)
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(0.5)
    start = time.time()
    while time.time() - start < 5:
        line = ser.readline()
        if line:
            sys.stdout.write(line.decode('utf-8', 'replace'))
    ser.close()
except Exception as e:
    print('ERROR', e)
