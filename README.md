# AirQuality

AirQuality es un dispositivo capaz de medir variables ambientales como humedad, temperatura, luminosidad, y nivel CO2. Para permitir una visualización de datos amigable, los mismos se envían, usando MQTT, a la plataforma IoT Beebotte en donde se ha diseñado un dashboard para tales variables.

El corazón del proyecto es el ampliamente difundido ESP32. En él se ha desarrollado el código en C que permite que todo funcione correctamente. Para ello se hace funcionar al ESP32 en modo AP generando un webserver donde se le puedan ingresar los datos de la red a la que deba conectarse como STA para poder enviar los datos adquiridos por los sensores

Se trata de un proyecto realizado en 5to año de la carrera de ingeniería electrónica.


## ¿Qué documentos encontraré en este repositorio?
En el presente repositorio se encuentra principalmente el informe final presentado a los profesores de la cátedra. En dicho informe se detalla la manera en la que se trabajó, el motivo de la selección de los sensores y microcontrolador, el funcionamiento, y las conclusiones. 

Por otro lado, se encuentra de manera libre, el software utilizado para implementar dicho dispositivo. AirQuality se programó integramente en lenguaje C utilizando el microcontrolador ESP32 y las API que ofrece Espressif.
Además, se han diseñado bibliotecas específicas para cada sensor desde cero, dado que la mayoría de los sensores utilizados no se encontraban implementados con ESP32, sino mas bien en Arduino. 


<img src="https://github.com/kevingiribuela/AirQuality/blob/main/Imagenes/a.jpg?raw=true" width=25% height=25%> <img src="https://github.com/kevingiribuela/AirQuality/blob/main/Imagenes/d.jpg?raw=true" width=25% height=25%>

<img src="https://github.com/kevingiribuela/AirQuality/blob/main/Imagenes/b.jpg?raw=true" width=25% height=25%> <img src="https://github.com/kevingiribuela/AirQuality/blob/main/Imagenes/c.jpg?raw=true" width=25% height=25%> 
