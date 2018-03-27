# Detect-Track
Two tracking algorithms(Staple and Template) organized with cascade classifier for object detection.

! Requires: OpenCV > 2.0

## TM_track
It is designed by template matching algorithm, which predict the object position in the next frame.

## my_staple
It is designed by Staple tracking algorithm, which is a tracker that combines two image patch representations that are sensitive to complementary factors to learn a model that is inherently robust to both colour changes and deformations.

## detect_fast
It is designed by cascade classifier algorithm, which is a face detection framework that is capable of processing images extremely rapidly while achieving high detection rates.

## cascade_TM & cascade_staple
They two combines cascade classifier and tracking algorithm, which get a trade-off between speed and accurancy. But the result stills acceptable.

! Requires: OpenCV > 3.2
## cascade_staple_cnn
It is designed by combining cascade classifier, staple tracking algorithm and convolution neural network.
The data set for training the CNN is picked from the result of cascade classifier, only in this way 
can we improve the whole performance of this program.
