# Connect to the wifi hotspot of the device
# Images converted with https://convertio.co/de/png-rgb/

curl -X POST http://192.168.4.1/write/00000000 --data-binary "@dmb_0.rgb"
curl -X POST http://192.168.4.1/write/001C2000 --data-binary "@dmb_14.rgb"