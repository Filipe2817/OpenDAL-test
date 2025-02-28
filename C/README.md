# OpenDAL C Binding Test

## Installation

### Prerequisites to build OpenDAL (there are others specific to contribution)

- Ubuntu / Debian

```bash
# install C/C++ toolchain
sudo apt install -y build-essential

# install CMake
sudo apt-get install cmake

# install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

- Arch

```bash
# install C/C++ toolchain
sudo pacman -S base-devel

# install CMake
sudo pacman -S cmake

# install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Build OpenDAL

Clone the OpenDAL repository and add a .env file with the absolute path to the repository \
Follow the [.env.example](../.env.example)

Run the makefile build OpenDAL rule:

```bash
make build-opendal
```

Try it out: [C Demo1](demos/c_demo.c) and [C Demo2](demos/c_error_demo.c)

```bash
# Compile the demos
make demos
# Run the demos
./build/demos/c_demo
./build/demos/c_error_demo
```

### Remove OpenDAL

Run the makefile clean OpenDAL rule to remove the build files:

```bash
make clean-opendal
```

## Running Code

Use the Makefile provided. It is pretty straightforward.

To find the available rules, run:

```bash
make help # or just make (help is the default rule)
```

The `make <dir>` rule is used exactly like specified, like in the example above with the `make demos` rule.

## C Features

- Error info (code/enum + description)

- Operator:
  - Objects: create_dir, delete, exists, rename, stat (metadata), list (returns a lister)
  - Info: get_<full/native>_capability (native and full), get_name, get_root (path), get_scheme (service)
  - Options: set (settings and credentials specific to the service)
  - Functions: read (data fully loaded), reader (incremental reading), reader_read, write (data fully written), writer (incremental writing), writer_write

- Metadata: length, last modified, is dir, is file

- Lister (iterator of objects under a path): entry_name; entry_path; next (lister examples on docs are wrong/outdated)

> Notes:
>> Most (not sure if all) of these features are blocking (not async)
>
>> Everything works based on error checking
>
>> Almost everything is allocated on the heap so it can be tricky to manage memory since in most cases freeing opendal objects is not enough and the user needs to free the inner objects as well
