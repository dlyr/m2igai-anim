# Compilation instruction, fast and maybe not portable

## first clone with submodule

```{.sh}
git clone --recurse-submodules git@github.com:dlyr/m2igai-anim.git
```

or 

```{.sh}
git clone git@github.com:dlyr/m2igai-anim.git
git submodule update --recursive --init
```

## second builds external

```{.sh}
cd m2igai-anim/external
cmake -S . -B build
cmake --build build -j 
```

This will install the dependencies in m2igai-anim/external/Bundle-GNU (or similar depending on your compiler)


## Then build the main project

go back to m2igai-anim
```{.sh}
cd ../
```

and build with the external install dir specified

```{.sh}
cmake -S . -B build  -DCMAKE_PREFIX_PATH=external/Bundle-GNU/
cmake --build build -j
```

run `main-app` from install dir (e.g. `m2igai-anim/Bundle-GNU/Debug/`)
