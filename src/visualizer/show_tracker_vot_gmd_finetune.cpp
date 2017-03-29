// Visualize the tracker performance.

#include <string>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "network/regressor.h"
#include "loader/loader_vot.h"
#include "tracker/tracker.h"
#include "tracker/tracker_gmd.h"
#include "tracker/tracker_manager.h"

// for fine tuning
#include "network/regressor_train.h"
#include "train/example_generator.h"
#include "train/tracker_trainer_multi_domain.h"

using std::string;

// Set to true to show more detailed tracking visualizations.
const bool show_intermediate_output = false;

int main (int argc, char *argv[]) {
  if (argc < 9) {
    std::cerr << "Usage: " << argv[0]
              << " deploy.prototxt network.caffemodel solver_file videos_folder MIN_SCALE MAX_SCALE GPU_ID RANDOM_SEED"
              << " [gpu_id] [video_num] [pauseval]" << std::endl;
    return 1;
  }

  ::google::InitGoogleLogging(argv[0]);

  const string& model_file   = argv[1];
  const string& trained_file = argv[2];
  const string& solver_file = argv[3];
  const string& videos_folder = argv[4];
  const double lambda_shift   = atof(argv[5]);
  const double lambda_scale   = atof(argv[6]);
  const double min_scale      = atof(argv[7]);
  const double max_scale      = atof(argv[8]);

  int gpu_id = 0;
  if (argc >= 10) {
    gpu_id = atoi(argv[9]);
  }

  int start_video_num = 0;
  if (argc >= 11) {
    start_video_num = atoi(argv[10]);
  }

  int pause_val = 1;
  if (argc >= 12) {
    pause_val = atoi(argv[11]);
  }

  // Set up the neural network.
  const bool do_train = true;
  RegressorTrain regressor_train(model_file,
                               trained_file,
                               gpu_id,
                               solver_file,
                               3,
                               do_train);

  TrackerGMD tracker_gmd(show_intermediate_output);

  // Get videos.
  LoaderVOT loader(videos_folder);
  std::vector<Video> videos = loader.get_videos();

  // Get example_generator
  ExampleGenerator example_generator(lambda_shift, lambda_scale,
                                    min_scale, max_scale); // TODO: change to from input instead

  // Visualize the tracker performance.
  TrackerFineTune tracker_fine_tune(videos, &regressor_train, &tracker_gmd, &example_generator, &regressor_train);
  tracker_fine_tune.TrackAll(start_video_num, pause_val);

  return 0;
}