#include <TargetConditionals.h>
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#if !TARGET_OS_SIMULATOR
#define MTLFX_PRIVATE_IMPLEMENTATION
#endif
#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#if !TARGET_OS_SIMULATOR
#include <MetalFX/MetalFX.hpp>
#endif
