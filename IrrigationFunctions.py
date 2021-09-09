# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a file with all functions to operate the irrigation and air system
"""

import struct
import time



class SettingsNotAcceptedError(Exception):
    pass



def add_crc(data):
    crc_sum = 0xFFFF
    
    for current_byte in data:
        
        crc_sum = ( crc_sum ^ current_byte << 8 )
        # print(crc_sum)
        for _ in range(8):
            if( crc_sum & 0x8000):
                
                # crc_sum <<= 1
                crc_sum = (crc_sum << 1) ^ 0x1021
            else:
                crc_sum <<= 1
    low=crc_sum & 0xff
    high = crc_sum  >> 8 & 0xff # this 0xff was my own addition to the code to make it work. Why? Otherwise numbers were too long, so I took only the first byte

    lowb = (struct.pack('B',low))
    highb = (struct.pack('B',high))
    message=data + highb + lowb
    return message


def read_setpoint(ser):
    variable = 37
    message=b'\xfa\x02\x02' + struct.pack('B',variable)
    message=add_crc(message)

    ser.write(message)
    response=ser.readline()
    # print(response)
    # value in counts
    valuec= struct.unpack('>H',response[2:4])[0]
    # value in %
    valuep =  (valuec - 400) * 100 / 3,300
    
    return valuep

def set_setpoint(ser, valuep):
    variable = 37
    # setpoint value is in %, needs to be converted to counts
    value =  int(( valuep * 3300 /100) + 400)
   
    message = b'\xfa\x04\x01' + struct.pack('B',variable) + struct.pack('>H',value)
    message=add_crc(message)
   
    ser.write(message)
    response=ser.readline()
    # print(response)
    
    if response == b'\x00\x01\x00\xff\xad':
        print('Setpoint value was accepted')
    else:
        raise SettingsNotAcceptedError
        

