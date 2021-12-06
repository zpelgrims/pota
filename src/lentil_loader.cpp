#include <ai.h>
#include <cstring>

typedef void (*NodeRegisterFunc)(AtNodeLib* node);

void registerLentilCameraPO(AtNodeLib* node);
void registerLentilFilterPO(AtNodeLib* node);
void registerLentilImager(AtNodeLib* node);

static NodeRegisterFunc registry[] = {
    &registerLentilCameraPO,
    &registerLentilFilterPO,
    &registerLentilImager,
};

static const int num_nodes = sizeof(registry) / sizeof(NodeRegisterFunc);

node_loader {
    if (i >= num_nodes)
        return false;

    strcpy(node->version, AI_VERSION);
    (*registry[i])(node);

    return true;
}