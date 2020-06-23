#include "reverse_player.hpp"

marker::ReversePlayer::ReversePlayer(const std::string &videoFile):
    cap{videoFile}
{
    CV_Assert(cap.isOpened());

    frameIndex = cap.get(cv::CAP_PROP_FRAME_COUNT);


    if(frameIndex <= 0) //opencv отказывается нормально открывать некторые файлы, приходиться применять следующий грязный хак. Переделаю в ближайшее время
    {
        cv::Ptr<cv::VideoWriter> writer;
        cv::Mat frame;
        cap >> frame;

        CV_Assert(!frame.empty());

        const int fps = cap.get(cv::CAP_PROP_FPS) > 0 ? cap.get(cv::CAP_PROP_FPS) : 20;

        for(;;)
        {
            if(writer.empty())
                writer = cv::makePtr<cv::VideoWriter>("/tmp/temp.avi", CV_FOURCC_DEFAULT, fps, frame.size());

            writer->write(frame);

            cap >> frame;
            if(frame.empty())
                break;
        }

        writer.release();

        cap = cv::VideoCapture("/tmp/temp.avi");

        CV_Assert(cap.isOpened());

        frameIndex = cap.get(cv::CAP_PROP_FRAME_COUNT);

        CV_Assert(frameIndex > 0);
    }
}

cv::Mat marker::ReversePlayer::next()
{
    return getFrame(--frameIndex);
}

cv::Mat marker::ReversePlayer::previous()
{
    return getFrame(++frameIndex);
}

cv::Mat marker::ReversePlayer::jump(long jumpFrameIndex)
{
    frameIndex = jumpFrameIndex;    
    return getFrame(frameIndex);
}

cv::Mat marker::ReversePlayer::getFrame(long frameIndex)
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

