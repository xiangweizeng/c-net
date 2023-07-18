cmake_minimum_required(VERSION 3.1) 

project(nn-case)

# for CMAKE_CXX_STANDARD
set(CMAKE_CXX_STANDARD 11)

add_executable(nn-case
        NetworkCase.cpp
        QuantizeData.cpp
        CNetCase.cpp
        CNetLayerCase.cpp
        CNetQuantizer.cpp
        layer/IgnoreLayerCase.cpp
        layer/ActivationCase.cpp
        layer/BatchNormCase.cpp
        layer/BinaryOpCase.cpp
        layer/ConcatCase.cpp
        layer/ConvolutionCase.cpp
        layer/CropCase.cpp
        layer/EltwiseCase.cpp
        layer/InnerProductCase.cpp
        layer/InterpCase.cpp
        layer/MemoryDataCase.cpp
        layer/PaddingCase.cpp
        layer/PermuteCase.cpp
        layer/PoolingCase.cpp
        layer/PReLUCase.cpp
        layer/ReshapeCase.cpp
        layer/ShuffleChannelCase.cpp
        layer/SigmoidCase.cpp
        layer/SliceCase.cpp
        layer/SoftmaxCase.cpp
        layer/UnaryOpCase.cpp
        )

target_include_directories(nn-case PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(nn-case  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/ncnn/src )
target_link_libraries(nn-case PRIVATE ncnn ${OpenCV_LIBS})

if(OpenMP_CXX_FOUND)
    if(NCNN_CMAKE_VERBOSE)
        message("Building with OpenMP")
    endif()
    target_link_libraries(nn-case PUBLIC OpenMP::OpenMP_CXX)
endif()