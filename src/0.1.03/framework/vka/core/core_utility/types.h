#pragma once
#include <vka/core/resources/unique/AccelerationStructure.h>
#include <vka/core/resources/unique/Buffer.h>
#include <vka/core/resources/unique/Image.h>
#include <vka/core/resources/unique/CmdBuffer.h>

typedef vka::Buffer_R        *Buffer;
typedef const vka::Buffer_R  *BufferRef;
typedef vka::Image_R         *Image;
typedef const vka::Image_R   *ImageRef;
typedef vka::BottomLevelAS_R *BLAS;
typedef vka::TopLevelAs_R    *TLAS;
typedef vka::CmdBuffer_R     *CmdBuffer;