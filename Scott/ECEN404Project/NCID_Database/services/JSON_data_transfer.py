import json
import serial

connection = serial.Serial(port="COM3", baudrate=115200)
connection.reset_input_buffer()

while True:
    data = connection.readline()
    print(data)

    #try:
    #    dict_json = json.loads(data)
    #    print(dict_json)
    #except json.JSONDecodeError as e:
    #    print("JSON:", e)