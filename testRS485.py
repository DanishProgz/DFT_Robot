import serial, time

ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=5)
time.sleep(0.3)

# Move forward 200 steps
ser.write(b"@01:FWD:80000000\n")
resp = ser.readline().decode().strip()
print("Response:", resp)
