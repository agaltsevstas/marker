#include "forward_player.hpp"

marker::ForwardPlayer::ForwardPlayer(const std::string &videoFile):
    cap{videoFile}
{
    CV_Assert(cap.isOpened());

    frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);
}

cv::Mat marker::ForwardPlayer::next()
{
    return getFrame(++frameIndex);
}

cv::Mat marker::ForwardPlayer::previous()
{
    return getFrame(--frameIndex);
}

cv::Mat marker::ForwardPlayer::jump(long jumpFrameIndex)
{
    frameIndex = jumpFrameIndex;
    return getFrame(frameIndex);
}

cv::Mat marker::ForwardPlayer::getFrame(long frameIndex)
{
    if(frameIndex <= 0 || frameIndex > cap.get(cv::CAP_PROP_FRAME_COUNT))
        return cv::Mat();

    const long targetIndex = frameIndex;
    const int stepBack{5};

    if(std::abs(targetIndex - cap.get(cv::CAP_PROP_POS_FRAMES)) > 15)//if we are too far
        cap.set(cv::CAP_PROP_POS_FRAMES, targetIndex);

    if(cap.get(cv::CAP_PROP_POS_FRAMES) >= targetIndex)
    {
        for(;;)
        {
            frameIndex -= stepBack;
            if(frameIndex < 0)
            {
                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                break;
            }
            else
            {
                cap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
                if(cap.get(cv::CAP_PROP_POS_FRAMES) < targetIndex)
                    break;
            }
        }
    }

    cv::Mat frame;

    for(;;)
    {
        cap >> frame;

        frameIndex = cap.get(cv::CAP_PROP_POS_FRAMES);

        if(frameIndex == targetIndex)
            break;

        CV_Assert(frameIndex < targetIndex);
    }

    return frame;
}

