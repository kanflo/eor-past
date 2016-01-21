# ESP8266 flash parameter storage

This module adds support for storing parameters in the internal flash of the ESP8266.

###Usage

```
cd esp-open-rtos/extras
git clone https://github.com/kanflo/eor-past.git past
```

Include the driver in your project makefile as any other extra component:

```
EXTRA_COMPONENTS=extras/past
```

See ```example/past.c``` for a complete example while noting that the makefile depends on the environment variable ```$EOR_ROOT``` pointing to your ESP8266 Open RTOS root.
