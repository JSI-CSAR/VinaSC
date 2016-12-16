#include "coi_func.h"

#ifdef __MIC__
COINATIVELIBEXPORT
void mic_rcv (uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
          uint64_t*        in_pBufferLengths,
          void*            in_pMiscData,
          uint16_t         in_MiscDataLength,
          void*            in_pReturnValue,
          uint16_t         in_ReturnValueLength)
{

    int* index = (int *) in_pMiscData;
    rcv_q->push(*index);
}

COINATIVELIBEXPORT
void wait_sig (uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
          uint64_t*        in_pBufferLengths,
          void*            in_pMiscData,
          uint16_t         in_MiscDataLength,
          void*            in_pReturnValue,
          uint16_t         in_ReturnValueLength)
{
	int tmp;
  done_q.pop(tmp);
  memcpy(in_pReturnValue, &tmp, in_ReturnValueLength);
}

#endif