# VecTHOR
This documentation briefly presents the seven approaches that have
been developed in the context of _Test Vector
Transmitting using enhanced compression-based TAP controllers_ (VecTHOR)’s retarget frame-
work.

These approaches include four fast heuristic methods that invoke a word-based Huffman encoding technique
and, furthermore, three formal approaches using a Pseudo-Boolean Optimization solving technique.
Since the basic principle of these retargeting techniques have been described
in [REF-TBA](http://tba.de), this document focuses on the software-side of the developed framework.

# Prerequistes

## Required packages for documentation (latex only)
texlive-multirow texlive-hanging texlive-stackengine texlive-sectsty texlive-tocloft texlive-newunicodechar texlive-etoc

## Test classes
    * SBDCM | passed
    * SCM
    * SDCM
    * SPC
    * SSCPP
    * SSC
    
# Overview of Retargeting Techniques
The individual retargeting technique can be enabled by setting the configuration accordingly,
which is further described in the remainder.

Four different heuristic retargeting techniques have been developed.
These techniques allow a fast retargeting completion even for large incoming data.
Depending on the applied technique, a promising test data volume reduction ratio is
achieved.
In contrast to this, these techniques typically introduce an over-
head concerning the test application time.
More precisely, the Compress technique executes
a heuristic approach while solely considering the pre-configured embedded
dictionary without any reconfiguration. As a reference, the Merge-Compress
technique introduces a run-length encoding capability to the retargeting pro-
cedure, which allows merging long series of same datawords effectively.

The
Dynamic-Compress technique takes advantage of the dynamic reconfiguration
of the embedded dictionary, which improves the compression efficacy signif-
icantly. Finally, Dynamic-Merge-Compress combines both the dynamic recon-
figuration with the run-length encoding capability and, by this, achieves the
best results from the heuristic approach with respect to the reduction of the
test data volume and test application time, respectively.

* Compress
* Merge-Compress
* Dynamic-Compress    
* Dynamic-Merge-Compress

Besides the heuristic approaches, different techniques have been proposed in-
voking formal techniques. More precisely, the task of determining a most ben-
eficial set of codewords has been mapped to a Pseudo-Boolean optimization instance, yielding the
three approaches as follows:

* SAT-Compress
* SAT-Postprocess-Compress
* Partition-SAT-Compress
	
# Getting Started
Since VecTHOR is solely implemented in _C++_ and utilizes heavily _C++11_ features, a compatible _C++_
compiler version has to be used. The code has been tested with the GNU
Compiler Collection _gcc v.4.9_ or newer. Besides the compiler, the boost C++
Library is required with a version compatible to version 1.41.0 and
the cross-building platform CMake in version 2.8 or newer.
The framework orchestrates _clasp 3.1.4_ as the underlying Pseudo-Boolean optimization solving engine and
YAML to process user-defined configuration files.
Note that the retargeting frame operates in a terminal-only mode. For a future
version of VecTHOR, a graphical user-interface is planned as an additional
feature. After the compilation has been completed, VecTHOR can be executed
by calling the compiled and linked executable.

## Required system packages
* libyaml-cpp

## Required external packages
* clasp-3.1.4
* stil (optional)
* libboost
 * filesystem
 * regex
 * iostreams
 * program_options
 * system
        
# Available Options
TBA

# (Planned) Future Works
1. One future feature concerns a graphical user interface, which will
simplify the users’ interaction and provide important information about the
retargeting progress and compression success.
2. From an engineering point of view, it is also planned to parallelize certain parts of the retargeting process.
3. Furthermore, a client-server architecture should be introduced as an optional
feature allowing for a distribution of the retargeting core and the emitter,
which will possibly yield an easier integration into existing flows.
4. From an algorithmic point of view, it is planned to integrate a multi-value
encoding scheme into VecTHOR’s retargeting framework. This implies that
the Pseudo-Boolean Optimization instance is extended as well, which will allow modeling X-values.
This extension requires an efficient encoding scheme and, hence, addresses a
highly relevant research question. It is assumed that this will further increase
compression efficacy since some test data are typically unspecified in different
test applications. Consequently, the full potential is not yet revealed in such
a circumstance.
