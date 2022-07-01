from tensorflow.keras.preprocessing import image
from tensorflow.keras.applications.resnet50 import preprocess_input, decode_predictions
import numpy as np
import json
import sys

img_path = sys.argv[1]
img = image.load_img(img_path, target_size=(224, 224))
x = image.img_to_array(img)
x = np.expand_dims(x, axis=0)
x = preprocess_input(x)
batch_size = 1
x = np.concatenate([x] * batch_size)
body = json.dumps({"signature_name": "serving_default", "instances": x.tolist()})
with open("test.json", "w") as f:
  f.write(body)
