#ifndef __ROAD_SCENE_MARKER_HPP__
#define __ROAD_SCENE_MARKER_HPP__

#include <string>

namespace marker
{
/*! @brief маркер дорожной сцены
 * @param video - путь к размечаемому видео
 * @param prototxt - путь к prototxt файлу модели DeepLabV2 (лежит в папке data, deploy_4.prototxt)
 * @param weights - путь к caffemodel файлу весов DeepLabV2 (лежит в папке data, train_iter_20000_4.caffemodel)
*/
void markScene(const std::string &video, const std::string &prototxt, const std::string &weights);

/*! @brief экспортер дорожной сцены
 * @param video - путь к размечаному видео
*/
void exportScene(const std::string &video, const std::string &group);
}

#endif
