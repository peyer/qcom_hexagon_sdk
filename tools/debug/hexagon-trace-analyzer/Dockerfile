# Use an official Python runtime as a parent image
FROM python:2.7-slim

# Set the working directory to /app
WORKDIR /app

# Copy the current directory contents into the container at /app
ADD . /app

# Install any needed packages specified in requirements.txt
RUN pip install --trusted-host pypi.python.org -r requirements.txt

RUN apt-get update && apt-get install -y libncurses5 binutils wget zip perl

RUN wget https://github.com/brendangregg/FlameGraph/archive/master.zip && unzip master.zip && rm master.zip && mv FlameGraph-master FlameGraph
RUN wget https://github.com/catapult-project/catapult/archive/master.zip && unzip master.zip -x *.json "*tracing/test_data/*" && rm master.zip && mv catapult-master catapult

# Make port 80 available to the world outside this container
EXPOSE 80

# Define environment variable
ENV NAME World

# Run app.py when the container launches
#CMD ["./bootstrap",";","&&",".","pyenv27/bin/activate",";","&&",python","pyetm.pyc","./sensors/sensors_config.py","./results","./sensors/AllSensFastest_1.bin.qdat_atid034_data"]
#CMD ["./pyetm","./sensors/config.py","./results","./sensors/AllSensFastest_1.bin.qdat_atid034_data"]
#CMD ["./docker_run.sh"]
#CMD ["ls","-al"]
#CMD ["ls","-al","hexagon" ]
#CMD ["ls","-1","/usr/lib/*" ]


