import sys
import torch
import json
import torchvision.transforms as transforms
from PIL import Image

import numpy as np

img_path = sys.argv[1]
input_image = Image.open(img_path).convert('RGB')

preprocess = transforms.Compose([
    transforms.Resize(256),
    transforms.CenterCrop(224),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
])
input_tensor = preprocess(input_image)
input_batch = input_tensor.unsqueeze(0)
x = input_batch.numpy()
with open("test_pt.json", "w") as f:
  json.dump(x.tolist(), f)

