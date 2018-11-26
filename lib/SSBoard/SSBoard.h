#ifndef _SSBOARD_H_
#define _SSBOARD_H_

#include "MyDebugger.h"
#include "Buffer.h"
/*
*   Sub-system Board (SSB)
*
*   Command: A8 8A {len} {cmd} {data...} {sum} ED
*
*/
#define SSB_BUFFER_SIZE 64
#define SSB_COMMAND_WAIT_TIME 1000

class SSBoard {
    public:
        SSBoard();
        ~SSBoard();

        void Begin(Stream *busPort, Stream *debugPort);
        void SetEnableTxCalback(void (*enableTxCallback)(bool));

        Buffer *ReturnBuffer() { return &_retBuf; }

    private:
        Stream *_bus = NULL;
        MyDebugger _dbg;
        bool _enableDebug = true;
        
        void (*_enableTxCallback)(bool) = NULL;
        void EnableTx(bool);
  
        byte _buf[SSB_BUFFER_SIZE];
        Buffer _retBuf;

        inline void ResetCommandBuffer() { memset(_buf, 0, SSB_BUFFER_SIZE); }
        inline void ResetReturnBuffer() { _retBuf.reset(); }

        void ShowCommand();
        void ClearRxBuffer();
        bool SendCommand(bool expectReturn);
        bool CheckReturn();
        void IsReturnCompleted();
};

#endif