# NewDetKDecomp

This project includes a library of tools to analyze hypergraph properties and 
algorithms to detect the (generalized) hypertree width.

## Getting Started

These instructions will get you a copy of the project up and running 
on your local machine for development and testing purposes. 

### Prerequisites

In order to compile NewDetKDecomp you only need a C++ compiler supporting C++14.

### Installing

Compile the programs by running make in the root directory.

```
make
```

This creates the binaries `hg-tools`, `detkdecomp`, `globalbipkdecomp`, `localbipkdecomp` and `balsepkdecomp`.

## Authors


* **Wolfgang Fischl** - *Algorithms on LocalBip, GlobalBip and BalSep*

## Links

* A website with benchmark results using these tools can be found here: [HyperBench](http://hyperbench.dbai.tuwien.ac.at)


## Acknowledgments

* Thanks to **Marko Samer** for the *Initial work*  on the [DetKDecomp Algorithm](https://dl.acm.org/citation.cfm?id=1412229)
* **Georg Gottlob** and **Reinhard Pichler** for their help on the theoretical underpinnings of the algorithms

## Publications

[[1]](https://arxiv.org/abs/1611.01090) W. Fischl, G. Gottlob, R. Pichler: General and Fractional Hypertree Decompositions: Hard and Easy cases. accepted for PODS'18.

[2] W. Fischl, G. Gottlob, D. M. Longo, R. Pichler: HyperBench: A Benchmark and Tool for Hypergraphs and Empirical Findings. Submitted to PODS'18.

