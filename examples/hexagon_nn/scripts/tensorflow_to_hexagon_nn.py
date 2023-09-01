########################################################################################
#  This script converts quantized proto buf to C file
#  python tensorflow_to_hexagon_nn.py <quantized proto buf> <configuration YAML file> 
#  Example: python tensorflow_to_hexagon_nn.py quantized_graph_opt.pb conf.yaml 
#
#  This script is provided as is. It is intended to provide an example on how one could
#  interface the Hexagon NN library.  It is solely up to the user to modify the script
#  further to meet their own needs.
#
####################################################################################### 
import sys
import os
import os.path
import tensorflow as tf
from tensorflow.python.platform import gfile
import re
import math
import numpy as np
import yaml

global input_values

# Id we want to start from
id_counter = 0x10000

append_const_list = []
append_node_list = []
input_values = {}
consumer_nodes = {}
opdict = []
results = {}
node_to_id = {}
id_to_node = {}
shape_ids = {}
seen = {}

def realopname(opname,op0type):
	"""Change a TF op name to a hexagon_nn graph name"""
	convdict = {
		"QuantizedConv2D": "OP_QuantizedConv2d_8x8to32",
		"QuantizedMatMul": "OP_QuantizedMatMul_8x8to32",
		"QuantizeDownAndShrinkRange": "OP_QuantizeDownAndShrinkRange_32to8",
		"QuantizedRelu": "OP_QuantizedRelu_8",
		"QuantizedReluX": "OP_QuantizedReluX_8",
		"QuantizedClamp": "OP_QuantizedClamp_8",
		"QuantizedMaxPool": "OP_QuantizedMaxPool_8",
		"QuantizedAvgPool": "OP_QuantizedAvgPool_8",
		"QuantizedBiasAdd": "OP_QuantizedBiasAdd_8p8to32",
		"QuantizedAdd": "OP_QuantizedAdd_8p8to32",
		"QuantizedMul": "OP_QuantizedMul_8x8to32",
		"QuantizeV2": "OP_Quantize",
		"Dequantize": "OP_Dequantize",
		"QuantizedConcat": "OP_QuantizedConcat_8",
		"QuantizedInstanceNorm": "OP_QuantizedInstanceNorm_8",
		"Requantize": "OP_Requantize_32to8",
		"RequantizationRange": "OP_RequantizationRange_32",
		"QuantizedReshape": "OP_QuantizedReshape",
		"Conv2D": "OP_Conv2d_f",
		"NoOp": "OP_Nop",
		"Identity": "OP_Nop",
		"Squeeze": "OP_Nop",
		"INPUT" : "OP_INPUT",
		"Flatten" : "OP_Flatten",
		"Reshape" : "OP_Reshape",
		"DepthwiseConv2dNative" : "OP_DepthwiseConv2d_f",
		"MatMul": "OP_MatMul_f",
		"BatchMatMul": "OP_MatMul_f",
	}
	typedict = {
		'float32' : "_f",
		'int32' : "_int32",
		'quint8' : "_FIXME_QUINT8",
		'qint32' : "_FIXME_QINT32",
		'uint8' : "_FIXME_UINT8",
		'' : "",
	}
	if opname in convdict: return convdict[opname]
	return "OP_"+opname+typedict[op0type]

def emit_fancy_data_array(dname,dtype,data,special_note):
	data = data.flatten().tolist()
	maxval = max(data)
	minval = min(data)
	maxval = max(maxval,-minval)
	stepsize = maxval/128
	n_zeros = len(filter(lambda x: abs(x) < stepsize, data))
	print ( '// number of %s values close to zero (%s): %d/%d (%f%%)' % (dname,special_note,n_zeros,len(data),100*float(n_zeros)/len(data)))

def emit_data_array(dname,dtype,data,special_note=""):
	"""Spit out a data array, named dname and typed with dtype's type.
	   The data array is print (ed however the type wants to be converted to str,
	   so if there's some funky class that print (s out incompatible to C you'll
	   need to fix that here."""
	def omit():
		print ( "/* OMITTED */ };")
		return 1
	"Print out the data array and return total size"
	print ( "static %s %s[] ALIGNED = {" % (dtype.name,dname))
	data = data.flatten().tolist()
	strdata = [str(x) for x in data]
	for i in range(0,len(strdata),16):
		print ( "  %s," % (",".join(strdata[i:i+16])))
	print ( "};")
	return dtype.size * len(data)

def emit_tensordef(tname,dname,dsize,shape):
	"""Emit a definition of the struct tensor named tname, with previously 
	   prototyped data named dname, sized dsize, and with shape shape."""
	vals = {
		'tname' : tname,
		'dname' : dname,
		'b' : shape[0],
		'h' : shape[1],
		'w' : shape[2],
		'd' : shape[3],
		'size' : dsize,
	}
	print ( """
static struct tensor %(tname)s = {
	.shape = {
		.batches = %(b)d,
		.height = %(h)d,
		.width = %(w)d,
		.depth = %(d)d,
	},
	.max_size = %(size)d,
	.data_size = %(size)d,
	.self = &%(tname)s,
	.data = %(dname)s,
};""" % vals)


def emit_common_inputs(op,inputs_name,extrainputs):
	"""Helper for emit_common: emit struct input array
	   named inputs_name, with the inputs for op, 
	   and also add extrainputs"""
	print ( "static hexagon_nn_input %s[] = {" % inputs_name)
	for inp in op.inputs:
		src_id = node_to_id[inp.op.name]
		port = inp.value_index
		check_name = 'const_check_ref_for_%x_%d' % (src_id,port)
		if CHECKS_ONLY and (check_name in node_to_id):
			src_id = node_to_id[check_name]
			port = 0
		print ( " { .src_id = 0x%x, .output_idx = %d, }," % (src_id,port))
	for src_id,port in extrainputs:
		print ( " { .src_id = 0x%x, .output_idx = %d, }," % (src_id,port))
	print ( "};")

def emit_common_outputs(op,outputs_name):
	"""Helper for emit_common: emit the struct output array
	   named outputs_name, with the outputs for op."""
	print ( "static hexagon_nn_output %s[] = {" % outputs_name)
	for out in op.outputs:
		val = results[out.name]
		if not val.shape: order=0
		else: order=len(val.shape)
		(b,h,w,d) = get_4dshape(val.shape)
		size = b*h*w*d*out.dtype.size
		shapestr = ",".join([str(x) for x in val.shape])
		print ( " OUTPUT_ND(%d,%s,%d) OUTPUT_4D(%d,%d,%d,%d,%d)," % \
			(order,shapestr,out.dtype.size,b,h,w,d,out.dtype.size))
	print ( "};")

def emit_common(op,myid,type_override="",padding="NA",extrainputs=[]):
	"""Normal function for emitting what we have to do for an op.
	   Also appends the initializer information to the append_node_list."""
	if op.outputs:
		out0_type = op.outputs[0].dtype.name
	else:
		out0_type = ""
	if type_override: optype = type_override
	else: optype = op.type
	#######  if  padding  is a byte object, must convert to string
	#######  without initial  b'  and final  '  being part of the string
	#######  so as to give e.g.  NN_PAD_SAME  instead of  NN_PAD_b'SAME'
	padding = repr(padding).lstrip("b'").rstrip("'")
	print ( '// %s node "%s": id=%x' % (optype,op.name,myid))
	inputs_name = "inputs_for_%x" % myid
	outputs_name = "outputs_for_%x" % myid
	emit_common_inputs(op,inputs_name,extrainputs)
	emit_common_outputs(op,outputs_name)
	append_node_list.append( (
		op.name,
		myid,
		realopname(optype,out0_type),
		"NN_PAD_"+padding,
		inputs_name,
		len(op.inputs) + len(extrainputs),
		outputs_name,
		len(op.outputs) ))


def get_4dshape(in_shape):
	"""We always want a 4D shape.  Turn something below 4D into 4D.
	   We always want the higher orders of the shapes to be 1 
	   if not 4D."""
	if not in_shape: return (1,1,1,1)
	if len(in_shape) == 1: return (1,1,1,in_shape[0])
	if len(in_shape) == 2: return (1,1,in_shape[0],in_shape[1])
	if len(in_shape) == 3: return (1,in_shape[0],in_shape[1],in_shape[2])
	if len(in_shape) == 4: return tuple(in_shape)
	raise Exception("FIXME: interesting shape: %r" % [(in_shape)])


def emit_Const_io_and_append(myid,shape,dname,size):
	"""This emits the constant input and output definitions
	   for a Const node, and appends the node to the list"""
	append_const_list.append( (
		myid,
		shape[0], shape[1], shape[2], shape[3],
		dname,
		size,
		) )

def emit_Const_common(myid,dtype,vals,special_note=""):
	"""Common code for emitting a Const node, with specified ID, dtype, 
	   and values.  This can come from a real TF Const node, or from a
	   node we want to generate for checking."""
	shape = get_4dshape(vals.shape)
	dname = 'data_for_op_%x' % myid
	dsize = emit_data_array(dname,dtype,vals,special_note)
	emit_Const_io_and_append(myid,shape,dname,dsize)

def emit_Const(op,myid):
	"""Generate a Const node from a TF Const node"""
	print ( "// CONST!! ")
	t = op.outputs[0]
	vals = results[t.name]
	if (type(vals)==type("")): return
	emit_Const_common(myid,t.dtype,vals)

def emit_Input(op,myid):
	"""Generate the INPUT node."""
	print ( "// INPUT!! ")
	if USE_CONST_INPUT:
		results[op.outputs[0].name] = input_values[op.outputs[0].name]
		emit_Const(op,myid)
	else:
		results[op.outputs[0].name] = input_values[op.outputs[0].name]
		emit_common(op,myid,type_override="INPUT")

def emit_Output(src_id,out_id):
	"""Generate the OUTPUT node."""
	print ( "// OUTPUT!! ")
	inputs_name = "inputs_for_%x" % out_id
	print ( "static hexagon_nn_input %s[] = {" % inputs_name)
	print ( " { .src_id = 0x%x, .output_idx = %d, }," % (src_id,0))
	print ( "};")
	append_node_list.append( (
		"OUTPUT",
		out_id,
		realopname("OUTPUT",""),
		"NN_PAD_NA",
		inputs_name,
		1,
		"NULL",
		0 ))

def gen_const_shape(shape):
	"""Generate a const node that is only used as a shape."""
	if shape in shape_ids: return shape_ids[shape]
	shape_name = "const_shape_%dx%dx%dx%d" % shape
	shape_id = make_node_id(shape_name)
	shape_ids[shape] = shape_id
	emit_Const_io_and_append(shape_id,shape,"NULL",0)
	return shape_id

def gen_check_Const(dut_id,t,specialmsg=""):
	"""Generate a const node that is used for checking."""
	refname = 'const_check_ref_for_%x_%d' % (dut_id,t.value_index)
	ref_id = make_node_id(refname)
	emit_Const_common(ref_id,t.dtype,results[t.name],"CHECK_" + specialmsg)
	return ref_id

def gen_Const(dtype,values,dstid,dstinp):
	const_name = "const_val_for_%x_%d" % (dstid,dstinp)
	const_id = make_node_id(const_name)
	emit_Const_common(const_id,dtype,values)
	return const_id

def gen_Check_q(dut_id,ref_ids,dtype):
	"""Generate a quantization-aware check node."""
	checkname = 'check_for_%x_q' % (dut_id)
	check_id = make_node_id(checkname)
	inputs_name = 'inputs_for_%x' % check_id
	print ( '''
static struct input %(name)s[] = {
	{ .src_id = 0x%(dut_id)x, .output_idx = 0, },
	{ .src_id = 0x%(dut_id)x, .output_idx = 1, },
	{ .src_id = 0x%(dut_id)x, .output_idx = 2, },
	{ .src_id = 0x%(ref0)x, .output_idx = 0, },
	{ .src_id = 0x%(ref1)x, .output_idx = 0, },
	{ .src_id = 0x%(ref2)x, .output_idx = 0, },
};''' % {
	'dut_id' : dut_id,
	'ref0': ref_ids[0],
	'ref1': ref_ids[1],
	'ref2': ref_ids[2],
	'name': inputs_name,
	})
	append_node_list.append( (
		checkname,
		check_id,
		"OP_Close_q_%s" % dtype.name,
		"NN_PAD_NA",
		inputs_name,
		6,
		"NULL",
		0))

def gen_Check(dut_id,dut_port,ref_id,ref_port,dtype):
	"""Generate a check node to verify values."""
	checkname = 'check_for_%x_%d' % (dut_id,dut_port)
	check_id = make_node_id(checkname)
	inputs_name = 'inputs_for_%x' % check_id
	print ( '''
static hexagon_nn_input %s[] = {
	{ .src_id = 0x%x, .output_idx = %d, },
	{ .src_id = 0x%x, .output_idx = %d, },
};
''' % (inputs_name,dut_id,dut_port,ref_id,ref_port))
	append_node_list.append( (
		"check_%x" % dut_id,
		check_id,
		realopname("Close","float32"),
		"NN_PAD_NA",
		inputs_name,
		2,
		"NULL",
		0))

def emit_PoolLike(op,myid):
	"""Generate an op that is Pool-like (has padding/strides/ksize)"""
	padding = op.get_attr('padding')
	strides = tuple(op.get_attr('strides'))
	stride_id = gen_const_shape(strides)
	ksize = tuple(op.get_attr('ksize'))
	ksize_id = gen_const_shape(ksize)
	print ( "// POOL LIKE")
	padding = repr(padding).lstrip("b'").rstrip("'")
	print ( "// padding:  %s %s" % (padding,type(padding)))
	print ( "// strides:  %s %s %d" % (strides,type(strides),stride_id))
	print ( "// ksize:  %s %s %d" % (ksize,type(ksize),ksize_id))
	emit_common(op,myid,extrainputs=[(ksize_id,0),(stride_id,0)],padding=padding)

def emit_ConvLike(op,myid):
	"""Generate an op that is Conv-like (has padding/strides)"""
	padding = op.get_attr('padding')
	strides = tuple(op.get_attr('strides'))
	stride_id = gen_const_shape(strides)
	print ( "// CONV LIKE")
	padding = repr(padding).lstrip("b'").rstrip("'")
	print ( "// padding:  %s %s" % (padding,type(padding)))
	print ( "// strides:  %s %s %d" % (strides,type(strides),stride_id))
	emit_common(op,myid,extrainputs=[(stride_id,0)],padding=padding)

#
# OP BatchSpace
#
def emit_BatchToSpaceLike(op,myid):
    padding = op.get_attr('padding')
    strides = tuple(op.get_attr('strides'))
    block_size = op.get_attr('block_size')
    block_size_id = gen_Const(tf.int32,np.array([block_size]),myid,len(op.inputs))
    if verbosity > 0: print (("// padding: " + str(padding) + ' ' + str(type(padding))))
    if verbosity > 0: print (("// block_size: " + str(block_size)+ ' ' + str(type(block_size)) \
        + ' ' + str(block_size_id)))
    outfile.write("// BatchToSpace Ops\n")
    emit_common(op,myid,extrainputs=[(block_size_id,0)],padding=padding)

def emit_SpaceToBatchLike(op,myid):
    padding = op.get_attr('padding')
    strides = tuple(op.get_attr('strides'))
    block_size = op.get_attr('block_size')
    block_size_id = gen_Const(tf.int32,np.array([block_size]),myid,len(op.inputs))
    if verbosity > 0: print (("// padding: " + str(padding) + ' ' + str(type(padding))))
    if verbosity > 0: print (("// block_size: " + str(block_size)+ ' ' + str(type(block_size)) \
        + ' ' + str(block_size_id)))
    outfile.write("// SpaceToBatch Ops\n")
    emit_common(op,myid,extrainputs=[(block_size_id,0)],padding=padding)
#
# OP Depthspace
# 
def emit_DepthToSpaceLike(op,myid):
    block_size = op.get_attr('block_size')
    block_size_id = gen_Const(tf.int32,np.array([block_size]),myid,len(op.inputs))
    if verbosity > 0: print (("// block_size: " + str(block_size)+ ' ' + str(type(block_size)) \
        + ' ' + str(block_size_id)))
    outfile.write("// DepthToSace Ops\n")
    emit_common(op,myid,extrainputs=[(block_size_id,0)])
    
def emit_SpaceToDepthLike(op,myid):
    block_size = op.get_attr('block_size')
    block_size_id = gen_Const(tf.int32,np.array([block_size]),myid,len(op.inputs))
    if verbosity > 0: print (("// block_size: " + str(block_size)+ ' ' + str(type(block_size)) \
        + ' ' + str(block_size_id)))
    outfile.write("// SpaceToDepth Ops\n")
    emit_common(op,myid,extrainputs=[(block_size_id,0)])

def emit_Relu6(op,myid):
	id_6val = gen_Const(tf.float32,np.array([6.0]),myid,len(op.inputs))
	emit_common(op,myid,extrainputs=[(id_6val,0)],type_override="ReluX")

def emit_QRelu6(op,myid):
	id_6val = gen_Const(tf.float32,np.array([6.0]),myid,len(op.inputs))
	emit_common(op,myid,extrainputs=[(id_6val,0)],type_override="QuantizedReluX")

def emit_QRelu1(op,myid):
	id_minus1 = gen_Const(tf.float,np.array([-1.0]),myid,len(op.inputs))
	id_plus1 = gen_Const(tf.float,np.array([1.0]),myid,len(op.inputs)+1)
	emit_common(op,myid,extrainputs=[(id_minus1,0),(id_plus1,0)],type_override="QuantizedClamp")

def emit_LRN(op,myid):
	"""Generate an op that is LRN-like (has bias/alpha/beta/window)"""
	radius = op.get_attr('depth_radius')
	alpha_val = op.get_attr('alpha')
	beta_val = op.get_attr('beta')
	bias_val = op.get_attr('bias')
	print ( "// alpha/beta/bias: %s/%s/%s" % (type(alpha_val),type(beta_val),type(bias_val)))
	print ( "// alpha/beta/bias: %s/%s/%s" % (alpha_val,beta_val,bias_val))
	window_id = gen_const_shape( (1,1,1,2*radius+1) )
	id_alpha = gen_Const(tf.float,np.array([alpha_val]),myid,len(op.inputs))
	id_beta = gen_Const(tf.float,np.array([beta_val]),myid,len(op.inputs))
	id_bias = gen_Const(tf.float,np.array([bias_val]),myid,len(op.inputs))
	print ( "// LRN LIKE")
	emit_common(op,myid)

def emit_AddRank(op,myid):
	"""Generate an op which needs to know the true rank.  
	   Add this as an extra const input.
	   Reductions are a good example fo this."""
	inp = op.inputs[0]
	print ( '// %s' %( dir(inp)))
	val = results[inp.name]
	rank = len(val.shape)
	rank_id = gen_Const(tf.int32,np.array([rank]),myid,len(op.inputs))
	try:
		print ( '// keep dims =  %s %s' % (op.get_attr('keep_dims'),type(op.get_attr('keep_dims'))))
		if (op.get_attr('keep_dims')): padding = 'SAME'
		else: padding = 'VALID'
	except:
		padding = 'NA'
	print ( '// shape=%s rank=%d' % (val.shape,rank))
	print ( '// Add Rank: true rank=%d' % rank)
	print ( '// padding = %s ' % padding)
	emit_common(op,myid,extrainputs=[(rank_id,0)],padding=padding)
	

def emit_Flatten(op,myid):
	"""Emit a Flatten node.  We can still do a Reshape node, but it looked
	   like we were always flattening and that makes sense to me, not sure
	   if we'd need to rearrange data in a real reshape."""
	(b,h,w,d) = get_4dshape(results[op.outputs[0].name].shape)
	if ((b != 1) or (h != 1) or (w != 1)): raise Exception("not a flatten")
	emit_common(op,myid,type_override="Flatten")

def emit_Dequantize(op,myid):
	"""Emit a Dequantize node.  Check to see if input is qint32 or quint8"""
	if (op.inputs[0].dtype.name == 'qint32'):
		emit_common(op,myid,type_override="Dequantize_qint32")
	else:
		emit_common(op,myid)

def emit_MirrorPad(op,myid):
	padding = op.get_attr('mode')
	emit_common(op,myid,padding="MIRROR_"+padding)


def make_node_id(name):
	"""Generate a new node ID for a node named name, 
	   and update associated tables"""
	global id_counter
	ret = id_counter
	node_to_id[name] = id_counter
	id_to_node[id_counter] = name
	id_counter += 1
	return ret

def emit_node(op):
	"""Emit information for the node op"""
	node_handlers = {
		"Const" : emit_Const,
		"QuantizedConv2D" : emit_ConvLike,
		"QuantizedMaxPool" : emit_PoolLike,
		"QuantizedAvgPool" : emit_PoolLike,
		"QuantizedRelu6" : emit_QRelu6,
		"QuantizedRelu1" : emit_QRelu1,
		"Relu6" : emit_Relu6,
		"Conv2D" : emit_ConvLike,
		"AvgPool" : emit_PoolLike,
		"MaxPool" : emit_PoolLike,
		"Reshape" : emit_Flatten,
		"Dequantize" : emit_Dequantize,
		"Min" : emit_AddRank,
		"Max" : emit_AddRank,
		"Prod" : emit_AddRank,
		"Sum" : emit_AddRank,
		"Mean" : emit_AddRank,
		"All" : emit_AddRank,
		"Any" : emit_AddRank,
		"Rank" : emit_AddRank,
		"ExpandDims" : emit_AddRank,
		"Transpose" : emit_AddRank,
		"MirrorPad" : emit_MirrorPad,
		"DepthwiseConv2dNative" : emit_ConvLike,
        "DepthToSpace" : emit_DepthToSpaceLike,
        "SpaceToDepth" : emit_SpaceToDepthLike,
        "BatchToSpace" : emit_BatchToSpaceLike,
        "SpaceToBatch" : emit_SpaceToBatchLike,
	}
	seen[op.name] = 1
	myid = make_node_id(op.name)
	if op.name == start_node: return emit_Input(op,myid)
	if op.name in start_nodes: return emit_Input(op,myid)
	if op.name == end_node:
		ret = node_handlers.get(op.type,emit_common(op,myid))
		out_id = make_node_id("OUTPUT")
		emit_Output(myid,out_id)
		return ret
	return node_handlers.get(op.type,emit_common)(op,myid)


def load_data():
	"""Load data"""
	return np.zeros((1,image_height,image_width,image_depth))

def get_results(sess,data):
	"""Get all results from the graph"""
	global results
	allops = sess.graph.get_operations()
	outlist = [ o for op in allops for o in op.outputs if (o.op.name != start_node)]
	resultlist = sess.run(outlist,input_values)
	results = { a.name: b for (a,b) in zip(outlist,resultlist) }
	out_results = results[end_node+":0"].flatten()
	print ( "// %s" %( sorted(zip(range(len(out_results)),out_results),key=lambda x: -x[1])[:5]))

def load_graph(sess):
	"""Load sys.argv[1] as the graph"""
	global opdict
	global consumer_nodes
	model_filename = sys.argv[1]
	with gfile.FastGFile(model_filename, 'rb') as f:
		graph_def = tf.GraphDef()
		graph_def.ParseFromString(f.read())
		_ = tf.import_graph_def(graph_def, name='')
	allops = sess.graph.get_operations()
	opdict = { op.name: op for op in allops }
	for op in allops:
		for inp in op.inputs:
			consumer_nodes.setdefault(inp.op.name,[]).append(op)

def try_emit_check_q(ts):
	if not USE_CHECK_Q: return False
	if not len(ts) == 3: return False
	if CHECK_FILTER and ts[0].op.type not in CHECK_FILTER: return False
	for t in ts:
		if t.name not in results: return False
	print ( "// checking outputs (quantization aware): %s" % ts)
	dut_id = node_to_id[ts[0].op.name]
	ref_ids = [ gen_check_Const(dut_id,t,"_" + t.name + "_" + t.op.type) for t in ts ]
	dtype = ts[0].dtype
	gen_Check_q(dut_id,ref_ids,dtype)
	return True

def emit_output_check(t):
	"""Emit any checking we want to do for the output of a node."""
	print ( "// Maybe check? name=",t.name)
	if t.name not in results: return
	print ( "// results found for",t.name,"op type",t.op.type)
	if CHECK_FILTER and t.op.type not in CHECK_FILTER: return
	print ( "// checking output %s" % t.name)
	dut_id = node_to_id[t.op.name]
	ref_id = gen_check_Const(dut_id,t,"_"+t.name+"_"+t.op.type)
	gen_Check(dut_id,t.value_index,ref_id,0,t.dtype)

def emit_output_checks(ts):
	if not EMIT_CHECKS: return
	if try_emit_check_q(ts): return
	for t in ts:
		emit_output_check(t)

def all_inputs_seen(op):
	"""Return a truthful value if all the inputs for an operation 
	   have already been emitted, a non-truthful value otherwise."""
	for inp in op.inputs:
		if inp.op.name not in seen: return False
	return True

def try_emit_node(op,typelimit=""):
	"""Try to emit a node.  Should recurse. """
	if op.name in seen: return
	if typelimit and (typelimit != op.type):
		return
	if not all_inputs_seen(op):
		print ( "// %s waiting for input %r"%(op.name,[ x.name for x in op.inputs if x.name not in seen ]))
		return
	emit_node(op)
	emit_output_checks(op.outputs)
	for consumer in consumer_nodes.get(op.name,[]):
		if consumer.name in seen: continue
		try_emit_node(consumer)

def gen_init_function():
	print ('')
	print ( r'''
	#define APPEND_CONST_NODE(ID,...) if (hexagon_nn_append_const_node(nn_id,ID,__VA_ARGS__) != 0) \
		printf("node %d returned nonzero\n",ID)
	#define APPEND_NODE(NAME,ID,OP,...) info_for_debug(ID,NAME,#OP); \
		if (hexagon_nn_append_node(nn_id,ID,OP,__VA_ARGS__) != 0) \
		printf("node %d <%s/%s> returned nonzero\n",ID,NAME,#OP)''')
	print ( "void init_graph(int nn_id) {")
	for tup in append_const_list:
		print ( '  APPEND_CONST_NODE(0x%x,%d,%d,%d,%d,(const uint8_t *)%s,%d);' % tup)
	for tup in append_node_list:
		print ( '  APPEND_NODE("%s",0x%x,%s,%s,%s,%d,%s,%d);' % tup)
	print ( "}")

def pprint_graph():
	for (opname,myid,realoptname,padtype,inputs_name,n_inputs,outputs_name,n_outputs) in append_node_list:
		if opname == "OUTPUT": continue
		if opname not in opdict: continue
		op = opdict[opname]
		print ( 'op: %s type: %s pad: %s n_inputs: %d n_outputs: %d' % (
			opname,realoptname,padtype,n_inputs,n_outputs))
		for inp in op.inputs:
			if inp.name in results: (b,h,w,d) = get_4dshape(results[inp.name].shape)
			else: (b,h,w,d) = (-1,-1,-1,-1)
			print ( '  Input %s: %dx%dx%dx%d' % (inp.name,b,h,w,d))
		for attr in ['strides','ksize']:
			try:
				attrval = op.get_attr(attr)
				attrtup = tuple(attrval)
				print ( '  Attribute %s: %s' % (attr,attrtup))
			except:
				pass
		for outp in op.outputs:
			if outp.name not in results: continue
			(b,h,w,d) = get_4dshape(results[outp.name].shape)
			print ( '  Output %s: %dx%dx%dx%d' % (outp.name,b,h,w,d))


if __name__=="__main__":
	if (len(sys.argv) < 3):
		print (('Usage: python <this_file> <input_path/file.pb> <configuration file/conf.yaml> [<data_path/file>] [<graph flags>]'))
		exit()

	EMIT_CHECKS = False
	USE_CHECK_Q = False
	CHECKS_ONLY = False
	USE_CONST_INPUT = False
	start_node = "Mul"
	end_node = "softmax"
	start_nodes = [ "Mul" ]
	image_height = 299
	image_width  = 299
	image_depth  = 3
	CHECK_FILTER = frozenset(['QuantizedRelu','QuantizedConcat','QuantizedMaxPool','QuantizedAvgPool','QuantizedRelu6'])

	if os.path.exists(sys.argv[2]):
		with open(sys.argv[2], 'r') as f:
			inp_dict = yaml.load(f)
		EMIT_CHECKS = inp_dict["EMIT_CHECKS"]
		USE_CHECK_Q = inp_dict["USE_CHECK_Q"]
		CHECKS_ONLY = inp_dict["CHECKS_ONLY"]
		USE_CONST_INPUT = inp_dict["USE_CONST_INPUT"]
		start_node = inp_dict["start_node"]
		end_node = inp_dict["end_node"]
		start_nodes = [inp_dict["start_nodes"]]
		image_height = inp_dict["image_height"]
		image_width = inp_dict["image_width"]
		image_depth = inp_dict["image_depth"]
		CHECK_FILTER = frozenset(inp_dict["check_filter"])
 
	for arg in sys.argv:
		if arg == EMIT_CHECKS:
			EMIT_CHECKS=True
		elif arg == USE_CHECK_Q:
			USE_CHECK_Q=True
		elif arg == CHECKS_ONLY:
			CHECKS_ONLY=True
		elif arg == USE_CONST_INPUT:
			USE_CONST_INPUT=True
       
	input_values = {
            start_node+":0" :  np.zeros((1,image_height, image_width, image_depth))    
	}
	print ( '''
#include <hexagon_nn.h>
#include <stdio.h>
#include "../interface/hexagon_nn_ops.h"
typedef int32_t qint32;
typedef int32_t int32;
typedef float float32;
typedef uint8_t quint8;
#define ALIGNED __attribute__((aligned(128)))
void info_for_debug(unsigned int id, const char *name, const char *opname);
#define OUTPUT_ND(N,...) /* NOTHING */
#ifdef OLD_FORMAT
#define OUTPUT_4D(B,H,W,D,ES) { .max_size = (B*H*W*D*ES), }
#else
#define OUTPUT_4D(B,H,W,D,ES) { .rank = 4, .max_sizes = {B,H,W,D}, .elementsize=ES, }
#endif
''')

	with tf.Session() as s:
		load_graph(s)
		data = load_data()
		get_results(s,data)
		# Do const nodes first
		for op in s.graph.get_operations():
			try_emit_node(op,typelimit="Const")
		# Emit other nodes
		startop = s.graph.get_operation_by_name(start_node)
		try_emit_node(startop)
		for op in s.graph.get_operations():
			try_emit_node(op)
		gen_init_function()
		print ( '''#if 0''')
		pprint_graph()
		print ( '''#endif''')

