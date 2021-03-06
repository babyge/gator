/* Copyright (C) 2019-2020 by Arm Limited. All rights reserved. */

#include "MaliHwCntrTask.h"

#include "IBlockCounterFrameBuilder.h"
#include "IBufferControl.h"
#include "Logging.h"
#include "SessionData.h"
#include "lib/Syscall.h"

#include <unistd.h>
#include <utility>

namespace mali_userspace {
    MaliHwCntrTask::MaliHwCntrTask(std::unique_ptr<IBufferControl> buffer,
                                   std::unique_ptr<IBlockCounterFrameBuilder> frameBuilder,
                                   std::int32_t deviceNumber,
                                   IMaliDeviceCounterDumpCallback & callback_,
                                   IMaliHwCntrReader & reader)
        : mBuffer(std::move(buffer)),
          mFrameBuilder(std::move(frameBuilder)),
          mCallback(callback_),
          mReader(reader),
          deviceNumber(deviceNumber)
    {
    }

    void MaliHwCntrTask::execute(int sampleRate,
                                 bool isOneShot,
                                 std::uint64_t monotonicStarted,
                                 std::function<void()> endSession)
    {
        bool terminated = false;
        // set sample interval, if sample rate == 0, then sample at 100Hz as currently the job dumping based sampling does not work... (driver issue?)
        const uint32_t sampleIntervalNs =
            (sampleRate > 0 ? (sampleRate < 1000000000 ? (1000000000U / sampleRate) : 1U) : 10000000U);

        if (!mReader.startPeriodicSampling(sampleIntervalNs)) {
            logg.logError("Could not enable periodic sampling");
            terminated = true;
        }
        // create the list of enabled counters
        const MaliDeviceCounterList countersList(mReader.getDevice().createCounterList(mCallback));
        while (!terminated) {
            SampleBuffer waitStatus = mReader.waitForBuffer(10000);

            switch (waitStatus.status) {
                case WAIT_STATUS_SUCCESS: {
                    if (waitStatus.data) {
                        const uint64_t sampleTime = waitStatus.timestamp - monotonicStarted;
                        if (mFrameBuilder->eventHeader(sampleTime) && mFrameBuilder->eventCore(deviceNumber)) {
                            mReader.getDevice().dumpAllCounters(
                                mReader.getHardwareVersion(),
                                countersList,
                                reinterpret_cast<const uint32_t *>(waitStatus.data.get()),
                                waitStatus.size / sizeof(uint32_t),
                                *mFrameBuilder,
                                mCallback);
                            mFrameBuilder->check(sampleTime);
                        }
                    }
                    break;
                }
                case WAIT_STATUS_TERMINATED: {
                    logg.logMessage("Stopped capturing HW counters");
                    terminated = true;
                    break;
                }
                case WAIT_STATUS_ERROR:
                default: {
                    logg.logError("Error - Stopped capturing HW counters");
                    break;
                }
            }
            if (isOneShot && (mBuffer->isFull())) {
                logg.logMessage("One shot (malihwc)");
                endSession();
            }
        }

        if (!mReader.startPeriodicSampling(0)) {
            logg.logError("Could not disable periodic sampling");
        }
        mBuffer->setDone();
    }

    bool MaliHwCntrTask::write(ISender & sender) { return mBuffer->write(sender); }
}
