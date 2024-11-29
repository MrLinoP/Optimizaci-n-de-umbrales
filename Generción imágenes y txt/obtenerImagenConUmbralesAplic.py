import cv2
import numpy as np
from PIL import Image

# Leer la imagen

ruta_imagen = 'C:/Users/DELL/Downloads/radiografia-torax-comprar-rayos-x.jpg';
imagen = cv2.imread(ruta_imagen, cv2.IMREAD_GRAYSCALE)

# Definir los umbrales obtenidos del programa en c++
umbrales = [41, 72, 151, 182, 223]

# Aplicar los umbrales
imagen_umbrales = np.digitize(imagen, bins=umbrales, right=True) * 255 // len(umbrales)

# Guardar la imagen resultante
imagen_salida = Image.fromarray(imagen_umbrales.astype(np.uint8))
imagen_salida.save('C:/Users/DELL/Downloads/radiografia.jpg')