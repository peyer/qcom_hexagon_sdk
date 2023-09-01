# Preprocesses .jpg files into .dat files for input into graph_app
#
# Usage:
#   See command output using --help argument.
#
# Purpose
#   This script helps you generate .dat files from .jpg images. This can help
#   you validate a Hexagon NN graph that has been created for use with graph_app.
#   Specifically, this script is intended to generate test images that can validate
#   a NN generated by one of our <tensorflow/caffe>_to_hexagon_nn.py scripts.
#
# Notes
#   - If specifying a file, this script will output two things in the same directory
#       as the input image: a cropped image and a preprocessed .dat output file
#
#   - If specifying a directory, this script will create two new directories in the
#       parent directory of the specified directory which contain cropped images and
#       generated .dat files, respectively.
#
#   - By default, this script will dump the data in a .dat without any preprocessing. 
#       For help generating inputs for specific NNs, see the notes below.
#
#   - To generate .dat files for usage with a VGG16 NN, use the following command:
#
#       python jpg_to_dat.py --root vgg --input IMAGE/DIRECTORY --size 224 --mean_r 123.68 --mean_g 116.779 --mean_b 103.939 --bgr
#
#   - To generate .dat files for usage with a Inceptionv3 NN, use the following command:
#
#       python jpg_to_dat.py --root iv3 --input IMAGE/DIRECTORY --size 299 --mean_r 128 --mean_g 128 --mean_b 128 --div_scalar 128 --mul_scalar 128 --bgr
#
#   - The above commands will generate float-based .dat files, add the --byte option if seeking uint8-based .dat files
#

import argparse
import numpy as np
import os

from PIL import Image

def get_raw_img(img_filepath, use_gray):
    img_filepath = os.path.abspath(img_filepath)
    img = Image.open(img_filepath)
    img_ndarray = np.array(img)
    
    if len(img_ndarray.shape)==3:
        if (img_ndarray.shape[2] != 3):
            raise RuntimeError('Requires image with RGB but number of channels is %d' % img_ndarray.shape[2])
        if use_gray:           
           img_ndarray=np.dot(img_ndarray[...,:3],[0.299,0.587,0.114]) # directly using img.convert('L') will result to some bit mismatch
           img_ndarray=np.trunc(img_ndarray+0.5)
           num_channels=1
        else:              
           num_channels=3
    elif len(img_ndarray.shape)==2:#if the input image is already gray scaled, imd_ndarray is of two dimension.        
        num_channels=1
    else:
        raise RuntimeError('Image shape' + str(img_ndarray.shape))    
    return img_ndarray,num_channels

def create_raw_mean(img_raw, mean_rgb):
    img_dim = (img_raw.shape[0], img_raw.shape[1])
   
    mean_raw_r = np.empty(img_dim)
    mean_raw_r.fill(mean_rgb[0])    
    mean_raw_g = np.empty(img_dim)
    mean_raw_g.fill(mean_rgb[1])
    mean_raw_b = np.empty(img_dim)
    mean_raw_b.fill(mean_rgb[2])

    # create with c, h, w shape first
    tmp_transpose_dim = (img_raw.shape[2], img_raw.shape[0], img_raw.shape[1])
    mean_raw = np.empty(tmp_transpose_dim)
    mean_raw[0] = mean_raw_r
    mean_raw[1] = mean_raw_g
    mean_raw[2] = mean_raw_b

    # back to h, w, c
    mean_raw = np.transpose(mean_raw, (1, 2, 0))
    return mean_raw.astype(np.float32)

def create_raw(root, img_filepath, output_filepath, mean_rgb, div,mul, req_bgr_raw, save_byte, planar, gray):
    img_raw,num_channels = get_raw_img(img_filepath, gray)


    if num_channels==3:
       mean_raw = create_raw_mean(img_raw, mean_rgb)

       # preprocess numerically
       processed_raw = img_raw - mean_raw
       processed_raw = processed_raw.astype(np.float32)
       processed_raw /= div
       processed_raw *= mul

       # process options
       processed_raw = processed_raw[..., ::-1] if req_bgr_raw else processed_raw
       processed_raw = np.transpose(processed_raw, (2, 0, 1)) if planar else processed_raw
       processed_raw = processed_raw.astype(np.uint8) if save_byte else processed_raw.astype(np.float32)
    else:
       processed_raw=img_raw
       processed_raw = processed_raw.astype(np.uint8) if save_byte else processed_raw.astype(np.float32)
        
    # output image file to output directory
    img_base_name = os.path.splitext(os.path.basename(img_filepath))[0]
    processed_raw_filename = img_base_name + '_' + root + ('_b.dat' if save_byte else '_f.dat')
    processed_raw_filepath = os.path.join(output_filepath, processed_raw_filename)
    processed_raw.tofile(processed_raw_filepath)

    return 0

def resize_to_square(src, dst, size):
    src_img = Image.open(src)
    # check if black and white image, if so make all 3 channels the same
    if len(np.shape(src_img)) == 2: 
    	src_img = src_img.convert(mode = 'RGB')
    # center crop to square
    width, height = src_img.size
    short_dim = min(height, width)
    crop_coord = (
        (width - short_dim) / 2,
        (height - short_dim) / 2,
        (width + short_dim) / 2,
        (height + short_dim) / 2
    )
    img = src_img.crop(crop_coord)
    dst_img = img.resize((size, size), Image.ANTIALIAS)
    # save output
    dst_img.save(dst)
    return 0

def cropped_filename(filename, size):
    return os.path.splitext(filename)[0] + '_%dx%d' % (size, size) + '.jpg'

def generate_raw_files(root, dirpath, output_dirpath, mean_rgb, div_scalar, mul_scalar, use_bgr, save_byte, use_planar, use_gray):
    for filename in os.listdir(dirpath):
            if filename.endswith('.jpg') or filename.endswith('.bmp'):
                current_file = os.path.join(dirpath, filename)
                create_raw(root, current_file, output_dirpath, mean_rgb, div_scalar, mul_scalar, use_bgr, save_byte, use_planar, use_gray)

def generate_raw_file(root, input_path, output_path, mean_rgb, div_scalar, mul_scalar, use_bgr, save_byte, use_planar, use_gray):
    if input_path.endswith('.jpg') or input_path.endswith('.bmp'):
        create_raw(root, input_path, output_path, mean_rgb, div_scalar, mul_scalar, use_bgr, save_byte, use_planar, use_gray)

def main():
    SIZE_MIN = 10
    SIZE_MAX = 1000
    size_help = 'Size of edge of output jpg between %d - %d. Default is 224' % (SIZE_MIN, SIZE_MAX)

    # Argument definitions
    parser = argparse.ArgumentParser(description='Preprocesses .jpg files into .dat files for input into graph_app', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--root', help='The root name of the output directory and files.', required=True, type=str)
    parser.add_argument('-i', '--input', help='Directory or file which contains input image(s)', required=True, type=str)
    parser.add_argument('-s', '--size', help=size_help, type=int, default=-1)
    parser.add_argument('-r', '--mean_r', help='R channel mean subtraction.', type=float, default=0)
    parser.add_argument('-g', '--mean_g', help='G channel mean subtract.', type=float, default=0)
    parser.add_argument('-b', '--mean_b', help='B channel mean subtract.', type=float, default=0)
    parser.add_argument('-d', '--div_scalar', help='Division scaling done after mean subtraction.', type=float, default=1)
    parser.add_argument('-m', '--mul_scalar', help='Multiplication scaling done after mean subtraction.', type=float, default=1)
    parser.add_argument('--byte', help='Stores output pixels as uint8 elements (default is float32)', action='store_true', default=False)
    parser.add_argument('--bgr', help='Stores output color channels in BGR order (.jpg is in RGB).', action='store_true', default=False)
    parser.add_argument('--planar', help='Stores output in planar mode (default is packed).', action='store_true', default=False)
    parser.add_argument('--gray', help='Stores output in gray scale mode (default is RGB).', action='store_true', default=False)
    args = parser.parse_args()

    # Process arguments
    root = args.root
    input_path = os.path.abspath(args.input)
    size = args.size
    use_byte = args.byte
    div_scalar = args.div_scalar
    mul_scalar = args.mul_scalar
    use_bgr = args.bgr
    use_planar = args.planar
    use_gray = args.gray;

    # If saving as uint8, do not subtract mean channel values, and use min/max extrema to adjust for mean
    mean_rgb = (0,0,0) if use_byte else (args.mean_r,args.mean_g,args.mean_b)
    
    # Check specified output size
    if ((size < SIZE_MIN) or (size > SIZE_MAX)) and (size != -1):
        raise RuntimeError(size_help)

    # Check if input specified is file
    if os.path.isfile(input_path):
        filename = os.path.basename(input_path)
        # Check if file is .jpg
        if filename.endswith('.jpg') or filename.endswith('.bmp'):
            input_path = os.path.dirname(input_path)
            input_file = os.path.join(input_path, filename)

            # Check if we want to resize to square
            if size != -1:
                cropped_file = os.path.join(input_path,cropped_filename(filename,size))
                resize_to_square(input_file,cropped_file,size)
                input_file = cropped_file

            # Generate .dat file
            generate_raw_file(root, input_file, input_path, mean_rgb, div_scalar, mul_scalar, use_bgr, use_byte, use_planar, use_gray)
        else:
            raise RuntimeError('%s is not a usable file' % input_path)

    # Check if input specified is directory
    elif os.path.isdir(input_path):
        # Create directory for output
        output_dirpath = input_path + '_' + root + ('_b' if use_byte else '_f')
        if not os.path.isdir(output_dirpath):
            os.makedirs(output_dirpath)

        # Crop files
        if size != -1:
            # Create directory for cropped images
            cropped_dirpath = input_path + '_cropped'
            if not os.path.isdir(cropped_dirpath):
                os.makedirs(cropped_dirpath)

            for filename in os.listdir(input_path):
                if filename.endswith('.jpg') or filename.endswith('.bmp'):
                    input_file = os.path.join(input_path, filename)
                    output_file = os.path.join(cropped_dirpath, cropped_filename(filename,size))
                    resize_to_square(input_file,output_file,size)
            input_path = cropped_dirpath

        # Generate .dat files
        generate_raw_files(root, input_path, output_dirpath, mean_rgb, div_scalar, mul_scalar, use_bgr, use_byte, use_planar, use_gray)
    else:
        raise RuntimeError('%s is not a usable directory or file' % input_path)

    return 0

if __name__ == '__main__':
    exit(main())
