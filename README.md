# Tool to measure Sagemaker Inference containers TPM (transactions per minute)

### Build
To build dlc_tpm
```
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Usage
```
./dlc_tpm --help

Default url: http://localhost:8080/invocations
Default payload: test_tf.json

./dlc_tpm --json_file test_tf.json --threads 8 --test_time 30 --warmup_time 5

./dlc_tpm --bin_file cat.jpg --threads 8 --test_time 30 --warmup_time 5

./dlc_tpm --url http://localhost:8501/v1/models/model:predict
```

### Test using curl

#### Tensorflow
```
python3 image_to_json_tf.py cat.jpg

curl -s -d "@test_tf.json" -H 'Content-Type: application/json' \
-X POST http://localhost:8080/invocations

# to run curl in loop use:
./test_curl_tf.sh
```

#### Pytorch
```
python3 image_to_json_pt.py cat.jpg

curl -s -d "@test_pt.json" -H 'Content-Type: application/json' \
-X POST http://localhost:8080/invocations

# to run curl in loop use:
./test_curl_pt.sh
```

#### Binary
```
# Some endpoints might support binary x-image input

curl --data-binary @cat.jpg -H 'Content-Type:application/x-image' \
-X POST http://localhost:8080/invocations

# to run curl in loop use:
./test_curl_bin.sh
```
