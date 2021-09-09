# -*- coding: utf-8 -*-
"""
Created on Thu Jul 29 16:49:17 2021

@author: Sandora
"""

import pyvisa
import numpy as np
import time
import pandas as pd
from datetime import datetime
import serial
import struct
from IrrigationFunctions import *
from serial.tools.list_ports import comports


# USER CONSTANTS

water_jet_setpoint_high = 30 # the value is in %
water_jet_setpoint_low = 0 # the value is in %
time_on = 2.5
time_off = 2.5


try:
    available_com_ports = comports(include_links=True)
    for item in available_com_ports:        
        vendor_id = item.vid
        if vendor_id == 10221:
            com_port = comports(include_links=True)[0].device
            
    ser = serial.Serial(port = com_port, baudrate=9600, rtscts=1, bytesize=8, parity='N', stopbits=2, timeout=1) 
    
    # set_setpoint(ser, water_jet_setpoint_high)
    # time.sleep(2.5)
    # set_setpoint(ser, water_jet_setpoint_low)
    # time.sleep(2.5)
    
    
    for _ in range(1):
        set_setpoint(ser, water_jet_setpoint_high) 
        time.sleep(time_on)
        # for i in range(time_on):
        #     time.sleep(1)
            
        set_setpoint(ser, water_jet_setpoint_low)
        time.sleep(time_off)
        # for i in range(time_off):
        #     time.sleep(1)
    
    # valuep = read_setpoint(ser)
    # # turn off the jet
    # set_setpoint(ser, 0) # the value is in %
 
    
    ser.close()
except SettingsNotAcceptedError:
    set_setpoint(ser, 0)
    print('Settings were not accepted')
    ser.close()
except:
    set_setpoint(ser, 0)
    print('Something went wrong')
    ser.close()
    
    