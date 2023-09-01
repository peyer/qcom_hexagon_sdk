# Converts caffe model to hexagon nn implementation
#
# Usage:
#        See command output using --help argument
#
# Restrictions:
#
#        This script was created by parsing a VGG 16 model into a working hexagon nn
#        implementation. It should be viewed as a tool illustrating how to convert
#        caffe models into hexagon nn implementations. Currently, it is not expected 
#        to work on other types of caffe models and it is solely up to the user to modify
#        the script further for their own needs.
#
#        Among the many limitations of this tool:
#        - Only the following caffe layers are currently supported:
#          - Input, Convolution, ReLU, Pooling, InnerProduct, Softmax
#        - The user must provide a properly preprocessed .dat file at application runtime
#          that is usable with your generated neural network. You can utilize jpg_to_dat.py
#          to generate this .dat file. For more information on using jpg_to_dat.py, please 
#          see the usage examples in tutorial 8.
#
# Usage "tricks":
#        All coefficients are encoded as plain C arrays which can take some time to generate
#        and compile. These coefficients are stored in a file separate from the actual graph 
#        implementation so that their generation can be bypassed when using '--leave_coefficients_unchanged'.
#        Using this option effectively, should save you a significant amount of time. Furthermore, 
#        dictionaries can be passed to the script to bypass the generated coefficients and extrema 
#        and use external ones (already tested presumably) instead.

import sys, os
import time
import numpy as np
import argparse
sys.path.append('./')
import caffe_quant_pb2 as cq
from google.protobuf import text_format
import yaml
import math
import struct

###########################################################################################
# Class definitions

class layer_parser:
    def __init__(self, num_output, kernel_size, output_width, output_height):
        self.num_output = num_output
        self.kernel_size = kernel_size
        self.output_width = output_width              
        self.output_height = output_height            

class node_type:
    def __init__(self, num_branches, size):
        self.num_branches = num_branches
        self.size = size

class node:
    def __init__(self, name, op, pad, inputs, output_type, output_index = None):
        self.name = name
        self.op = op
        self.pad = pad
        self.inputs = inputs
        self.output_type = output_type
        self.output_index = 0 if output_index is None else output_index

class branch:
    def __init__(self, name, b, w, h, d, size):
        self.name = name
        self.b = b
        self.w = w
        self.h = h
        self.d = d
        self.size = size

class layer_params:
    def __init__(self):
        self.sw = 0
        self.sy = 0
        self.sx = 0
        self.max_w = 0
        self.min_w = 0
        self.max_b = 0
        self.min_b = 0
        self.coefficient_size = 0
        self.kernel_size = 0
        self.num_input = 0
        self.num_output = 0
        self.top  = []
        self.bottom  = []
        self.output_width = 1
        self.output_height = 1

###########################################################################################
# Globals

#------------------------------------------------------------------------------------------
# Modify the entries below to support additional caffe layers

# Take no action
NA = "NA"

# Node output types
NODE_FIX32 = node_type(3,4)
NODE_FIX8 = node_type(3,1)
NODE_FLOAT = node_type(1,4)
NODE_CHAR = node_type(1,1)

# Branches available
# Branch Parameters:     (branch name, batch, width, height, depth, element size or activation branch index):
B_ACTIVATION=branch('activation','1','1','1','1',0)
B_ACTIVATION_2=branch('activation_2','1','1','1','1',1)
B_BIASES_8B=branch('biases_8b','1','1','1','num_output',1)
B_BIASES_32B=branch('biases_32b','1','1','1','num_output',4)
B_BIASES_FLOAT=branch('biases_float','1','1','1','num_output',4)
B_WEIGHTS_CONV_8B=branch('weights_8b','kernel_size','kernel_size','num_input','num_output',1)
B_WEIGHTS_CONV_FLOAT=branch('weights_float','kernel_size','kernel_size','num_input','num_output',4)
B_WEIGHTS_MATMUL_8B=branch('weights_8b','1','1','num_input*input_width*input_height','num_output',1)
B_WEIGHTS_MATMUL_FLOAT=branch('weights_float','1','1','num_input*input_width*input_height','num_output',4)
B_KNOWN_MIN=branch('min_known','1','1','1','1',4)
B_KNOWN_MAX=branch('max_known','1','1','1','1',4)
B_MIN_BIASES_8B=branch('min_biases_8b','1','1','1','1',4)
B_MAX_BIASES_8B=branch('max_biases_8b','1','1','1','1',4)
B_MIN_BIASES_32B=branch('min_biases_32b','1','1','1','1',4)
B_MAX_BIASES_32B=branch('max_biases_32b','1','1','1','1',4)
B_MIN_WEIGHTS_8B=branch('min_weights_8b','1','1','1','1',4)
B_MAX_WEIGHTS_8B=branch('max_weights_8b','1','1','1','1',4)
B_STRIDE_1=branch('stride_1','1','1','1','1',0)
B_STRIDE_2=branch('stride_2','1','2','2','1',0)
B_WINDOW=branch('window','1','2','2','1',0)
B_INPUT=branch('input','1','layer.input_param.shape[0].dim[2]','layer.input_param.shape[0].dim[3]','layer.input_param.shape[0].dim[1]',4)
B_SHAPE=branch('shape','1','2','2','1',4)
B_START_SLICE_1=branch('start_slice_1','1','1','1','4',4)
B_SIZE_SLICE_1=branch('size_slice_1','1','1','1','4',4)  
B_START_SLICE_2=branch('start_slice_2','1','1','1','4',4)
B_SIZE_SLICE_2=branch('size_slice_2','1','1','1','4',4) 

# Describe how to map caffe layers into hexagon nn nodes
# Layer Parameters: succession of nodes
# Node Parameters: (name, hexagon nn operation, padding type, input branches, output type):
dic_layer_to_nodes = {
    'convolution_8b_biases': [
        node('conv2d','QuantizedConv2d_8x8to32','NN_PAD_SAME',[B_ACTIVATION,B_WEIGHTS_CONV_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B,B_STRIDE_1],NODE_FIX32),
        node('shrink2_2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8),
        node('bias_add','QuantizedBiasAdd_8p8to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_8B,B_MAX_BIASES_8B],NODE_FIX32),
        node('shrink2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8) ],
    'convolution_32b_biases': [
        node('conv2d','QuantizedConv2d_8x8to32','NN_PAD_SAME',[B_ACTIVATION,B_WEIGHTS_CONV_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B,B_STRIDE_1],NODE_FIX32),
        node('bias_add','QuantizedBiasAdd_32p32to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_32B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_32B,B_MAX_BIASES_32B],NODE_FIX32),
        node('shrink2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8) ],
    'convolution_float': [
        node('conv2d','Conv2d_f','NN_PAD_SAME',[B_ACTIVATION,B_WEIGHTS_CONV_FLOAT,B_STRIDE_1],NODE_FLOAT),
        node('bias_add','BiasAdd_f','NN_PAD_NA',[B_ACTIVATION,B_BIASES_FLOAT],NODE_FLOAT) ],
    'convolution_8b_biases_known_range': [
        node('conv2d','QuantizedConv2d_8x8to32_32','NN_PAD_SAME',[B_ACTIVATION,B_WEIGHTS_CONV_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B,B_STRIDE_1],NODE_FIX32),
        node('shrink2_2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8),
        node('bias_add','QuantizedBiasAdd_8p8to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_8B,B_MAX_BIASES_8B],NODE_FIX32),
        node('requantize','Requantize_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION,B_KNOWN_MIN,B_KNOWN_MAX],NODE_FIX8) ],
    'convolution_32b_biases_known_range': [
        node('conv2d','QuantizedConv2d_8x8to32','NN_PAD_SAME',[B_ACTIVATION,B_WEIGHTS_CONV_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B,B_STRIDE_1],NODE_FIX32),
        node('bias_add','QuantizedBiasAdd_32p32to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_32B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_32B,B_MAX_BIASES_32B],NODE_FIX32),
        node('requantize','Requantize_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION,B_KNOWN_MIN,B_KNOWN_MAX],NODE_FIX8) ],    
    'inner_product_8b_biases': [
        node('matmul','QuantizedMatMul_8x8to32','NN_PAD_NA',[B_ACTIVATION,B_WEIGHTS_MATMUL_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B],NODE_FIX32),
        node('shrink2_2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8),
        node('bias_add','QuantizedBiasAdd_8p8to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_8B,B_MAX_BIASES_8B],NODE_FIX32),
        node('shrink2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8) ],
    'inner_product_32b_biases': [
        node('matmul','QuantizedMatMul_8x8to32','NN_PAD_NA',[B_ACTIVATION,B_WEIGHTS_MATMUL_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B],NODE_FIX32),
        node('bias_add','QuantizedBiasAdd_32p32to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_32B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_32B,B_MAX_BIASES_32B],NODE_FIX32),
        node('shrink2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8) ],
    'inner_product_float': [
        node('matmul','MatMul_f','NN_PAD_NA',[B_ACTIVATION,B_WEIGHTS_MATMUL_FLOAT],NODE_FLOAT),
        node('bias_add','BiasAdd_f','NN_PAD_NA',[B_ACTIVATION,B_BIASES_FLOAT],NODE_FLOAT) ],
    'inner_product_8b_biases_known_range': [
        node('matmul','QuantizedMatMul_8x8to32','NN_PAD_NA',[B_ACTIVATION,B_WEIGHTS_MATMUL_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B],NODE_FIX32),
        node('shrink2_2','QuantizeDownAndShrinkRange_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8),
        node('bias_add','QuantizedBiasAdd_8p8to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_8B,B_MAX_BIASES_8B],NODE_FIX32),
        node('requantize','Requantize_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION,B_KNOWN_MIN,B_KNOWN_MAX],NODE_FIX8) ],
    'inner_product_32b_biases_known_range': [
        node('matmul','QuantizedMatMul_8x8to32','NN_PAD_NA',[B_ACTIVATION,B_WEIGHTS_MATMUL_8B,B_ACTIVATION,B_ACTIVATION,B_MIN_WEIGHTS_8B,B_MAX_WEIGHTS_8B],NODE_FIX32),
        node('bias_add','QuantizedBiasAdd_32p32to32','NN_PAD_NA',[B_ACTIVATION,B_BIASES_32B,B_ACTIVATION,B_ACTIVATION,B_MIN_BIASES_32B,B_MAX_BIASES_32B],NODE_FIX32),
        node('requantize','Requantize_32to8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION,B_KNOWN_MIN,B_KNOWN_MAX],NODE_FIX8) ],
    'pooling': [
        node('pool','QuantizedMaxPool_8','NN_PAD_SAME',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION,B_WINDOW,B_STRIDE_2],NODE_FIX8)],
    'pooling_float': [
        node('pool','MaxPool_f','NN_PAD_SAME',[B_ACTIVATION,B_WINDOW,B_STRIDE_2],NODE_FLOAT)],
    'input_8b': [
        node('input','INPUT','NN_PAD_NA',[],NODE_CHAR)],
    'input_float_w_quantize': [
        node('input','INPUT','NN_PAD_NA',[],NODE_FLOAT),
        node('autoquant','AutoQuantize','NN_PAD_SAME',[B_ACTIVATION],NODE_FIX8)],
    'input_float': [
        node('input','INPUT','NN_PAD_NA',[],NODE_FLOAT)],
    'relu': [
        node('relu','QuantizedRelu_8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FIX8)],
    'relu_float': [
        node('relu','Relu_f','NN_PAD_NA',[B_ACTIVATION],NODE_FLOAT)],
    'softmax': [
        node('reshape','QuantizedReshape','NN_PAD_NA',[B_ACTIVATION,B_SHAPE,B_ACTIVATION,B_ACTIVATION],NODE_FIX8),
        node('dequantize','Dequantize','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION,B_ACTIVATION],NODE_FLOAT),
        node('softmax','Softmax_f','NN_PAD_NA',[B_ACTIVATION],NODE_FLOAT)],
    'softmax_float': [
        node('reshape','Reshape','NN_PAD_NA',[B_ACTIVATION,B_SHAPE],NODE_FLOAT),
        node('softmax','Softmax_f','NN_PAD_NA',[B_ACTIVATION],NODE_FLOAT)],
    'eltwise_max': [
        node('eltwise_max','QuantizedMaximum_8','NN_PAD_NA',[B_ACTIVATION,B_ACTIVATION_2,B_ACTIVATION,B_ACTIVATION,B_ACTIVATION_2,B_ACTIVATION_2],NODE_FIX8)],
    'slice': [
        node('slice_1','QuantizedSlice_8','NN_PAD_NA',[B_ACTIVATION,B_START_SLICE_1,B_SIZE_SLICE_1,B_ACTIVATION,B_ACTIVATION,],NODE_FIX8,0),
        node('slice_2','QuantizedSlice_8','NN_PAD_NA',[B_ACTIVATION,B_START_SLICE_2,B_SIZE_SLICE_2,B_ACTIVATION,B_ACTIVATION,],NODE_FIX8,1)],
   }

# Describe how to parse parameters from caffe layers
# Layer parser parameters: (num output, kernel size, output_width, output_height)
dic_layer_parsers = {
    'convolution':layer_parser('layer.convolution_param.num_output','layer.convolution_param.kernel_size[0]','int(input_width)','int(input_height)'),
    'inner_product':layer_parser('layer.inner_product_param.num_output',NA,'1','1'),
    'pooling':layer_parser('lp.num_input',NA,'int(input_width/layer.pooling_param.stride)','int(input_height/layer.pooling_param.kernel_size)'),
    'input':layer_parser('layer.input_param.shape[0].dim[1]',NA,'layer.input_param.shape[0].dim[2]','layer.input_param.shape[0].dim[3]'),
    'relu':layer_parser('lp.num_input',NA,'int(input_width)','int(input_height)'),
    'slice':layer_parser('lp.num_input/2',NA,'int(input_width)','int(input_height)'),
    'eltwise_max':layer_parser('lp.num_input',NA,'int(input_width)','int(input_height)'),
    'softmax':layer_parser('lp.num_input',NA,'int(input_width)','int(input_height)'),
}

# End of caffe layer definitions
#------------------------------------------------------------------------------------------

# Internal variables
node_id_value = 0x00010000
node_ids=[]
mem_filler = "FOO"
imp_file=""
data_file=""
data_file_32b=""
data_file_float=""
header_file=""
is_model_quantized=False

# Default config values (may be overridden in config file or as a command line parameter)
check_init=False
generate_binary_model=False
leave_coefficients_unchanged=False
debug_level=1
perf_dump=False
generate_float_imp=False
use_runtime_requantization=False
use_8b_biases=False
use_float_input=False
min_input=None
max_input=None
dic_external_extrema = {}
dic_external_coefficients = {}

###########################################################################################
# Function definitions

def parse_args():
    parser = argparse.ArgumentParser(description='Converts a float or quantized caffe model into an hexagon nn implementation')
    parser.add_argument('proto', help="Path to prototxt")
    parser.add_argument('model', help="Path to caffemodel")
    parser.add_argument('root', help="Root name for generated implementation")
    parser.add_argument('--config', 
                        help='Config file controlling hexagon nn implementation (overridden by command-line options)', action='store', type=str)
    parser.add_argument('--debug_level', 
                        help='Set debug level to be used by hexagon nn', action='store', type=int, default=1)
    parser.add_argument('--perf_dump', 
                        help='Generates output suitable for use with perf_dump', action='store_true', default=False)
    parser.add_argument('--generate_float_imp', 
                        help='Set to generate a float-based Hexagon NN implementation for reference/debugging purposes', action='store_true', default=False)
    parser.add_argument('--use_runtime_requantization', 
                        help='Set to use automatic runtime quantization instead of fixed quantization based on known activation range', action='store_true', default=False)
    parser.add_argument('--check_init', 
                        help='Set to run checks when initializing the graph', action='store_true', default=False)
    parser.add_argument('--leave_coefficients_unchanged', 
                        help='Set to not update data file (saves compilation time)', action='store_true', default=False )
    parser.add_argument('--use_8b_biases', 
                        help='Set to store biases as 8-bit values', action='store_true', default=False)
    parser.add_argument('--use_float_input', 
                        help='Set to generate graph that takes input as floating point (default is uint8)', action='store_true', default=False)
    parser.add_argument('--min_input', 
                        help='Minimum float input. Required if using uint8 input', action='store', type=float)
    parser.add_argument('--max_input', 
                        help='Maximum float input. Required if using uint8 input', action='store', type=float)
    parser.add_argument('--generate_binary_model', 
                        help='Set to generate an implementation to be turned into a canned model', action='store_true', default=False)
    args = parser.parse_args()

    return args

def external_coefficients(name):
    if name in dic_external_coefficients:
        return dic_external_coefficients[name]
    else:
        return name

def external_extrema(name,value):
    if name in dic_external_extrema:
        return dic_external_extrema[name]
    else:
        return value

def base_type(type):
    if is_convolution(type):
        return "convolution"
    elif is_inner_product(type):
        return "inner_product"
    elif is_pooling(type):
        return "pooling"
    elif is_input(type):
        return "input"
    elif is_relu(type):
        return "relu"
    elif is_softmax(type):
        return "softmax"
    elif is_eltwise_max(type):
        return "eltwise_max"
    elif is_slice(type):
        return "slice"
    return ""

def specific_type(type):
    base_type_name = base_type(type) 

    if generate_float_imp:
        return base_type_name + "_float"
    else:
        if is_inner_product(type) or is_convolution(type):
            base_type_name += "_8b_biases" if use_8b_biases else "_32b_biases"
            base_type_name += "" if use_runtime_requantization else "_known_range"
        elif is_input(type):
            base_type_name += "_float_w_quantize" if use_float_input else "_8b"
    return base_type_name

def is_convolution(type):    
    return type=="Convolution" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["CONVOLUTION"].number

def is_inner_product(type):
    return type=="InnerProduct" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["INNER_PRODUCT"].number

def is_pooling(type):
    return type=="Pooling" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["POOLING"].number

def is_input(type):
    return type=="Input" 

def is_relu(type):
    return type=="ReLU" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["RELU"].number

def is_softmax(type):
    return type=="Softmax" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["SOFTMAX"].number

def is_eltwise_max(type):    
    return type=="Eltwise" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["ELTWISE"].number

def is_slice(type):    
    return type=="Slice" or type==cq.V1LayerParameter.LayerType.DESCRIPTOR.values_by_name["SLICE"].number

# Change coefficient storage format from caffe to TensorFlow
# w=width, h=height, i=#input channels, o=#output channels
#
# Leftest dimension corresponds to outermost loop
# E.g. contiguous elements in hwio format correspond to elements
# with same h,w,i and contiguous output channels 
def oihw_to_hwio(input,hw,i,o):
    return np.transpose(np.array(input).reshape(o,i,hw),(2,1,0)).reshape(o*i*hw)

def quant_u8(value):
    value=int(value+0.5)
    if value>=255:
        return 255
    if value<0:
        return 0
    return value

def quant_s32(value):
    m=(1<<31)
    value=int(round(value))
    if value>=m-1:
        return m-1
    if value<-m:
        return -m
    return value

def declare_node_in(name,contents):
    imp_file.write("static hexagon_nn_input " + inputs(name) + "[]={\n"+contents+"};\n")

def declare_node_out(name,w,h,d,type):
    if type == NODE_FIX32 or type == NODE_FIX8:
        contents = nodes_with_3d_output(w,h,d,type.size)
    else:
        contents = nodes_with_1d_output(w,h,d,type.size)

    imp_file.write("static hexagon_nn_output " + outputs(name) + "[]={\n"+contents+"};\n")

def node_in(source,index):
    source_id=node_id(source)
    return "  {.src_id=" + source_id + ", .output_idx=" + str(index) + "},\n"

def node_out(width,height,depth,element_size):
    rank=4
    zero_offset=mem_filler
    step_size="0.0"
    max_sizes="{1,"+str(width)+","+str(height)+","+str(depth)+","+\
        mem_filler+","+mem_filler+","+mem_filler+","+mem_filler+"}"
    return "  {.rank="+str(rank)+", .max_sizes="+max_sizes+", .elementsize="+\
        str(element_size)+", .zero_offset="+zero_offset+", .stepsize=" + step_size+"},\n"

def nodes_with_3d_input_and_coeff(input,coeff):
    value = node_in(input,0)
    value+= node_in(coeff,0)
    value+= node_in(input,1)
    value+= node_in(input,2)
    value+= node_in(mini(coeff),0)
    value+= node_in(maxi(coeff),0)
    return value

def nodes_with_3d_input_and_const(input,const):
    value = node_in(input,0)
    value+= node_in(const,0)
    value+= node_in(input,1)
    value+= node_in(input,2)
    return value

def nodes_with_3d_input(input):
    value = node_in(input,0)
    value+= node_in(input,1)
    value+= node_in(input,2)
    return value

def nodes_with_3d_output(width,height,depth,element_size):
    value = node_out(width,height,depth,element_size)
    value+= node_out(1,1,1,4)
    value+= node_out(1,1,1,4)
    return value

def nodes_with_1d_output(width,height,depth,element_size):
    return node_out(width,height,depth,element_size)

def declare_array(file,name,length,element_size):
    declaration="uint8_t " + name + "[" + str(length * element_size) + "]"
    header_file.write("extern " + declaration + ";\n")
    file.write(declaration + "__attribute__((aligned(128))) = {")

# Converts a float to binary
def float_to_binary(num):
    return str(''.join(bin(ord(c)).replace('0b', '').rjust(8, '0') for c in struct.pack('!f', num)))

# Converts an int to binary
def int_to_binary(num):
    if num >= 0:
        return '{0:032b}'.format(num)
    else:
        return '{0:032b}'.format((2**32) + num)

# Converts a list of word-sized values to a binary equivalent list of byte-sized values
def to_uint8_array(word_input, type):
    output = [None] * len(word_input) * 4
    i = 0
    while i < len(word_input):
        binary_num = int_to_binary(word_input[i]) if type == 'int32' else float_to_binary(word_input[i])
        start_index = i*4
        output[start_index] = int(binary_num[24:32],2)
        output[start_index+1] = int(binary_num[16:24],2)
        output[start_index+2] = int(binary_num[8:16],2)
        output[start_index+3] = int(binary_num[0:8],2)
        i+=1
    return output

# Stores unsigned char values as string constants of <=120 bytes each
# Allows for much faster compilation times on some systems
def store_octal_array(file,nums):
    i = 0
    nums_length = len(nums)
    while i < nums_length:
        j = 0
        file.write("\"")
        while j < 120 and i < nums_length:
            file.write("\\%o" % nums[i])
            i+=1
            j+=1
        file.write("\"\n")

def declare_array_of_four(type,name,x1,x2,x3,x4):
    imp_file.write(type + " " + name + "[] = {%d"%x1 +", %d"%x2 +", %d"%x3 +", %d"%x4 +"};\n")

def declare_array_of_one(type,name,value):
    comment = "  // changed from " + str(value) if name in dic_external_extrema else ""
    if external_extrema(name,value)==0.0:
        imp_file.write(type + " " + name + "[] = {" + str(external_extrema(name,value)) + "};" + comment + "\n")
    else:
        imp_file.write(type + " " + name + "[] = {" + "%.12g"%float(external_extrema(name,value)) + "};" + comment + "\n")

def declare_min_max_arrays(name,min,max):
    declare_array_of_one("float",mini(name),min)
    declare_array_of_one("float",maxi(name),max)

# Return node id associated to given name. If node id not already defined, define it.
def node_id(name):
    global node_ids
    global node_id_value
    node_id = name+"_NID"
    if node_id not in node_ids:
        value =  "0x" + format(node_id_value, '08X')
        node_id_value+=1
        header_file.write("#define " + node_id + " " + value + "\n")
        node_ids.append(node_id)
    return node_id

def inputs(name):
    return "inputs_"+name

def outputs(name):
    return "outputs_"+name

def input(name):
    return "input_"+name

def output(name):
    return "output_"+name

def autoquant(name):
    return "autoquant_"+name

def shape(name):
    return "shape_"+name

def quantize(name):
    return "quantize_"+name

def print_32(name):
    return "print_32_"+name

def weights(name):
    if generate_float_imp:
        return "weights_float_"+name
    return  "weights_8b_"+name

def biases(name):
    if generate_float_imp:
        return "biases_float_"+name
    elif use_8b_biases:
        return "biases_8b_"+name
    return "biases_32b_"+name

def mini(name):
    return "min_"+name

def maxi(name):
    return "max_"+name

def known(name):
    return "known_"+name

def printf(before,txt):
    imp_file.write(before+"printf(\""+txt+"\\n\");\n")

def append_const_node_with_name_and_id(name_id,node_name,b,w,h,d,element_size):
    if element_size==0:
        node_name="NULL"
    if check_init==1:
        printf("  ","Gently apply const node "+node_name+" with b=" + str(b) + ", w="+str(w)+", h="+str(h)+", d="+str(d)+", size=" + str(element_size))
        imp_file.write("  if(")
    imp_file.write("  hexagon_nn_append_empty_const_node_large_array(nng_id,"+name_id+","+str(b)+","+str(w)+","+str(h)+","+str(d)+\
        ",(const uint8_t *)"+node_name+","+str(element_size*b*w*h*d)+")")
    if check_init==1:
        printf(" ) ","Error appending const node "+node_name)
    else:
        imp_file.write(";\n")

def append_const_node_with_external_coefficients(node_name,b,w,h,d,element_size):
    append_const_node_with_name_and_id(node_id(node_name),external_coefficients(node_name),b,w,h,d,element_size)

def append_const_node(node_name,b,w,h,d,element_size):
    append_const_node_with_name_and_id(node_id(node_name),node_name,b,w,h,d,element_size)

def append_min_max_const_nodes(node_name):
    append_const_node(mini(node_name), 1, 1, 1, 1, 4)
    append_const_node(maxi(node_name), 1, 1, 1, 1, 4)

def append_node(nn_op,pad,in_size,out_size,output_name):
    input_tensor=inputs(output_name) if in_size>0 else "NULL"
    output_tensor=outputs(output_name) if out_size>0 else "NULL"
    if perf_dump:
        imp_file.write("  info_for_debug(" + node_id(output_name) + ",\"" + output_name + "\",\"OP_" + nn_op + "\");\n")
    if check_init==1:
        printf("  ","Apply node "+output_name+" with op=" + nn_op + ", pad="+pad+", in_size="+str(in_size)+", out_size="+str(out_size))
        imp_file.write("  if(")
    imp_file.write("  hexagon_nn_append_node(nng_id,"+node_id(output_name)+",OP_"+nn_op+","+pad+","+\
                input_tensor+","+str(in_size)+","+output_tensor+","+str(out_size)+")")
    if check_init==1:
        printf(" ) ","Error appending node "+output_name)
    else:
        imp_file.write(";\n")

def run(proto, model, root):

    global imp_file
    global data_file
    global data_file_32b
    global data_file_float
    global header_file
    global is_model_quantized
    global use_runtime_requantization

    ###########################################################################################
    # Parse caffe prototxt and model files
    f = open(proto, 'r')
    proto_contents = f.read()
    if 'quantization_param' in proto_contents:
        is_model_quantized=True
    else:
        use_runtime_requantization=True

    print ("*********************************************************************")
    print ("Script configurations")
    print ("  check_init:%d"%check_init)
    print ("  generate_binary_model:%d"%generate_binary_model)
    print ("  leave_coefficients_unchanged:%d"%leave_coefficients_unchanged)
    if generate_binary_model!=1:
        print ("  debug_level:%d"%debug_level)
    print ("  perf_dump:%d"%perf_dump)
    print ("  generate_float_imp:%d"%generate_float_imp)
    if is_model_quantized:
        print ("  is_model_quantized:%d"%is_model_quantized)
        print ("  use_runtime_requantization:%d"%use_runtime_requantization)
    print ("  use_8b_biases:%d"%use_8b_biases)
    print ("  use_float_input:%d"%use_float_input)
    if not use_float_input and not generate_float_imp:
        print ("  min_input:%f"%min_input)
        print ("  max_input:%f"%max_input)
    print ("  prototxt file:%s" %args.proto)
    print ("  model file:%s"%args.model)
    print ("  root name for generated implementation:%s"%args.root)
    print ("  number of coefficients manually bypassed: %d"%len(dic_external_coefficients ))
    print ("  number of extrema manually bypassed: %d"%len(dic_external_extrema))
    print ("*********************************************************************")

    net_txt = cq.NetParameter()
    text_format.Parse(proto_contents, net_txt)
    f.close()

    f = open(model, 'rb')
    net = cq.NetParameter()
    net.ParseFromString(f.read())
    f.close()

    ###########################################################################################
    # Open files to be generated and add includes and generic macros
    data_file_name = root + "_data.c"
    data_file_name_32b = root + "_data_32b.c"
    data_file_name_float = root + "_data_float.c"
    imp_file_name = root + ".c"
    header_file_name = root + ".h"

    if generate_binary_model:
        imp_all_file_name = root + "_all.c"
        imp_all_file = open(imp_all_file_name,"w") 
        imp_all_file.write("#include \"" + data_file_name +"\"\n")
        imp_all_file.write("#include \"" + data_file_name_32b +"\"\n")
        imp_all_file.write("#include \"" + data_file_name_float +"\"\n")
        imp_all_file.write("#include \"" + imp_file_name +"\"\n")

    if leave_coefficients_unchanged:
        data_file_name+=".IGNORED"
        data_file_name_32b+=".IGNORED"
        data_file_name_float+=".IGNORED"
    else:
        if use_8b_biases and not generate_float_imp:
            data_file_name_32b+=".IGNORED"
        elif generate_float_imp:
            data_file_name_32b+=".IGNORED"
            data_file_name+=".IGNORED"

    data_file = open(data_file_name,"w")
    data_file_32b = open(data_file_name_32b,"w")
    data_file_float = open(data_file_name_float, "w") 
    imp_file = open(imp_file_name,"w") 
    header_file = open(header_file_name,"w") 


    imp_file.write("#include \"" + header_file_name +"\"\n\n")

    data_file.write("#include \"stdint.h\"\n")
    data_file.write("#include \"" + header_file_name +"\"\n\n")

    data_file_32b.write("#include \"stdint.h\"\n")
    data_file_32b.write("#include \"" + header_file_name +"\"\n\n")

    data_file_float.write("#include \"stdint.h\"\n")
    data_file_float.write("#include \"" + header_file_name +"\"\n\n")

    header_file.write("#include \"hexagon_nn_large_array.h\"\n")
    header_file.write("#include \"hexagon_nn_ops.h\"\n")
    header_file.write("#include \"stdint.h\"\n")

    header_file.write("\n#define " + mem_filler + " 0x1F1F1F1F\n")
    if generate_binary_model!=1:
        header_file.write("#define DEBUG_LEVEL " + str(debug_level) + "\n")

    if perf_dump:
        header_file.write("\n//Required for perfdump output\n")
        header_file.write("void info_for_debug(unsigned int id, const char *name, const char *opname);\n")

    for key, value in dic_external_coefficients.items():
        header_file.write("\nextern uint8_t " + value + "[];  // replaces " + key + "\n")

    ###########################################################################################
    # Parse graph parameters specified in prototxt file

    data_file.write("\n// Define weight and bias arrays\n\n")
    data_file_float.write("\n// Define weight and bias arrays\n\n")
    data_file_32b.write("\n// Define bias arrays\n\n")
    header_file.write("\n// Declare weight and bias arrays\n") 

    # Depending on the situation, it is useful to have a direct path from either layer name or layer top
    # branch to layer parameters.  Note however that the mapping from top branch to layer param relies on
    # the layer param top and bottom values and NOT the top and bottom values provided in the prototxt file.
    # This is because prototxt file may have multiple layers use the same top branch names (e.g. a ReLU
    # following a convolution layer).
    dic_name_to_layer_params={}
    dic_top_to_layer_params={}
    dic_identical_layer_name={}
    layer_qp=net_txt.layer[0].quantization_param if is_model_quantized else 0
    
    txt_layers = net_txt.layer if len(net_txt.layer)>0 else net_txt.layers
    if len(txt_layers)==0:
        print ("Layers are not specified where we can find them in " + proto)

    # Input parameters are either specified outside all layers or in an input layer
    if len(net_txt.input_dim):
        lp = layer_params()
        lp.num_output=net_txt.input_dim[1]
        lp.output_width=net_txt.input_dim[2] 
        lp.output_height=net_txt.input_dim[3]
        dic_top_to_layer_params[net_txt.input[0]]=lp
    for layer in txt_layers:
        type = specific_type(layer.type)
        if type not in dic_layer_to_nodes:
            continue
        layer_parser = dic_layer_parsers[base_type(layer.type)]
        lp = layer_params()

        for bottom in layer.bottom:
            bottom = str(bottom)
            if bottom in dic_identical_layer_name:
                if dic_identical_layer_name[bottom]>1:
                    bottom += "_v"+str(dic_identical_layer_name[bottom]-1)
            lp.bottom.append(bottom)
            
        if len(layer.bottom)!=0:
            lp_previous=dic_top_to_layer_params[bottom]
            lp.num_input=lp_previous.num_output
            input_width=lp_previous.output_width
            input_height=lp_previous.output_height
        else:
            lp.num_input = 0
        lp.num_output = eval(layer_parser.num_output)         
        lp.output_width = eval(layer_parser.output_width)
        lp.output_height = eval(layer_parser.output_height)

        if is_model_quantized:
            layer_qp = layer.quantization_param
            lp.sw = layer_qp.sw
            lp.sy = layer_qp.sy
            lp.sx = 1 if len(layer.bottom)==0 else dic_top_to_layer_params[lp.bottom[0]].sy
        lp.kernel_size = eval(layer_parser.kernel_size) if layer_parser.kernel_size != NA else 1

        for top in layer.top:
            top = str(top)
            if top in dic_identical_layer_name:
                dic_identical_layer_name[top]+=1
                top += "_v"+str(dic_identical_layer_name[top]-1)
            else:
                dic_identical_layer_name[top]=1
            lp.top.append(top)
            dic_top_to_layer_params[top]=lp

        dic_name_to_layer_params[layer.name]=lp

    dic_coefficient_sizes = {}

    data_layers = net.layer if len(net.layer)>0 else net.layers
    if len(data_layers)==0:
        print ("Layers are not specified where we can find them in " + model)

    ###########################################################################################
    # Parse coefficients from quantized model

    for layer in data_layers:
        if not is_model_quantized:
            break
        if layer.name not in dic_name_to_layer_params:
            print("WARNING: layer " + str(layer.name) + " not present in prototxt file")
            continue
        if len(layer.blobs) > 0:
            layer_parser = dic_layer_parsers[base_type(layer.type)]
            array_name = weights(layer.name)
            uw=[ord(x) for x in layer.blobs[0].byte_data]
            b=layer.blobs[1].word_data
            lp = dic_name_to_layer_params[layer.name]
            #lp = dic_top_to_layer_params[layer.top[0]]

            w = [x-256 if x > 127 else x for x in uw]
            
            # Since model has already been quantized, there is no point in leveraging the asymmetric
            # fixed-point data support from nn_graph since that would lead to larger errors in the
            # process of re-quantizing the coefficients differently.  We scale down max slightly to
            # guarantee that zero_points falls on an int precisely)
            lp.max_w = 127/lp.sw
            lp.min_w = -128/lp.sw

            lp.max_b = 127/(lp.sw*lp.sx)
            lp.min_b = -128/(lp.sw*lp.sx)
            
            if generate_float_imp:
                declare_array(data_file_float,array_name,len(w),4)
            else:
                declare_array(data_file,array_name,len(w),1)

            dic_coefficient_sizes[array_name] = len(w)

            if leave_coefficients_unchanged:
                w=w[0:8]

            if generate_float_imp:
                w = [float(x) / lp.sw for x in w] 
            else:
                w = [quant_u8(x + 128) for x in w]
                
            # Reorder caffe data for hexagon nn
            if not leave_coefficients_unchanged:
                w=oihw_to_hwio(w,len(w)/(lp.num_input*lp.num_output),lp.num_input,lp.num_output)
                
            if not generate_float_imp:
                store_octal_array(data_file, w)
                data_file.write("};\n")
            else:
                store_octal_array(data_file_float, to_uint8_array(w, 'float'))
                data_file_float.write("};\n")

            array_name = biases(layer.name)
            dic_coefficient_sizes[array_name] = len(b)

            max_abs_b = max(np.absolute(b))

            if leave_coefficients_unchanged:
                b=b[0:8]

            if generate_float_imp:
                declare_array(data_file_float,array_name,dic_coefficient_sizes[array_name],4)
                if max_abs_b==0:
                    lp.max_b=127
                    lp.min_b=-128
                b = [float(x) / (lp.sw * lp.sx) for x in b]
                store_octal_array(data_file_float, to_uint8_array(b, 'float'))
                data_file_float.write("};\n")
            else:
                if not use_8b_biases:
                    declare_array(data_file_32b,array_name,dic_coefficient_sizes[array_name],4)
                    if max_abs_b==0:
                        lp.max_b=127   # Note: Keeping an add_bias layer with no coefficients allows supernodes to be triggered more easily 
                        lp.min_b=-128
                        b=[0]*len(b)
                    else:
                        log2_max_b = int(math.log(max_abs_b-.5,2))
                        shift_amount = log2_max_b - 6 if log2_max_b>6 else 0
                        b = [quant_s32(x << (32-8-shift_amount)) for x in b]
                        if shift_amount>0:
                            lp.max_b *= (1<<shift_amount) 
                            lp.min_b *= (1<<shift_amount)
                    store_octal_array(data_file_32b,to_uint8_array(b, 'int32'))
                    data_file_32b.write("};\n")
                else:
                    if max_abs_b>128:  # in case biases do not fit readily into bytes
                        log2_max_b = int(math.log(max_abs_b-.5,2))
                        shift_amount = log2_max_b - 6
                        b = [x>>shift_amount for x in b]
                        lp.max_b *= (1<<shift_amount) 
                        lp.min_b *= (1<<shift_amount) 
                    declare_array(data_file,array_name,dic_coefficient_sizes[array_name],1)
                    if max_abs_b==0:
                        lp.max_b=127   # Note: Keeping an add_bias layer with no coefficients allows supernodes to be triggered more easily
                        lp.min_b=-128
                        b=[128]*len(b)
                    else:
                        b = [quant_u8(x +128) for x in b]
                    store_octal_array(data_file,b)
                    data_file.write("};\n")

    ###########################################################################################
    # Parse coefficients from caffe float model

    for layer in data_layers:
        if is_model_quantized:
            break
        if len(layer.blobs)>0:
            # Initialize values for the current layer
            lp = dic_name_to_layer_params[layer.name]
            #lp = dic_top_to_layer_params[layer.top[0]]
            weights_name = weights(layer.name)
            biases_name = biases(layer.name)
            w = layer.blobs[0].data
            b = layer.blobs[1].data
            dic_coefficient_sizes[weights_name] = len(w)
            dic_coefficient_sizes[biases_name] = len(b)

            # Generating quantized implementation from caffe float model
            if not generate_float_imp:
                # Set max and min for symmetric coefficients
                lp.max_w = max(np.absolute(w))
                lp.min_w = -lp.max_w * 128 / 127

                lp.max_b = max(np.absolute(b))
                lp.min_b = -lp.max_b * 128 / 127
                
                domain_w = lp.max_w - lp.min_w
                domain_b = lp.max_b - lp.min_b

                # Check that 0.0 maps to 0
                max_loss_on_zero=.00001

                zero_point = quant_u8(-lp.min_w / domain_w * 255)
                quantization_error = abs(128-zero_point)
                if quantization_error>max_loss_on_zero:
                    print ("WARNING: Quantization error " + str(quantization_error) + " seems large for encoding zero, which maps to " + str(zero_point))

                zero_point = quant_u8(-lp.min_b / domain_b * 255)
                quantization_error = abs(128-zero_point)
                if quantization_error>max_loss_on_zero:
                    print ("WARNING: Quantization error " + str(quantization_error) + " seems large for encoding zero, which maps to " + str(zero_point))

                # Declare and quantize arrays
                declare_array(data_file,weights_name,len(w),1)

                if leave_coefficients_unchanged:
                    w=w[0:8]
                    
                w = [quant_u8((x - lp.min_w) / domain_w * 255) for x in w]

                if not leave_coefficients_unchanged:
                    # Reorder caffe data for hexagon nn
                    w=oihw_to_hwio(w,int(len(w)/(lp.num_input*lp.num_output)),lp.num_input,lp.num_output)

                store_octal_array(data_file,w)
                data_file.write("};\n")

                if leave_coefficients_unchanged:
                    # Set coefficients to first 8-elements to be written to dummy file
                    b=b[0:8]

                if not use_8b_biases:
                    # 32-bit symmetric coefficients will already have the zero point close enough to zero
                    lp.min_b = -lp.max_b
                    declare_array(data_file_32b,biases_name,dic_coefficient_sizes[biases_name],4)
                    s_max = 1 << 32
                    s_offset = 1 << 31
                    b = [quant_s32((x - lp.min_b) / domain_b * s_max - s_offset) for x in b]
                    b_char = to_uint8_array(b, 'int32')
                    store_octal_array(data_file_32b,b_char)
                    data_file_32b.write("};\n")
                else:
                    declare_array(data_file,biases_name,dic_coefficient_sizes[biases_name],1)
                    b = [quant_u8((x - lp.min_b) / domain_b * 255) for x in b]
                    store_octal_array(data_file,b)
                    data_file.write("};\n")

            # Generating float implementation from caffe float model
            else:
                # Skip generating new coefficients
                if leave_coefficients_unchanged:
                    w=w[0:8]
                    b=b[0:8]
                else:
                    # Reorder caffe data for hexagon nn
                    w=oihw_to_hwio(w,int(len(w)/(lp.num_input*lp.num_output)),lp.num_input,lp.num_output)

                # Save weights in floating point form
                declare_array(data_file_float,weights_name,dic_coefficient_sizes[weights_name],4)
                store_octal_array(data_file_float,to_uint8_array(w, 'float'))
                data_file_float.write("};\n")

                # Save biases in floating point form
                declare_array(data_file_float,biases_name,dic_coefficient_sizes[biases_name],4)
                store_octal_array(data_file_float,to_uint8_array(b, 'float'))
                data_file_float.write("};\n")

    ###########################################################################################
    # Define weights and biases arrays for each layer

    imp_file.write("// Define weight and bias mini/maxi arrays\n")

    # Some caffe models have the input parameters specified outside of an input layer
    if len(net_txt.input_dim):
        layer_name=net_txt.input[0]
        output_width=net_txt.input_dim[2]
        output_height=net_txt.input_dim[3]
        if not use_float_input and not generate_float_imp:
            declare_min_max_arrays(layer_name, min_input, max_input)

    unconnected_tops=set()  # use to track tops that aren't used and therefore assumed output to the graph

    for layer in txt_layers:

        for bottom in layer.bottom:
            unconnected_tops.discard(bottom)
        for top in layer.top:
            unconnected_tops.add(top)

        type = specific_type(layer.type)
        num_input = 0 if len(layer.bottom)==0 else dic_top_to_layer_params[layer.bottom[0]].num_output

        if type not in dic_layer_to_nodes:
            continue

        layer_parser = dic_layer_parsers[base_type(layer.type)]

        lp = dic_name_to_layer_params[layer.name]
        #lp = dic_top_to_layer_params[layer.top[0]]

        # process different types of layers
        if (is_convolution(layer.type) or is_inner_product(layer.type)) and not generate_float_imp:
            declare_min_max_arrays(weights(layer.name), lp.min_w, lp.max_w)
            declare_min_max_arrays(biases(layer.name), lp.min_b, lp.max_b)
            
        if is_slice(layer.type):
            # turn this into a loop again if/when we are sure this is the right approach
            #for node in dic_layer_to_nodes["slice"]:
            #    for branch in node.inputs:
            #        if branch != B_ACTIVATION and branch != B_ACTIVATION_2:
            #            declare_array_of_four("int",branch.name+"_"+layer.name,1,2,3,4)
            declare_array_of_four("int","start_slice_1_"+layer.name,0,0,0,0)
            declare_array_of_four("int","size_slice_1_"+layer.name,-1,-1,-1,num_input/2)
            declare_array_of_four("int","start_slice_2_"+layer.name,0,0,0,num_input/2)
            declare_array_of_four("int","size_slice_2_"+layer.name,-1,-1,-1,num_input/2)

        # process various options that affect layers in graph
        if is_model_quantized and not use_runtime_requantization:
            if is_input(layer.type) and not use_float_input and not generate_float_imp:
                if (layer.transform_param.scale!=1):
                    declare_min_max_arrays(layer.top[0], min_input * layer.transform_param.scale, max_input * layer.transform_param.scale)
                else:
                    declare_min_max_arrays(layer.top[0], min_input / lp.sy, max_input / lp.sy)                
            if (is_convolution(layer.type) or is_inner_product(layer.type)) and not generate_float_imp:
                declare_min_max_arrays(known(layer.name), -128 / lp.sy, 127 / lp.sy)
        else:
            if is_input(layer.type) and not use_float_input and not generate_float_imp:
                declare_min_max_arrays(layer.top[0], min_input * layer.transform_param.scale, max_input * layer.transform_param.scale)
                
        if is_softmax(layer.type):
            declare_array_of_one("int32_t",shape(layer.name),num_input)

    ###########################################################################################
    # Declare input/output branches for each node

    header_file.write("\n// Define node ids\n");
    imp_file.write("\n// Define tensors\n")
    current_node_name="NO_NAME"

    dic_top_to_last_node_name={} # returns node that last produced top

    # In case input parameters specified at the top instead of using an input layer
    if len(net_txt.input_dim):
        layer_name=net_txt.input[0]
        num_input=dic_top_to_layer_params[layer_name].num_output
        output_width=dic_top_to_layer_params[layer_name].output_width
        output_height=dic_top_to_layer_params[layer_name].output_height
        if not generate_float_imp:
            if use_float_input:
                declare_node_out(input(layer_name),output_width,output_height,num_input,NODE_FLOAT)
                current_node_name=autoquant(layer_name)
                declare_node_in(current_node_name,node_in(input(layer_name),0))
                declare_node_out(current_node_name,output_width,output_height,num_input,NODE_FIX8)
            else:
                current_node_name=input(layer_name)
                declare_node_out(current_node_name,output_width,output_height,num_input,NODE_CHAR)
        else:
            current_node_name=input(layer_name)
            declare_node_out(current_node_name,output_width,output_height,num_input,NODE_FLOAT)
        dic_top_to_last_node_name[layer_name]="input"

    for layer in txt_layers:

        type = specific_type(layer.type)
        if type not in dic_layer_to_nodes:
            continue
        lp = dic_name_to_layer_params[layer.name]
        num_output = lp.num_output 
        output_width=lp.output_width
        output_height=lp.output_height
        nodes = dic_layer_to_nodes[type]
        
        
        layer_parser = dic_layer_parsers[base_type(layer.type)]
        
        node_idx = 0
        for node in nodes:
        
            branch_names=""
                        
            # Holds the next index to use for each input branch to a given node. Allows to track
            # number of repeats of each branch to a given node.
            next_branch_idx = {}
            
            count = 0
            for branch in node.inputs:
                branch_name = branch.name

                if branch_name in next_branch_idx:
                    branch_idx = next_branch_idx[branch_name]
                    next_branch_idx[branch_name]=next_branch_idx[branch_name]+1 if next_branch_idx[branch_name]<=1 else 0
                else:
                    branch_idx = 0
                    next_branch_idx[branch_name]=1
                    
                if branch == B_ACTIVATION or branch == B_ACTIVATION_2:
                    if node_idx==0:  # If first node in a given layer
                        if not use_float_input and lp.bottom[branch.size] == "data":
                            if branch_idx == 1:
                                branch_idx=0
                                name=mini(lp.bottom[branch.size])
                            elif branch_idx == 2:
                                branch_idx=0
                                name=maxi(lp.bottom[branch.size])
                            else:
                                name=dic_top_to_last_node_name[lp.bottom[branch.size]]+"_"+lp.bottom[branch.size]
                        else:
                            name=dic_top_to_last_node_name[lp.bottom[branch.size]]+"_"+lp.bottom[branch.size]
                    else: # Any other node in a given layer
                        name=(dic_top_to_last_node_name[lp.top[node.output_index]]+"_"+lp.top[node.output_index]) if not is_slice(layer.type) else name
                        
                    branch_names+=node_in(name,branch_idx)
                else:
                    branch_names+=node_in(branch_name + "_" + layer.name,branch_idx)
            
            current_node_name = node.name + "_" + lp.top[node.output_index]

            if len(node.inputs) > 0:
                declare_node_in(current_node_name,branch_names)
                
            declare_node_out(current_node_name,output_width,output_height,num_output,node.output_type)
            node_idx+=1
            dic_top_to_last_node_name[lp.top[node.output_index]]=node.name

    ###########################################################################################
    # Add final output tensors
    
    num_output_nodes=0
    output_array=""

    for layer in txt_layers:
        for top in layer.top:
            if top in unconnected_tops:
                type = specific_type(layer.type)
                if type not in dic_layer_to_nodes:
                    print ("Unable to add output node to unconnected top " + top)
                    continue
                nodes = dic_layer_to_nodes[type]
                current_node_name = nodes[len(nodes)-1].name + "_" + top
                if generate_binary_model==1:
                    num_output_nodes+=1
                    output_array+=node_in(current_node_name,0)
                else:
                    declare_node_in(output(top),node_in(current_node_name,0))
                    
    if generate_binary_model==1:
        output_array_name = "output_array"
        declare_node_in(output_array_name,output_array)                

    ###########################################################################################
    # Begin graph init function

    imp_file.write("\nvoid init_graph(int nng_id) {\n");
    if generate_binary_model!=1:
        imp_file.write("\n  hexagon_nn_set_debug_level(nng_id, DEBUG_LEVEL);\n\n");

    imp_file.write("  printf(\"*********************************************************************\\n\");\n")
    imp_file.write("  printf(\"Script configurations\\n\");\n")
    imp_file.write("  printf(\"  check_init:" + str(int(check_init)) + "\\n\");\n")
    if generate_binary_model!=1:
        imp_file.write("  printf(\"  debug_level:" + str(int(debug_level)) + "\\n\");\n")
    imp_file.write("  printf(\"  perf_dump:" + str(int(perf_dump)) + "\\n\");\n")
    if is_model_quantized:
        imp_file.write("  printf(\"  is_model_quantized:" + str(int(is_model_quantized)) + "\\n\");\n")
        imp_file.write("  printf(\"  use_runtime_requantization:" + str(int(use_runtime_requantization)) + "\\n\");\n")
    imp_file.write("  printf(\"  generate_float_imp:" + str(int(generate_float_imp)) + "\\n\");\n")
    imp_file.write("  printf(\"  use_8b_biases:" + str(int(use_8b_biases)) + "\\n\");\n")
    imp_file.write("  printf(\"  use_float_input:" + str(int(use_float_input)) + "\\n\");\n")
    if not use_float_input and not generate_float_imp:
        imp_file.write("  printf(\"  min_input:" + str(min_input) + "\\n\");\n")
        imp_file.write("  printf(\"  max_input:" + str(max_input) + "\\n\");\n")
    imp_file.write("  printf(\"  prototxt file:" + str(proto) + "\\n\");\n")
    imp_file.write("  printf(\"  model file:" + str(model) + "\\n\");\n")
    imp_file.write("  printf(\"  root name for generated implementation:" + str(root) + "\\n\");\n")
    imp_file.write("  printf(\"  number of coefficients manually bypassed: " + str(len(dic_external_coefficients )) + "\\n\");\n")
    imp_file.write("  printf(\"  number of extrema manually bypassed: " + str(len(dic_external_extrema)) + "\\n\");\n")
    imp_file.write("  printf(\"  generate_binary_model:" + str(int(generate_binary_model)) + "\\n\");\n")
    imp_file.write("  printf(\"*********************************************************************\\n\");\n")

    num_input=-1

    ###########################################################################################
    # Add runtime 8-bit only comparisons between override coefficients (when provided) and original coefficients
    if len(dic_external_coefficients)>0:
        imp_file.write("  printf(\"array         | size    | min %%err | max %%err | avg(%%err) | avg(abs(%%err))\\n\");\n")
        imp_file.write("  float sum_error, sum_abs_error, max_error, min_error;\n")
        imp_file.write("  int firstErrors;\n")
        for key, value in dic_coefficient_sizes.items():
            if key not in dic_external_coefficients:
                continue
            layer_name = key.replace('weights_','').replace('biases_','')
            lp = dic_name_to_layer_params[layer_name]
            min_original = lp.min_b if "biases" in key else lp.min_w
            max_original = lp.max_b if "biases" in key else lp.max_w

            imp_file.write("  sum_error=0; sum_abs_error=0; max_error=0; min_error=0; firstErrors=20;\n")
            imp_file.write("  for (long long i=0;i<" + str(value) + ";i++) {\n")
            imp_file.write("    int original=" + str(key) + "[i];\n")
            imp_file.write("    int signed_original=original-128;\n")
            imp_file.write("    float float_original = " + str(min_original) + " + (float)original/255*(" + str(max_original) + "-(" + str(min_original) + "));\n")
            imp_file.write("    int override=" + str(dic_external_coefficients[key]) + "[i];\n")
            imp_file.write("    int signed_override=override-128;\n")
            imp_file.write("    float float_override = min_"+str(key)+"[0] + (float)override/255*(max_"+str(key)+"[0] - min_"+str(key)+"[0]);\n")
            imp_file.write("    float diff = (float_override - float_original)/(max_"+str(key)+"[0] - min_"+str(key)+"[0] + .01);\n")
            imp_file.write("    float abs_diff = diff>0?diff:-diff;\n")
            imp_file.write("    if ((abs_diff>.1) && (firstErrors-->0)) printf(\"  %s[%lld] override (float_override=%f, fix_override=%d) != original (float_original=%f, fix_original=%d from [%f,%f])\\n\",\""+
                key+"\",i,float_override, signed_override, float_original, signed_original,"+str(min_original)+","+str(max_original)+");\n")
            imp_file.write("    max_error = diff>max_error?diff:max_error;\n")
            imp_file.write("    min_error = diff<min_error?diff:min_error;\n")
            imp_file.write("    sum_error += diff;\n")
            imp_file.write("    sum_abs_error += abs_diff;\n")
            imp_file.write("  }\n")
            imp_file.write("  printf(\"%-20s | %-10d | %3.6f | %3.6f | %3.6f | %3.6f\\n\",\"" + 
                key + "\"," + str(value) + ",min_error,max_error, (float)sum_error/" + str(value) + ",(float)sum_abs_error/" + str(value) + ");\n") 
        imp_file.write("  printf(\"Range for const arrays: override vs. original [min:zero point:max]\\n\");\n")
        for key, value in dic_coefficient_sizes.items():
            if (mini(key)) not in dic_external_extrema:
                continue
            layer_name = key.replace('weights_','').replace('biases_','')
            lp = dic_name_to_layer_params[layer_name]
            min_original = lp.min_b if "biases" in key else lp.min_w
            max_original = lp.max_b if "biases" in key else lp.max_w

            imp_file.write("  printf(\"  %-20s: [%3.6f;%3.6f;%3.6f] vs. [%3.6f;%3.6f;%3.6f]\\n\",\"" + key + "\"," +
                "min_"+str(key)+"[0],-255*min_"+str(key)+"[0]/(max_"+str(key)+"[0]-min_"+str(key)+"[0]),max_"+str(key)+"[0]," + 
                str(min_original) +",255*(" + str(-min_original/(max_original-min_original)) + ")," + str(max_original) + ");\n") 

    ###########################################################################################
    # Add graph nodes

    # In case input parameters specified at the top instead of using an input layer
    if len(net_txt.input_dim):
        layer_name=net_txt.input[0]
                
        imp_file.write("\n  // Nodes derived from Caffe input parameters\n")

        append_node("INPUT","NN_PAD_NA",0,1,"input_data")
        if not generate_float_imp:
             if use_float_input:
                 if layer_name not in dic_name_to_layer_params:
                     append_node("AutoQuantize","NN_PAD_SAME",1,3,autoquant(layer_name))
                 else:
                     lp = dic_name_to_layer_params[layer_name]
                     append_node("AutoQuantize","NN_PAD_SAME",1,3,autoquant(lp.top[0]))
             else:
                 append_min_max_const_nodes(layer_name)

    for layer in txt_layers:

        type = specific_type(layer.type)
        if type not in dic_layer_to_nodes:
            continue

        imp_file.write("\n  // Caffe layer " + layer.name + " of type " + str(layer.type) + "\n")
        lp = dic_name_to_layer_params[layer.name]
        num_input = 0 if len(lp.bottom)==0 else dic_top_to_layer_params[lp.bottom[0]].num_output
        num_output = lp.num_output
        nodes = dic_layer_to_nodes[type]
        layer_parser = dic_layer_parsers[base_type(layer.type)]

        kernel_size=eval(layer_parser.kernel_size) if layer_parser.kernel_size != NA else 1
        
        # needed in the branch eval below
        if len(lp.bottom)>0:
            input_width=dic_top_to_layer_params[lp.bottom[0]].output_width
            input_height=dic_top_to_layer_params[lp.bottom[0]].output_height

        for node in nodes:
            if is_input(layer.type) and not use_float_input and not generate_float_imp:
                append_min_max_const_nodes(layer.top[0])

            for branch in node.inputs:
                b=eval(branch.b)
                w=eval(branch.w)
                h=eval(branch.h)
                d=eval(branch.d)
                size=branch.size
                if branch != B_ACTIVATION and branch != B_ACTIVATION_2:
                    if b*w*h*d == 1:  # For now, simplistic way of distinguishing constants from others
                        append_const_node(branch.name+"_"+layer.name,b,w,h,d,size)
                    else:
                        append_const_node_with_external_coefficients(branch.name+"_"+layer.name,b,w,h,d,size)
                        
            append_node(node.op,node.pad,len(node.inputs),node.output_type.num_branches,node.name+"_"+lp.top[node.output_index])  

    ###########################################################################################
    # Append output node

    if generate_binary_model==1:
        append_node("OUTPUT","NN_PAD_NA",num_output_nodes,0,output_array_name)
    else:
        for layer in txt_layers:
            for top in layer.top:
                if top in unconnected_tops:
                    if type not in dic_layer_to_nodes:
                        continue
                    append_node("OUTPUT","NN_PAD_NA",1,0,output(top))        
    
    imp_file.write("}\n");

    data_file.close()
    data_file_32b.close()
    data_file_float.close()
    imp_file.close()

if __name__ == "__main__":

    args = parse_args()

    # Check for options in config file
    if args.config is not None and os.path.exists(args.config):
        with open(args.config, 'r') as f:
            dic_configs = yaml.load(f)
        dic_external_extrema = dic_configs["dic_external_extrema"] if "dic_external_extrema" in dic_configs else dic_external_extrema
        dic_external_coefficients = dic_configs["dic_external_coefficients"] if "dic_external_coefficients" in dic_configs else dic_external_coefficients
        check_init = dic_configs["check_init"] if "check_init" in dic_configs else check_init
        generate_binary_model = dic_configs["generate_binary_model"] if "generate_binary_model" in dic_configs else generate_binary_model
        leave_coefficients_unchanged = dic_configs["leave_coefficients_unchanged"] if "leave_coefficients_unchanged" in dic_configs else leave_coefficients_unchanged
        debug_level = dic_configs["debug_level"] if "debug_level" in dic_configs else debug_level
        perf_dump = dic_configs["perf_dump"] if "perf_dump" in dic_configs else perf_dump
        generate_float_imp = dic_configs["generate_float_imp"] if "generate_float_imp" in dic_configs else generate_float_imp
        use_runtime_requantization = dic_configs["use_runtime_requantization"] if "use_runtime_requantization" in dic_configs else use_runtime_requantization
        use_8b_biases = dic_configs["use_8b_biases"] if "use_8b_biases" in dic_configs else use_8b_biases
        use_float_input = dic_configs["use_float_input"] if "use_float_input" in dic_configs else use_float_input
        min_input = dic_configs["min_input"] if "min_input" in dic_configs else min_input
        max_input = dic_configs["max_input"] if "max_input" in dic_configs else max_input

    # Options in config are overridden by command line arguments
    if args.check_init is not None:
        check_init=int(args.check_init)
    if args.generate_binary_model is not None:
        generate_binary_model=int(args.generate_binary_model)
    if args.leave_coefficients_unchanged:
        leave_coefficients_unchanged=int(args.leave_coefficients_unchanged)
    if args.debug_level is not None:
        debug_level=int(args.debug_level)
    if args.perf_dump is not None:
        perf_dump=int(args.perf_dump)
    if args.generate_float_imp is not None:
        generate_float_imp=int(args.generate_float_imp)
    if args.use_runtime_requantization:
        use_runtime_requantization=int(args.use_runtime_requantization)
    if args.use_8b_biases is not None:
        use_8b_biases=int(args.use_8b_biases)
    if args.use_float_input is not None:
        use_float_input=int(args.use_float_input)
    if args.min_input is not None:
        min_input=float(args.min_input)
    if args.max_input is not None:
        max_input=float(args.max_input)

     # check for argument errors
    if not os.path.isfile(args.proto):
       print ("%s + prototxt file not found "%args.proto)
       sys.exit()

    if not os.path.isfile(args.model):       
       print  ("%s + model file not found "%args.model)
       sys.exit()

    if (not use_float_input and not generate_float_imp) and (min_input is None or max_input is None):
        print ('Must specify the floating-point range represented by the uint8 input data.')
        sys.exit()
    elif (use_float_input or generate_float_imp) and (min_input != None or max_input != None):
        print ('No need to specify the floating-point range represented by the uint8 input data when using float input data.')
        sys.exit()

    start = time.time()
    run(args.proto, args.model, args.root)
    end = time.time()

    print ("Generating Hexagon NN implementation took %f seconds." % (end - start))
