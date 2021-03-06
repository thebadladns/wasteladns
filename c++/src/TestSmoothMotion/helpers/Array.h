#ifndef __WASTELADNS_ARRAY_H__
#define __WASTELADNS_ARRAY_H__

#ifndef __WASTELADNS_TYPES_H__
#include "Types.h"
#endif

namespace Array {
    
    struct Params {
        void* data;
        s32* count;
        s32 maxCount;
        s32 stride;
    };
    // Array::BatchParams historyParams;
    // historyParams.data = (void**) &Controller::inputDirWSHistory;
    // historyParams.batchCount = 2;
    // historyParams.dataCount = &Controller::historyCount;
    // historyParams.dataMaxCount = Controller::historyMaxCount;
    // historyParams.dataStride = sizeof(f32);
    // Array::Queue::Push(historyParams, historyData);
    struct BatchParams {
        void** data;
        s32* dataCount;
        s32 batchCount;
        s32 dataMaxCount;
        s32 dataStride;
    };
    
    namespace Queue {
        
        template <typename _T>
        void Push(_T* data, s32& count, s32 maxCount, _T& elem);
        template <typename _T>
        void BatchPush(_T** data, u32 batchCount, s32& count, s32 maxCount, _T* elem);
        
        void Push(const Params& params, void* elem);
        void Push(const BatchParams& params, void* elem);
    }
}

#ifndef __WASTELADNS_ARRAY_IMPL__
extern template void Array::Queue::Push<f32>(f32* data, s32& count, s32 maxCount, f32& elem);
extern template void Array::Queue::BatchPush<f32>(f32** data, u32 batchCount, s32& count, s32 maxCount, f32* elem);
#endif

#endif // __WASTELADNS_ARRAY_H__

#ifdef __WASTELADNS_ARRAY_IMPL__
#undef __WASTELADNS_ARRAY_IMPL__

#ifndef __WASTELADNS_C_CSTRING_H__
#include <cstring>
#define __WASTELADNS_C_CSTRING_H__
#endif

namespace Array {
    namespace Queue {
        
        template <typename _T>
        void Push(_T* data, s32& count, s32 maxCount, _T& elem) {
            if (count >= maxCount) {
                memmove(data, &data[1], count);
                count--;
            }
            data[count++] = elem;
        }
        template <typename _T>
        void BatchPush(_T** data, u32 batchCount, s32& count, s32 maxCount, _T* elem) {
            f32* batchItem;
            u32 i;
            
            if (count >= maxCount) {
                for (i = 0, batchItem = (_T*)data; i < batchCount; i++, batchItem += maxCount) {
                    memmove(batchItem, &(batchItem[1]), sizeof(_T) * count);
                }
                count--;
            }
            
            for (i = 0, batchItem = (_T*)data; i < batchCount; i++, batchItem += maxCount) {
                batchItem[count] = elem[i];
            }
            count++;
        }
        
        void Push(const Params& params, void* elem) {
            if (*params.count >= params.maxCount) {
                memmove(params.data, (char*)params.data + params.stride, params.stride * *params.count);
                (*params.count)--;
            }
            memcpy((char*)params.data + *params.count * params.stride, elem, params.stride);
            (*params.count)++;
        }
        void Push(const BatchParams& params, void* elem) {
            s32 i;
            char* batchItem;
            s32 batchStride = params.dataMaxCount * params.dataStride;
            
            if (*params.dataCount >= params.dataMaxCount) {
                char* batchItem;
                for (i = 0, batchItem = (char*)params.data; i < params.batchCount; i++, batchItem += batchStride) {
                    memmove(batchItem, batchItem + params.dataStride, params.dataStride * *params.dataCount);
                }
                (*params.dataCount)--;
            }
            
            for (i = 0, batchItem = (char*)params.data; i < params.batchCount; i++, batchItem += batchStride) {
                memcpy(batchItem + *params.dataCount * params.dataStride, (char*)elem + params.dataStride * i, params.dataStride);
            }
            (*params.dataCount)++;
        }
    }
}

template void Array::Queue::Push<f32>(f32* data, s32& count, s32 maxCount, f32& elem);
template void Array::Queue::BatchPush<f32>(f32** data, u32 batchCount, s32& count, s32 maxCount, f32* elem);

#endif // __WASTELADNS_ARRAY_IMPL__
