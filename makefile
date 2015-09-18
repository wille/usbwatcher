usbwatcher: ./src/usbwatcher.cpp
	g++ -Wall ./src/usbwatcher.cpp -o usbwatcher

noconsole: ./src/usbwatcher.cpp
	g++ -DNO_CONSOLE -Wall ./src/usbwatcher.cpp -o usbwatcher