import requests
import json

api_url = 'http://127.0.1.2:4444/calc'
#params ={'Light': '1.564814814814815', 'Moisture 1': '-82.96296296296295' , 'Moisture 2': '0.5597993827160495', 'Temp': '0.3445987654320987', 'Humidity': '18.203703703703702'}
#params = {'Light': {0: 1.564814814814815}, 'Moisture 1': {0: -82.96296296296295}, 'Moisture 2': {0: 0.5597993827160495}, 'Temp': {0: 0.3445987654320987}, 'Humidity': {0: 18.203703703703702}}
#paramso = [85.37037037037038,172.9259259259259,205.22222222222226,29.27037037037037,17.185185185185187]
params =[ "0.08611226061567466" , "0.08156113102349655", "0.08850692662904062", "0.22089688050165154",  "0.2149282761974228"]
j_data = json.dumps(params)



response = requests.post(url= api_url, json= params)

print(response.text)










