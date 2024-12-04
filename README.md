# TrainSpeedometer #

The *TrainSpeedometer* sketch runs on an Arduino-class microcontroller, using a pair of sensors (usually optical) to measure the scale speed of a passing model train.

This version uses a pair of [Adafruit VL6180X time-of-flight ranging sensors](https://www.adafruit.com/product/3316).  The sensors measure the time which pulses of laser light need to travel to the object being sensed, and then reflect back to the sensor.  From this the distance to the object is calculated and returned in millimeters.  These distance measures are not used to calculate the speed, but allow the code to determine if the object (the train) is present in front of the sensor or not, and is within a preset range of accepted distances.  Using the range does allow the code to ignore trains passing within the sensor's field of view (FOV) but which are not within a preset range of distances, such as trains on tracks other than the one being monitored.  It can also be used to ignore fixed objects behind the track.

## Dependencies ##
* StateMachine library from [github.com/twrackers/StateMachine-library](https://github.com/twrackers/StateMachine-library) 
* SparkFun Alphanumeric Display library from [github.com/sparkfun/SparkFun\_Alphanumeric\_Display\_Arduino\_Library](https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library)
* STM32duino VL6180X library from [github.com/stm32duino/VL6180X](https://github.com/stm32duino/VL6180X)
