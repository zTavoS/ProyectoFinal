import paho.mqtt.client as mqtt
import time
import csv
import pandas as pd
from sklearn.linear_model import LinearRegression
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error, r2_score
from influxdb_client.client.write_api import SYNCHRONOUS
from influxdb_client import InfluxDBClient, Point, WritePrecision

token = "token-secreto"
org = "org"
bucket = "rabbit"
client1 = InfluxDBClient(url="http://3.86.13.130:8086", token=token)


# Define las variables 
mensaje_1 = '' 
mensaje_2 = '' 
mensaje_3 = '' 
count = 0

# Abre el archivo CSV en modo de escritura y define el encabezado
with open('datos.csv', 'w', newline='') as archivo_csv:
    writer = csv.writer(archivo_csv)
    writer.writerow(['Temperatura', 'Humedad', 'Sensacion Termica'])

# Define la función que se llamará cada vez que se reciba un mensaje MQTT
def on_message(client, userdata, message):
    global mensaje_1, mensaje_2, mensaje_3
    # Recupera el mensaje y lo guarda en la variable correspondiente
    if message.topic == 'Temperatura':
        mensaje_1 = str(message.payload.decode('utf-8'))
    elif message.topic == 'Humedad':
        mensaje_2 = str(message.payload.decode('utf-8'))
    elif message.topic == 'Sensacion Termica':
        mensaje_3 = str(message.payload.decode('utf-8'))

# Configura el cliente MQTT y se conecta al broker de RabbitMQ
client = mqtt.Client()
client.connect('3.86.13.130', 1883)

# Configura la función que se llamará cada vez que se reciba un mensaje
client.on_message = on_message

# Se suscribe a los topics de interés
client.subscribe('Temperatura')
client.subscribe('Humedad')
client.subscribe('Sensacion Termica')

# Inicia el loop del cliente MQTT
client.loop_start()

# Bucle infinito que se ejecutará cada 5 segundos para mostrar los mensajes
while True:
    # Abre el archivo CSV en modo de añadir para agregar los nuevos datos
    with open('datos.csv', 'a', newline='') as archivo_csv:
        writer = csv.writer(archivo_csv)
        writer.writerow([mensaje_1, mensaje_2, mensaje_3])

    # Crea un DataFrame con los datos del archivo CSV

        archivo_csv = pd.read_csv('datos.csv')
        archivo_csv= archivo_csv.dropna()
        total_rows=len(archivo_csv.axes[0])
        if total_rows > 10:
            Datos_numericos = ['Humedad', 'Sensacion Termica']
            archivo_csv[Datos_numericos + ['Temperatura']].describe()

            # Separate features and labels
            X, y = archivo_csv[['Humedad','Sensacion Termica']].values, archivo_csv['Temperatura'].values

            # Split data 70%-30% into training set and test set
            X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.30, random_state=0)

            # Fit a linear regression model on the training set
            model = LinearRegression().fit(X_train, y_train)
            predictions = model.predict(X_test)[-12:]
            np.set_printoptions(suppress=True)
            R2 = r2_score(y_test[-12:], predictions)
            MSE = mean_squared_error(y_test[-12:], predictions)
            RMSE = np.sqrt(MSE)
        

            write_api = client1.write_api(write_options=SYNCHRONOUS)

            for i in predictions:
            # Crear punto de datos
                point = Point("Analitica").tag("ubicacion", "oficina").field("prediccion", i).field("r2", R2).field("mse", MSE).field("rmse", RMSE)

                # Escribir los datos en la base de datos
                write_api.write(bucket=bucket, org=org, record=point)
                count += 1
                if count == 12:
                    break

    time.sleep(5)