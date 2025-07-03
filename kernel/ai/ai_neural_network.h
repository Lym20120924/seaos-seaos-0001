#ifndef AI_NEURAL_NETWORK_H
#define AI_NEURAL_NETWORK_H

#include <stdint.h>

#define AI_MAX_NEURONS 1024
#define AI_MAX_LAYERS 16

typedef struct {
    float weight;
    float bias;
    float activation;
} ai_neuron_t;

typedef struct {
    ai_neuron_t neurons[AI_MAX_NEURONS];
    uint32_t neuron_count;
} ai_layer_t;

typedef struct {
    ai_layer_t layers[AI_MAX_LAYERS];
    uint32_t layer_count;
    float learning_rate;
} ai_neural_network_t;

void ai_neural_network_init(void);
int ai_neural_network_predict(float* inputs, int input_count);
void ai_neural_network_train(float* inputs, float* expected_outputs, int count);
void ai_neural_network_save(const char* filename);
void ai_neural_network_load(const char* filename);

#endif