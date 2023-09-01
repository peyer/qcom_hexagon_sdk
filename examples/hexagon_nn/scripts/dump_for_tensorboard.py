# Takes a tensorflow model and generate a file that can be fed into
# Tensorboard to display the corresponding graph.

# Usage: python dump_for_tensorboard.py <tf_model_file> <output_dir>

#!/usr/bin/python
import sys
import os
import os.path
import tensorflow as tf
from tensorflow.python.platform import gfile

model_filename = sys.argv[1]
output_dirname = sys.argv[2]

if not os.path.exists(output_dirname):
    os.makedirs(output_dirname)
with tf.Session() as sess:
    with gfile.FastGFile(model_filename, 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())
        _ = tf.import_graph_def(graph_def, name='')
    writer = tf.summary.FileWriter(output_dirname, tf.get_default_graph())
    writer.close()

