import pandas as pd
from influxdb_client import InfluxDBClient

# Crea un cliente de InfluxDB
client = InfluxDBClient(url="http://54.85.203.214:8086", token="token-secreto")

# Especifica la base de datos a utilizar
org = "org"
bucket = "rabbit"

# Ejecuta una consulta
query = 'from(bucket:"{}") |> range(start: -15d) '.format(bucket)
result = client.query_api().query(org=org, query=query)
df = pd.DataFrame(result[0].records)

df.to_csv('datos.csv', index=False)