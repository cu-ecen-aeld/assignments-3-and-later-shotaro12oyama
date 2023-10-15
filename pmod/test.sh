#! /bin/sh

echo "Pmod Test!"
# if needed please conduct:
# sudo -s
# ./pmod_load
# echo "pmod 0x40" > /sys/bus/i2c/devices/i2c-1/new_device
var=$(cat /sys/bus/iio/devices/iio\:device0/in_temp_raw)
echo "temperature value(Celsius) is :" 
result=`echo "scale=2; $var / 65536 * 165 - 40" | bc`
echo $result

var2=$(cat /sys/bus/iio/devices/iio\:device0/in_humidityrelative_raw)
echo "humidity value(%) is :" 
result=`echo "scale=2; $var2 / 65536 * 100" | bc`
echo $result
