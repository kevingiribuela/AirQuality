# AirQuality
AirQuality es un proyecto realizado en 5to año de la carrera de ingeniería electrónica. Se trata de un proyecto realizado para la materia de "Taller de Sistemas Digitales".

AirQuality es un dispositivo IoT capaz de adquirir variables ambientales tales como lo son la temperatura, la humedad, la luminosidad, y el CO2 presente en una sala para luego transmitirlos por WiFi vía MQTT a un servidor IoT como lo es BeeBotte. 


## ¿Qué documentos encontraré en este repositorio?
En el presente repositorio se encuentra principalmente el informe final presentado a los profesores de la cátedra. En dicho informe se detalla la manera en la que se trabajó, el motivo de la selección de los sensores y microcontrolador, el funcionamiento, y las conclusiones. 

Por otro lado, se encuentra de manera libre, el software utilizado para implementar dicho dispositivo. AirQuality se programó integramente en lenguaje C utilizando el microcontrolador ESP32 y las API que ofrece Espressif.
Además, se han diseñado bibliotecas específicas para cada sensor desde cero, dado que la mayoría de los sensores utilizados no se encontraban implementados con ESP32, sino mas bien en Arduino. 
