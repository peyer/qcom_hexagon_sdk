
# Using virtual environments

*Note: Using a virtual environment is optional.*

virtualenv is a tool to create isolated Python environments. virtualenv creates a folder which contains all the necessary executables to use the packages that a Python project would need. This approach will ensure that your python dependencies are identical to those with which the library has been tested internally and thus maximize your chances of running your code successfully from the beginning.

## One-time setup

* If you do not have virtualenv installed, you can install it with  
```$ pip install virtualenv```

* Create a new virtual environment  
```$ virtualenv -p python3 env3```

*Note: For Windows, only python 3.6.7 is supported. Please install it from https://www.python.org/ftp/python/3.6.7/python-3.6.7-amd64.exe.*

* Activate your environment
```$ source env3/bin/activate```

* Install the latest version of pip
```
$ pip install -U pip
```

* Install all dependencies as specified in the `environment` folder:
```$ pip3 install -r environments/req3.txt```

## Activating and deactivating your environment

Once the one-time setup has been complete, you should be able to get in and out of your environment as follows.
To activate the virtual environment:
```
$ source env3/bin/activate
```
You should now be within your `env3` environment and, for example, be able to use tensorflow from python:
```$ python
Python 3.4.3 (default, Nov 28 2017, 16:41:13)
[GCC 4.8.4] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import tensorflow as tf
>>> exit()
$
```

This environment should include the same python dependencies as those tested for the release of this project.
To exit your environment, simply type:
```
$ deactivate
```
