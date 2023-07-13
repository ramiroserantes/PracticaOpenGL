# Ejercicio 3

### Engadide un mapa difuso nas caras do(s) modelo(s) para o cálculo de Phong, o que permite ter distintas propiedades de reflexión difusa por fragmento.

1) Se ha modificado el fichero spinningcube_withlight_fs.glsl para permitir que se añada el mapa difuso
en el material que compone los objetos de la escena siguiendo las diapositivas de teoría (y el enlace que aparece en ellas).

2) Se ha modificado el fichero spinningcube_withlight.cpp para:
   - Añadir nuevos imports y variables necesarias, así como un nuevo flag -lstdc++ en el makefile
   - En las matrices de vértices de los objetos se añadieron nuevos coeficientes para soportar las texturas, así como actualizar las funciones que las emplean para poder usarlos.
   - Cargar la textura en el mapa difuso con la función de loadTexture() (proporcionada en el enlace de la diapositiva de teoría).
   - Al renderizar se le asigna la textura para el cubo y la pirámide.
