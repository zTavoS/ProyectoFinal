FROM python:3.9.16

WORKDIR /app

RUN apt-get update

RUN apt -y install python3-aiohttp

RUN pip install influxdb-client
RUN pip install pandas
RUN pip install numpy
RUN pip install sklearn
RUN pip install paho-mqtt


COPY /analitica/analitica.py /app/analitica.py

CMD ["python", "/app/analitica.py"]