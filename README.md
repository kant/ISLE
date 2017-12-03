#Build

Both Linux and Windows 10 builds use Intel(R) MKL(R) library. We used 2017 version; other versions might work as well.
Suppose you have installed MKL at <MKL_ROOT>. In Linux, this is typically `/opt/intel/mkl`.
In Windows 10, this is typically `C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2017\windows\mkl`

## Linux
```
export LD_LIBRARY_PATH=<MKL_ROOT>/lib/intel64/:.
make -j
```
This should generate two executables `trainFromFile` and `inferFromFile` in the root directory.


## Compilation instructions for VS2015

Load `ISLE.sln`, and build the `ISLETrain` and `ISLEInfer` projects.  

If there is a problem with this, you could configure project file properties as follows:
* Under VC++ Directories >
  ** Include '.' folder to path, to include Eigen and Spectra include folders.
  ** Include '<MKL_ROOT>\include\intel64\ilp64'

* Under C/C++ >
  ** Enable optimizations, and disable runtime and SCL checks (this conflicts with turning on optimizations).
  ** Code Generation > Enable Parallel Code
  ** Languages > Enable OpenMP support

* Under 'Intel Performance Libraries' >
  ** Enable 'Parallel' option.
  ** Enable MKL_ILP64 option for 8-byte MKL_INTs.

* Under Linker > Input > Additional dependencies:
  ** `<MKL_ROOT>\lib\intel64_win\mkl_core.lib`
  ** `<MKL_ROOT>\lib\intel64_win\mkl_intel_lp64.lib`
  ** `<MKL_ROOT>\lib\intel64_win\mkl_sequential.lib`

# Training on a dataset

1. Create two files for input 
        a. one file has <doc_id> <word_id> <frequency> entries, one per line.
        b. second file provides the mapping from word id to the word; the i-th line of the file is the word with word_id=i.
2. Run ??


# Inference for a dataset using the trained model



# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
