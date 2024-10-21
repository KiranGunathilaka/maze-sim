# mms-cpp

Micromouse maze-solving algorithm written for the Jerry2D2 micro mouse.

For use with [mackorone/mms](https://github.com/mackorone/mms), Micromouse simulator.

## How to run this code on mackorone's mms. (Below are from his readme.md)
### Setup

1. Clone this repository
1. [Download the Micromouse simulator](https://github.com/mackorone/mms#download)
1. Run the simulator and click the "+" button to configure a new algorithm
1. Enter the config for your algorithm (name, directory, build command, and run command)
1. Click the "Run" button

### Examples

![](https://github.com/mackorone/mms-cpp/blob/master/config-windows.png)



## Notes

- If you're using Windows, you may need to download and install [MinGW](http://mingw.org/wiki/Getting_Started)
- Communication with the simulator is done via stdin/stdout, use stderr to print output
- Descriptions of all available API methods can be found at [mackorone/mms#mouse-api](https://github.com/mackorone/mms#mouse-api)
- The example code is a simple left wall following algorithm
