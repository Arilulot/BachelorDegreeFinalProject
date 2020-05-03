import sys
import json
import datetime

time = datetime.datetime.now() #Gets the current time
in_json = sys.argv[1] #Lecture of the data sent as an input in the JSON Node-Red flow
data = json.loads(in_json) #Parse data into JSON
voltage = float(data[1]) #Read of voltage value from input data
current = float(data[0]) #Read of current value from input data
temp = float(data[2]) #Read of temperature value from input data
ident = "001" #PV ID
config = "2s1p" # Electrical Configuration
power = voltage*current #Power calculation
power_round = round(power,2) #Round value of power to 2 decimals

data_dict = {'Voltage': voltage, #Output JSON creation
             'Current': current,
             'Power': power_round,
             'Temperature': temp,
             'Time': time.strftime("%d/%m/%Y %H:%M:%S") ,
             'Configuration': config,
			 'Identificador': ident,
             }
data_json = json.dumps(data_dict) #Parse
print(data_json) #Return JSON to Node Red