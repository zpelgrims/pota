#include <ai.h>
#include <cstring>

typedef void (*NodeRegisterFunc)(AtNodeLib* node);

void registerLentilCamera(AtNodeLib* node);
// void registerLentilCameraDebug(AtNodeLib* node);
void registerLentilFilter(AtNodeLib* node);
void registerLentilImager(AtNodeLib* node);
// void registerLentilOperator(AtNodeLib* node);

static NodeRegisterFunc registry[] = {
    &registerLentilCamera,
    // &registerLentilCameraDebug,
    &registerLentilFilter,
    &registerLentilImager
    // &registerLentilOperator
};

static const int num_nodes = sizeof(registry) / sizeof(NodeRegisterFunc);

node_loader {
    if (i >= num_nodes)
        return false;

    strcpy(node->version, AI_VERSION);
    (*registry[i])(node);

    return true;
}