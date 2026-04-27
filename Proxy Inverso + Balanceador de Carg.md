Proxy Inverso + Balanceador de Carga + Web Server (PIBL-WS)
Proyecto I
Telemática/Internet: Arquitectura y Protocolos
1. Objetivo
Desarrollar habilidades en la programación de aplicaciones distribuidas, particularmente
las que requieren de una arquitectura cliente/servidor utilizando la API sockets.
2. Introducción
Los proxies inversos, así como balanceadores de carga son elementos intermedios en el
funcionamiento y operación de aplicaciones cliente/servidor.
En este sentido, un proxy inverso se entiende como aquel que recibe (intercepta) cada
una de las peticiones del cliente y la envía a un servidor con la capacidad de procesar la
petición para finalmente enviar la respuesta al cliente. Por otro lado, un balanceador de
carga, es la entidad encargada de distribuir las peticiones entrantes por parte de los
clientes hacia un conjunto de servidores. Para cada petición, debe posteriormente, debe
retornar la respuesta al cliente. Adicional a los proxies y balanceadores de carga, se tiene
otro elemento fundamental en la arquitectura web que es el servidor web. En términos
generales la función principal de un servidor web es la entrega de recursos (páginas html,
imágenes, archivos de estilos, etc) web a un cliente que lo solicita (p.ej., web browser).
Para esto, tanto el cliente como el servidor se comunican a través de un mismo protocolo
que es HTTP.
De esta forma el objetivo final para este proyecto es desarrollar e implementar un
servidor proxy así como un servidor web denominado Telematics Web Server – (TWS))
que soporte la versión HTTP/1.1
3. Arquitectura a Desplegar.
En la figura que se observa a continuación, se presenta la arquitectura de alto nivel que
usted debe implementar para lograr el objetivo de la práctica.
La solución a implementar debe contar con los siguientes componentes:
• El servidor Proxy Inverso + Balanceador de Carga (PIBL).
• Tres servidores de aplicación web. La aplicación se debe replicar en los tres
servidores, es decir, es la misma. Para efectos de esta práctica, usted tiene la
potestad de realizar su aplicación web, para efectos de prueba, en cualquier
lenguaje de programación.
Como se aprecia, se requiere que usted disponga de una máquina
4. Recursos:
Toda la arquitectura detallada en el ítem anterior debe ser desplegada en la nube de
Amazon Web Services utilizando la cuenta que se le ha asignado en el curso. Para esto
debe utilizar emplear instancias EC2 para la instalación y configuración de su solución.
5. Aspectos por considerar para el desarrollo de la práctica:
En este proyecto se requiere que usted implemente un Proxy Inverso + Balanceador de
Carga (PIBL) + WS, que permita recibir las peticiones web del cliente, las procese, las envié
a uno de los tres (o más) servidores que se encuentran detrás de éste para finalmente
retornar la respuesta al cliente. Para lograr esto se requiere que usted considere los
siguientes aspectos:
1. Su PIBL+WS debe ser escrito en lenguaje de programación C o Rust.
2. Se debe emplear la API Sockets.
3. Se debe soportar peticiones de forma concurrente desde diferentes tipos de
clientes que envíen peticiones HTTP. Por favor debe implementar una
estrategia para lograr esto.
4. Se requiere que se procesen peticiones para la versión de protocolo HTTP/1.1.
5. Se requiere que su servidor escuche peticiones en el puerto 80 u 8080. Una
vez reciba la petición de un cliente (p.ej., browser, terminal de consola,
postman, etc), se debe iniciar un nuevo socket cliente para comunicarse con
el servidor web destino elegido, y enviar la petición a éste.
6. Una vez envié la petición al servidor, debe esperar la respuesta y enviarla al
cliente web que la solicito que solicite el recurso web.
7. Se requiere que la aplicación PIBL implemente un proceso “log” donde se
registren todas las peticiones que recibe. En este sentido, el log debe permitir
registrar todas las peticiones que se reciben y debe visualizar la petición que
se hace y la respuesta que se entrega. Esto se debe visualizar por la salida
estándar, y de igual forma, se debe implementar el registro en un archivo.
8. La función de proxy debe permitir el caché para los diferentes recursos que se
soliciten por parte de los clientes. Para esto debe considerar lo siguiente:
a. Para todos los recursos solicitados en las peticiones hecha por los
clientes, la respuesta (el recurso solicitado) debe ser almacenada en en
un archivo en el disco del servidor. De esta forma se garantiza que el
cache persista en caso tal se presente una falla en el servidor PIBL. Así,
la próxima vez que se realice la petición de este recurso, se debe
acceder desde al disco y enviar la respuesta desde aquí hacia el cliente.
b. Los recursos para almacenar en el cache deben ser localizados en el
directorio donde se ejecuta la aplicación principal del PIBL.
c. Se debe implementar un mecanismo para implementar un TTL para
cada recurso que se mantenga en el cache. Esto debe ser un parámetro
que se pase al momento de lanzar la aplicación.
9. Para efectos de distribución de la carga de las peticiones, la política a
implementar es Round Robin.
10. Su PIBL debe tener un archivo de configuración que permita parametrizar el
puerto en el que se ejecuta (p.ej., por defecto es 8080) así como incluir la lista
de servidores (backend) que contestan las peticiones.
Para las funcionalidades de servidor web:
a. Se requiere que su implementación deba ser capaz de analizar (parsing) tres tipos
de métodos a nivel de un HTTPRequest: GET|HEAD|POST|. Para para revisar la
estructura de este mensaje, se debe remitir a la especificación del estándar de
HTTP/1.1 RFC 2616 (https://datatracker.ietf.org/doc/rfc2616/). Igualmente, de
manera complementaria, puede revisar tanto las presentaciones, así como el
material de referencia (libros) suministrado para el curso. A continuación, una
breve descripción de lo que realizan los métodos.
i. GET: permite solicitar un recurso y la respuesta debe incluir los datos.
ii. HEAD: este método pide una respuesta idéntica a la de una petición GET.
La diferencia radica en que no va nada (datos) en el cuerpo de la respuesta
iii. POST: este método se utiliza para enviar una entidad (datos) al recurso
identificado en la URI y que se aloja en un servidor. Los datos enviados al
servidor van en el cuerpo (body) de la petición. Su servidor debe ser capaz
de manejar de manera robusta los errores. Para esto, debe soportar los
siguientes mensajes de error:
1. Código de respuesta: 200. La petición enviada al servidor web fue
procesada con éxito y se devolvió la respuesta al cliente.
2. Código de respuesta: 400. La petición solicitada por el cliente no
pudo ser procesada. Este código debe ser enviado siempre que no
se encuentre el recurso, no se entienda la petición, etc.
3. Código de respuesta: 404. Esta petición no puede ser procesada
por el servidor web dado que no encontró el recurso solicitado.
“Page/File Not Found”
iv. Su servidor debe soportar procesar peticiones de manera concurrente,
para esto puede implementar una aproximación Thread Based para el
manejo de la concurrencia. Esto quiere decir que puede utilizar hilos para
el manejo de múltiples peticiones.
v. Su servidor debe implementar el concepto de “logger”. Esto con el fin de
poder visualizar por la terminal todas las peticiones entrantes a nivel de
HTTP, así como la respuesta que se envía a cada cliente.
vi. Su servidor TWS se debe ejecutar de la siguiente manera:
1. $./server <HTTP PORT> <Log File> <DocumentRootFolder> Donde:
HTTP PORT es el puerto de la máquina en la cual está corriendo su
servidor web TWS. LogFile es el archivo de log que se va a generar
con toda la información concerniente a peticiones, errores, info,
etc.
2. DocumentRootFolder es la carpeta donde se alojarán los diferentes
recursos web
vii. Para efectos de prueba, se puede utilizar telnet al puerto en que este
corriendo su servidor web, script escrito en Python, Postman, wireshark o
un browser real.
viii. Casos de Prueba:
1. Despliegue: Para efectos de la prueba del funcionamiento de su
servidor, este debe ser desplegado en la infraestructura de AWS.
2. Recursos web a solicitar:
a. Caso 1. Página web con algunos hipertextos y una imagen.
b. Caso 2. Página web con algunos hipertextos y múltiples
imágenes.
c. Caso 3: Página web que contiene un solo archivo de
aproximadamente un tamaño de 1M.
d. Caso 4: Página web que contiene múltiples archivos y que
aproximadamente tiene un tamaño de 1MB
6. Grupos de trabajo:
a. El proyecto 1 debe ser desarrollado en grupos de tres personas.
7. Fechas:
a. Fecha de inicio: Marzo 26 de 2026
b. Fecha de entrega: Mayo 6 de 2026
c. Sustentaciones: Cada equipo dispondrá de 40 minutos para la presentación de los
detalles de su proyecto.
8. Mecanismo de Entrega:
a. La entrega se debe realizar por el buzón de entrega por interactiva virtual. La
documentación se debe incluir en el repo en un archivo README.md. En este
archivo se requiere que usted incluya los detalles de implementación donde como
mínimo se esperan las siguientes secciones:
i. Introducción.
ii. Desarrollo
iii. Conclusiones
iv. Referencias
9. Versión:
a. Fecha de Creación: Marzo de 2026
b. Fecha de primera actualización: Marzo de 2026