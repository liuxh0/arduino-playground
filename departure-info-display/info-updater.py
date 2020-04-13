import datetime
import dateutil.parser
import http.client
import json
import serial
import time
import urllib.parse

# Connect arduino via USB port
arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=.1)
time.sleep(1)

while True:     # Send current information to arduino repeatedly
    now = datetime.datetime.now()
    nowdate = now.strftime('%Y%m%d')
    nowtime = now.strftime('%H%M')
    print(now)

    station_code = 'de:08111:6112'

    request_url = '/mngvvs/XML_DM_REQUEST' + \
        '?SpEncId=0&coordOutputFormat=EPSG:4326&deleteAssignedStops=1&itdDate={date}&itdTime={time}' + \
        '&limit=2&macroWebDep=true&mode=direct&name_dm={station}&outputFormat=rapidJSON&serverInfo=1&type_dm=any' + \
        '&useRealtime=1&version=10.2.10.139'
    request_url = request_url.format(date = nowdate, time = nowtime, station = station_code)

    connection = http.client.HTTPSConnection('www3.vvs.de')
    connection.request('GET', request_url)
    response = connection.getresponse()
    response_data = json.loads(response.read())

    arduino_payload = ''
    for entry in response_data['stopEvents']:
        departureTime = entry['departureTimePlanned']
        if 'departureTimeEstimated' in entry:
            departureTime = entry['departureTimeEstimated']
        trainNumber = entry['transportation']['number']
        destination = entry['transportation']['destination']['name']

        timediff = (dateutil.parser.parse(departureTime) - datetime.datetime.now(datetime.timezone.utc)).total_seconds()

        timestr = ''
        if timediff < 60:
            timestr = '<1m'
        elif timediff < 6000:
            timestr = str(int(timediff) // 60) + 'm'

        timestr= str.rjust(timestr, 3)
        trainstr = str.ljust(trainNumber, 3)
        print(timestr, trainstr, destination)
        arduino_payload += timestr + ' ' + trainstr + ' ;' + destination + ';'

    # Since special character can't be displayed out of box,
    # first just replace these special characters.
    # TODO Later, display of special characters should be supported.
    arduino_payload = arduino_payload.replace('ü', 'ue')
    arduino_payload = arduino_payload.replace('ö', 'oe')
    arduino_payload = arduino_payload.replace('ß', 'ss')
    print('Sending to Arduino: "' + arduino_payload + '"')
    arduino.write(arduino_payload.encode())
    print()
    time.sleep(10)
