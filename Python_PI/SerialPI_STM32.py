import serial

ser = serial.Serial()

def serialInit(baudrate , port):
    ser.baudrate = baudrate
    ser.port = port
    ser.open()
    if ser.is_open == True:
        print("Serial init OK")
    else:
        print("Serial n'est pas bien initialiser")

def serialWrite(message):
    ser.write(message.encode())
    ser.close()

def serialRead(ser,size):
    data = ser.read(size)
    return data


serialInit(115200,'COM4')

