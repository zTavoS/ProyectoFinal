FROM python:3.9.16

WORKDIR /app

RUN apt-get update

RUN apt -y install python3-aiohttp

RUN pip install influxdb-client
RUN pip install pandas
RUN pip install numpy
RUN pip install sklearn
RUN pip install paho-mqtt


COPY Proyecto/analitica/analitica.py /app

CMD ["python", "/app/analitica.py"]