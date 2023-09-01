set -e
adb push ${HEXAGON_NN}/android_Release/ship/graph_app /data
adb push ${HEXAGON_NN}/hexagon_Release_dynamic_toolv83_v65/ship/libhexagon_nn_skel.so /vendor/lib/rfsa/adsp
adb push ${HEXAGON_NN}/test/vgg_labels.txt /vendor/etc/
adb push animals_vgg_b/panda_224x224_vgg_b.dat /vendor/etc/
adb shell /data/graph_app --iters 1 /vendor/etc/panda_224x224_vgg_b.dat --labels_filename "/vendor/etc/vgg_labels.txt"

