import cv2
import numpy as np
from PIL import Image

# Cargar la imagen
imagen_ruta = 'C:/Users/DELL/Downloads/radiografia-torax-comprar-rayos-x.jpg'
imagen = Image.open(imagen_ruta).convert('L')

# Convertir la imagen a un arreglo de numeros
arreglo_imagen = np.array(imagen)

# Guardar el arreglo en un archivo de texto
np.savetxt('C:/Users/DELL/Downloads/radiografia.txt', arreglo_imagen, fmt="%d")

