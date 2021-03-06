#ifndef EXAMPLE_GENERATOR_H
#define EXAMPLE_GENERATOR_H

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper/bounding_box.h"
#include "loader/loader_imagenet_det.h"
#include "loader/video.h"
#include "helper/Common.h"
#include "helper/CommonCV.h"
#include "helper/Constants.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <gsl/gsl_rng.h> 
#include <gsl/gsl_randist.h> /* GAUSSIAN*/

struct BBParams {
  double lambda_shift;
  double lambda_scale;
  double min_scale;
  double max_scale;
};

// Generates additional training examples by taking random crops of the target object,
// causing apparent translation and scale changes.
class ExampleGenerator
{
public:
  ExampleGenerator(const double lambda_shift, const double lambda_scale,
                   const double min_scale, const double max_scale);

  // Set up to train on the previous and current image, and the previous and current bounding boxes.
  void Reset(const BoundingBox& bbox_prev, const BoundingBox& bbox_curr,
             const cv::Mat& image_prev, const cv::Mat& image_curr);

  // Shift the whole bounding box for the current frame
  // (simulates camera motion)
  void MakeTrainingExampleBBShift(const bool visualize_example,
                                  cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;
  void MakeTrainingExampleBBShift(cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;

  // Focus the current image at the location of the previous frame's
  // bounding box (true motion).
  void MakeTrueExample(cv::Mat* image_focus, cv::Mat* target_pad,
                       BoundingBox* bbox_gt_scaled) const;
  void MakeTrueExampleTight(cv::Mat* image_focus, cv::Mat* target_tight,
                      BoundingBox* bbox_gt_scaled) const;

  // Make batch_size training examples according to the input parameters.
  void MakeTrainingExamples(const int num_examples, std::vector<cv::Mat>* images,
                            std::vector<cv::Mat>* targets,
                            std::vector<BoundingBox>* bboxes_gt_scaled);

  // Helper function to generate one moved box, non-gaussian version
  // trans_range, scale_range are defaults for uniform sampling of uniform samples
  static BoundingBox GenerateOneRandomCandidate(BoundingBox &bbox, gsl_rng* rng, int W, int H, 
                                                const string method = "uniform", const double trans_range = POS_TRANS_RANGE, const double scale_range = POS_SCALE_RANGE, 
                                                const double sd_x = SD_X, const double sd_y = SD_Y, const double sd_scale = SD_SCALE);

  // Make candidates given one frame
  // candidates: 200 neg and 50 pos candidates 
  // labels: vector of scalar 1 means pos and 0 means neg 
  // fixed number output, candidates and labels will have size == num_pos + num_neg
  void MakeCandidatesAndLabels(vector<Mat> *candidates, vector<double> *labels, 
                               const int num_pos = POS_CANDIDATES,
                               const int num_neg = NEG_CANDIDATES);

  // Actual woker under MakeCandidatesAndLabels
  void MakeCandidatesAndLabelsBBox(vector<BoundingBox> *candidate_bboxes, vector<double> *labels,
                                   const int num_pos = POS_CANDIDATES,
                                   const int num_neg = NEG_CANDIDATES);
  
  void MakeCandidatesPos(vector<BoundingBox> *candidates, const int num = POS_CANDIDATES,
                                  const string method = "gaussian", const double trans_range = POS_TRANS_RANGE, const double scale_range = POS_SCALE_RANGE,
                                  const double sd_x = SD_X, const double sd_y = SD_Y, const double sd_scale = SD_SCALE, const double pos_iou_th = POS_IOU_TH );

  void MakeCandidatesNeg(vector<BoundingBox> *candidates, const int num = NEG_CANDIDATES,
                                  const string method = "uniform", const double trans_range = 2 * SD_X, const double scale_range = POS_SCALE_RANGE,
                                  const double sd_x = SD_X, const double sd_y = SD_Y, const double sd_scale = SD_SCALE);

  void set_indices(const int video_index, const int frame_index) {
    video_index_ = video_index; frame_index_ = frame_index;
  }

private:
  void MakeTrainingExampleBBShift(const bool visualize_example,
                                  const BBParams& bbparams,
                                  cv::Mat* image_rand_focus,
                                  cv::Mat* target_pad,
                                  BoundingBox* bbox_gt_scaled) const;

  void VisualizeExample(const cv::Mat& target_pad,
                        const cv::Mat& image_rand_focus,
                        const BoundingBox& bbox_gt_scaled) const;

  void get_default_bb_params(BBParams* default_params) const;

  // To generate synethic examples, shift the bounding box by an exponential with the given lambda parameter.
  double lambda_shift_;

  // To generate synethic examples, shift the bounding box by an exponential with the given lambda parameter.
  double lambda_scale_;

  // Do not scale the synthetic examples by more than max_scale_ shrink them by more than min_scale_.
  double min_scale_;
  double max_scale_;

  // Current training image.
  cv::Mat image_curr_;

  // Location of the target within the current and previous images.
  BoundingBox bbox_curr_gt_;
  BoundingBox bbox_prev_gt_;

  // Cropped and scaled image of the target object from the previous image.
  cv::Mat target_pad_;
  // tight previous target in previous image
  cv::Mat target_tight_;

  // Video and frame index from which the current example was generated.
  // These values are only used when saving images to a file, to assign them
  // a unique identifier.
  int video_index_;
  int frame_index_;

  gsl_rng* rng_;

  static std::mt19937 engine_;
};

#endif // EXAMPLE_GENERATOR_H
