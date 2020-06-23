#ifdef DEEPLABV2_FOUND

#include <iostream>

#include <opencv2/opencv.hpp>

#include <caffe/caffe.hpp>
#include "segmentation.hpp"

namespace marker
{
    class DeepLabV2: public Segmentation
    {
    public:
        DeepLabV2(int deviceId, const std::string &prototxt, const std::string &caffemodel, const cv::Size &_internalSize):
        internalSize{_internalSize}, mean{internalSize, CV_32FC3, cv::Scalar(122.675,116.669,104.008)}
        {

            caffe::Caffe::SetDevice(deviceId);
            caffe::Caffe::set_mode(caffe::Caffe::GPU);

            CV_Assert(internalSize.area());
            net = cv::makePtr<caffe::Net<float>>(prototxt, caffe::TEST);
            net->CopyTrainedLayersFrom(caffemodel);

            std::vector<caffe::Blob<float>*> inputLayers {net->input_blobs()};
            CV_Assert(inputLayers.size() == 1);

            inputLayers[0]->Reshape(1, 3, internalSize.height, internalSize.width);
            net->Reshape();

            std::vector<caffe::Blob<float>*> outputLayers {net->output_blobs()};
            CV_Assert(outputLayers.size() == 1);


        }
        cv::Mat segment(const cv::Mat &frame)
        {
            cv::Mat resized;
            cv::resize(frame, resized, internalSize);

            std::vector<cv::Mat> inputChannels;
            
            wrapInputLayer(&inputChannels);
            preprocess(resized, &inputChannels);
            
            net->ForwardPrefilled();

            cv::Mat argMax {argmax(net->output_blobs()[0])};

            return argMax;
        }

    private:
        void preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels)
        {
            cv::Mat sample_float;
            img.convertTo(sample_float, CV_32FC3);

            cv::Mat sample_normalized;
            cv::subtract(sample_float, mean, sample_normalized);
            cv::split(sample_normalized, *input_channels);
        }

        void wrapInputLayer(std::vector<cv::Mat>* input_channels)
        {
            caffe::Blob<float>* input_layer = net->input_blobs()[0];

            int width = input_layer->width();
            int height = input_layer->height();
            float* input_data = input_layer->mutable_cpu_data();
            for (int i = 0; i < input_layer->channels(); ++i)
            {
                cv::Mat channel(height, width, CV_32FC1, input_data);
                input_channels->push_back(channel);
                input_data += width * height;
            }
        }

        cv::Mat argmax(caffe::Blob<float>* output_layer)
        {
            const std::vector<int> shape = output_layer->shape();

            const int width = shape[3];
            const int height = shape[2];
            const int channels = shape[1];

            cv::Mat argmax(height, width, CV_8UC1);

            #pragma omp parallel for
            for (int i = 0; i < height; ++i)
            {
                for(int j = 0; j < width; ++j)
                {
                    float maxVal = 0.f;
                    int argMax = 0;
                    for(int k =0; k < channels; ++k)
                    {
                        if(output_layer->data_at(0,k, i, j) > maxVal)
                        {
                            maxVal = output_layer->data_at(0, k, i, j);
                            argMax = k;
                        }
                    }
                    argmax.at<uchar>(i,j) = argMax;
                }
            }
            return argmax;
        }

    private:
        const cv::Size internalSize;
        const cv::Mat mean;
        cv::Ptr<caffe::Net<float>> net;
    };

    cv::Ptr<Segmentation> Segmentation::create(const std::string &prototxt, const std::string &caffemodel,
        const cv::Size &internalSize, int deviceId)
    {
        return cv::makePtr<DeepLabV2>(deviceId, prototxt, caffemodel, internalSize);
    }
}

#endif
