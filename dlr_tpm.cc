// g++ -O3 -std=c++17 dlr_tpm.cc -o dlr_tpm -ldlr -lpthread -L/root/workspace/neo-ai-dlr/build/lib
#define _POSIX_C_SOURCE 200809L
#include "dlr.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <thread>
#include <iostream>
#include <vector>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::vector;

int th_num = 1;
int warmup_time = 2;
int test_time = 10;

void readArgs(int argc, char** argv) {
  vector<string> args(argv + 1, argv + argc);

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      cout << "Syntax: run-dlr "
              "--threads <n> "
              "--test_time <n> "
              "--warmup_time <n>"
           << endl;
      exit(1);
    } else if (*i == "--threads") {
      th_num = stoi(*++i);
    } else if (*i == "--warmup_time") {
      warmup_time = stoi(*++i);
    } else if (*i == "--test_time") {
      test_time = stoi(*++i);
    }
  }
  cout << "Number of threads: " << th_num << endl;
  cout << "Warmup time sec: " << warmup_time << endl;
  cout << "Test time sec: " << test_time << endl;
  cout << "==================================================" << endl;
}

int run(int th_id, DLRModelHandle* model, float* input, float* output0) {
  const char* input_name="input_10/_0";
  const int64_t in_shape[4] = {1,3,224,224};
  if (SetDLRInput(model, input_name, in_shape, input, 4) != 0) {
    printf("Error: Could not set input %s\n", input_name);
    return -1;
  }
  if (RunDLRModel(model) != 0) {
    printf("RunDLRModel Error %s\n", DLRGetLastError());
    return -1;
  }
  if (GetDLROutput(model, 0, (void*) output0) != 0) {
    printf("GetDLROutput Error %s\n", DLRGetLastError());
    return -1;
  }
  printf("%d: run() Done\n", th_id);
  return 0;
}

void tpm_runner(int th_id, DLRModelHandle* model, float* input, time_t ss, int* my_cnt) {
  int batch_size = 1;
  int cnt = 0;
  time_t start_time = ss + warmup_time;
  time_t stop_time = start_time + test_time;
  time_t break_time = stop_time + warmup_time;
  float* output0 = (float*) calloc(1*1000*1, sizeof(float));
  while(true) {
    if (run(th_id, model, input, output0) != 0) {
      printf("Run method returns non-zero. Exit");
      exit(4);
    }
    time_t t0 = time(NULL);
    if (t0 >= start_time && t0 < stop_time) {
      cnt++;
    } else if (t0 >= break_time) {
      break;
    }
  }
  std::cout << th_id << ": Done, cnt: " << cnt << endl;
  *my_cnt = cnt;
  free(output0);
}

int main(int argc, char** argv) {
  readArgs(argc, argv);
  const char* dlr_ver;
  GetDLRVersion(&dlr_ver);
  printf("DLR version: %s\n", dlr_ver);
  struct timespec start, end;
  const char* model_file = "./model_ioc_compiled/compiled_models/1/AwsNeoTvm_0_abd61e3127e039dc";
  printf("Loading model...\n");
  DLRModelHandle model = NULL;
  //DLR_TFConfig tf_config = {};
  int batch_size = 1;
  if (CreateDLRModel(&model, model_file, 1, 0)) {
    printf("CreateDLRModel Error %s\n", DLRGetLastError());
    return -1;
  }
  printf("Model was created\n");
  const char* backend;
  int res_code = GetDLRBackend(&model, &backend);
  printf("Backend: %s, code: %d\n", backend, res_code);

  int num_inputs = 0;
  res_code = GetDLRNumInputs(&model, &num_inputs);
  printf("NumInputs: %d\n", num_inputs);
  for (int i = 0; i < num_inputs; ++i) {
    const char* i_name;
    if (GetDLRInputName(&model, i, &i_name)) {
      printf("GetDLRInputName Error %s\n", DLRGetLastError());
      return -1;
    }
    printf("Input %d, name: %s\n", i, i_name);
  }
  //exit(1);
  int num_outputs = 0;
  res_code = GetDLRNumOutputs(&model, &num_outputs);
  printf("NumOutputs: %d, code: %d\n", num_outputs, res_code);
  for (int i = 0; i < num_outputs; ++i) {
    const char* o_name;
    if (GetDLROutputName(&model, i, &o_name)) {
      printf("GetDLROutputName Error %s\n", DLRGetLastError());
      return -1;
    }
    printf("Output %d, name: %s\n", i, o_name);
  }

  float* input = (float*) calloc(1*3*224*224, sizeof(float));
  float* output0 = (float*) calloc(1*1000*1, sizeof(float));

  printf("Warming up...\n");
  int N = 10;
  for (int i = 0; i < N ; i++) {
    if (run(0, &model, input, output0) != 0) {
      return -1;
    }
  }
  // Get Output Size and Dim 
  for (int i = 0; i < num_outputs; ++i) {
    int64_t o_size = 0;
    int o_dim = 0;
    if (GetDLROutputSizeDim(&model, i, &o_size, &o_dim)) {
      printf("GetDLROutputSizeDim Error %s\n", DLRGetLastError());
      return -1;
    }
    printf("Output %d, size: %ld, dim: %d\n", i, o_size, o_dim);
  }

  time_t ss = time(NULL);
  int all_cnt[th_num];
  vector<std::thread> th_vector;
  for (int i = 0; i < th_num; i++) {
    th_vector.emplace_back(tpm_runner, i, &model, input, ss, &(all_cnt[i]));
  }
  for (auto& t : th_vector) {
    t.join();
  }

  free(output0);
  free(input);

  std::cout << "All Done" << endl;
  int total_cnt = 0;
  for (int i = 0; i < th_num; i++) {
    total_cnt += all_cnt[i];
  }
  float tpm = total_cnt * 60 / test_time;
  cout << "==================================================" << endl;
  cout << "Number of threads: " << th_num << endl;
  cout << "Test time sec: " << test_time << endl;
  cout << "Total N of Transactions: " << total_cnt << endl;
  printf("TPM: %.2f\n", tpm);
  cout << "==================================================" << endl;
  return 0;
}
