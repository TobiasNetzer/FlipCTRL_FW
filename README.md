# FlipCTRL_FW

This is an example application for the [FlipCTRL](https://github.com/TobiasNetzer/FlipCTRL_HW) hardware.

## Features

* Wi-Fi setup via webserver
* SNTP time synchronization
* OpenWeatherMap API integration
* OpenWeatherMap configuration via webserver
* Driver for Flip-Dot matrix display over RS485

### Time Screen
![Image](docs/flipdot_time_app.png)

Time is automatically synchronized once per hour with the NTP server. The bottom bar also indicates the current weekday.

### Weather Screen
![Image](docs/flipdot_weather_app.png)

An API request is made every 15 minutes to retrieve the current weather conditions from OpenWeatherMap. The display shows a weather icon alongside the current temperature. Humidity is additionally represented by a horizontal bar at the bottom of the matrix display.

## TODO

* Improve webserver - Currently very basic implementation and look
* Add additional screens
* Add support for local temperature/humidity sensor via expansion port

## Credit

* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) is the basis for most of the drawing functions