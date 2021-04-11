import json
from flask import Flask, request, jsonify
import pandas as pd
#import tensorflow as tf
import csv
import keras
from tensorflow.keras.models import load_model
import numpy as np


# ------- LOAD MODEL -------

model = load_model(r"Tensorflow Models/irrigation.h5")

# ------- LIST ASSIGNMENT -------
min_max_values = []

# ------ READING MinMaxValue Csv Created During Normalization --------
with open(r'CSVs/Target/minMaxVals.csv','rt')as f:
  data = csv.reader(f)
  for row in data:
    min_max_values.append(row[0])
# ------ Converting to Float --------

for i in range(2,len(min_max_values)):
  min_max_values[i] = float(min_max_values[i])



# ------ FLASK APP START ------

app = Flask(__name__)
@app.route("/")
def index():
    return "Atleat server is working !!! Hello Lets ML!"


# define a predict function as an endpoint
@app.route("/calc", methods=["GET","POST"])

def predict():
    # paramIndex = [7,5,6,3,4]

    data = []
    parList = []

    params = request.json
    parDict = params[0]

    # print("####################")
    # print(params)
    # print(parDict)
    # print("####################")

    for i in range(len(params)):
        params[i] = float(params[i])

    # parListValues = list(parDict.values())

    parList = params

    print("####################")
    print(parList)
    print("####################")

    if (params == None):
        answer = "No Data Recieved"
        return str(answer),201

    elif (params != None):

        # ------ Converting to Float --------
        for i in parList:
            data.append(float(i))
        
        # ------ Normalizing The Input To The Network --------
        data[0] = ((data[0] - (min_max_values[2])) / ((min_max_values[3])- (min_max_values[2])))
        data[1] = ((data[1] - (min_max_values[4])) / ((min_max_values[5])- (min_max_values[4])))
        data[2] = ((data[2] - (min_max_values[6])) / ((min_max_values[7])- (min_max_values[6])))
        data[3] = ((data[3] - (min_max_values[8])) / ((min_max_values[9])- (min_max_values[8])))
        data[4] = ((data[4] - (min_max_values[10])) /((min_max_values[11]) -( min_max_values[10])))

        # ------ Creating A Dataframe Object --------
        df = pd.DataFrame([data], columns=["Light", "Moisture 1", "Moisture 2", "Temp", "Humidity"])
        properties = list(df.columns.values)
        x = df[properties]
        
        # ------ Passing The inputData To Tensorflow Model --------
        prediction = (model.predict(x))
        answer = round(prediction[0][0], 0)

        # ------ Returning The Answer --------
        return str(answer),201       

@app.route("/calc", methods=["GET","POST"])

def convCsv():
    collisions = []

    # ------ READING MinMaxValue Csv Created During Normalization --------
    with open(r'collisions.csv','rt')as f:
        data = csv.reader(f)
        for row in data:
            collisions.append(row[0])
    
    collisionsTemp = collisions

    for i in range(len(collisions)):
        if i != 0:
            collisions[i] - collisionsTemp[i-1]


# start the flask app, allow remote connections
app.run(debug=True, host='127.0.1.2', port= 4444)