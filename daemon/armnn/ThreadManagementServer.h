/* Copyright (C) 2019-2020 by Arm Limited. All rights reserved. */
#pragma once

#include "armnn/IAcceptor.h"
#include "armnn/ISession.h"
#include "armnn/IStartStopHandler.h"

#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

namespace armnn {
    /**
     * Class that takes management of threads and kills them when they are done
     **/
    class ThreadManagementServer : public ICaptureStartStopHandler {
    public:
        ThreadManagementServer(std::unique_ptr<IAcceptor> acceptor);
        void stop();
        ~ThreadManagementServer() override { stop(); };

        /**
         * Enables the capture on all capture sessions
         **/
        void startCapture() override;

        /**
         * Disables the capture on all capture sessions
         **/
        void stopCapture() override;

    private:
        struct ThreadData {
            std::unique_ptr<std::thread> thread;
            std::unique_ptr<ISession> session;
            std::unique_ptr<bool> done;
        };

        std::mutex mMutex;
        std::condition_variable mSessionDiedCV;
        std::vector<ThreadData> mThreads;
        bool mEnabled;
        bool mDone;
        std::unique_ptr<IAcceptor> mAcceptor;
        std::thread mReaperThread;
        std::thread mAcceptorThread;
        bool mIsRunning;

        void reaperLoop();
        void acceptLoop();
        void closeThreads();
        void runIndividualThread(ISession & session, bool & done);
        void addThreadToVector(ThreadData threadData);
        void removeCompletedThreads();
    };
}
