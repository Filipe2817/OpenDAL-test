# OpenDAL Notes

## Useful Links

[Apache OpenDAL Website](https://opendal.apache.org/)

[Apache OpenDAL Repository](https://github.com/apache/opendal)

[Xuanwo Blog](https://bento.me/xuanwo)

[Apache OpenDAL: One Layer, All Storage (Xuanwo)](https://youtu.be/eVMIzY1K5iQ): Presentation video from the creator

## Why?

### Why not S3 API

- Not all storage services are S3-compatible (Azure)

- Not all S3-compatible services are fully compliant (XML API Google Cloud Storage does not support delete objects. A lot of edge cases in multiple services)

- S3-compatible only is not enough for production (AWS IAM, many S3 compatible services do not provide the same IAM compatible API's. The users need implement their own features that could create unsafe, unreliable and poorly performing systems)

- A better SDK for storage access (OpenDAL remains a better SDK for users focused solely on storage access, easy to use even for non-experts in object storage)

### Why not Vendor's SDK

- No official Rust SDK (not every storage service has an official Rust SDK or if it does, it may not keep up with the latest features)

- Not well-mainained or well-designed (lack of documentation, poorly designed API's, insufficient test, depending on outdated dependencies)

- Challenge for integrating with multiple SDKs (even if the SDK is good, it's hard to integrate with multiple SDKs. OpenDAL implements all the API's from scratch, so it's easier to integrate with multiple services and control behavior)

### Why not build yourself

- S3 API is only simple for happy path (realistic production has too many things to take in consideration; 200 OK with failed upload, so this errors need to be handled carefully)

- It's not worth. Just reuse building blocks (contributing for the community is better than building yourself)

### Why not posix fs API

- FUSE is slow and difficult to work

- DFS is complex, often starts with POSIX, ends with a new SDK (The more optimization requires, the further they move away from the simple FUSE fs API)

- New Trend: Data Lake, Cloud Computing (DB's often need to write directly to S3, in such cases relying on POSIX fs is not feasible)

## Design

**SERVICE --------> LAYER --------> OPERATOR**

**SERVICE:** S3, fs, etc all implement the same trait in rust (services do not use API's from the vendor, they implement the API's from scratch)

**LAYER:** on the top of the services, there are layers that apply common functionalities (retry handling, metrics, logging, ...)

**OPERATOR:** everything encapsulated within an operator, that is an entry point for OpenDAL users

Example S3:

- Service: PUT, GET, etc

- Layer: Tracing, Automatic Retry, Concurrency limit, etc (layers can be enabled based on needs)

- Operator: Expose API redirect directly (read, write, etc)

## Features

- OpenDAL just provides the features and the user configures them to their needs

- No need to worry about the underlying service
  - local fs -> write_at, S3 -> multipart upload, Azure Blob -> put_block. With OpenDAL it's just write for all of them
  - no need to worry about offset, updload_id, block_id. OpenDAL handles that

## Downsides

- If a Vendor has a different feature not common to all services of that type, OpenDAL will probably not support it because it aims to provide a consistent view for all storage (in this case it's better to use the service SDK directly)

## Service/Platform Support

> Notes:
>> Can be run in WebAssembly (Parquet Viewer)
>
>> It's possible to compose layers (cache with memcached and object storage with s3)
>

- IPFS (not used widely, inefficient) can be used to create blockchain databases

### Methods

- `OAY`: API Gateway for OpenDAL
  - Enable users to access different storage services using APIs they already know
  - Simplify working with multiple storage backends
  - Initial focus on S3 compatibility

- `OLI`: OpenDAL Command Line Interface
  - Unified way to manage data across multiple backends
  - Interact with various storage services using familiar file manipulation commands (ls, cp, rm, etc.)

- `OFS`: OpenDAL Filesystem
  - Mount remote storage backends as local directories
  - Interact with cloud storage as if it were local storage
  - Limitations: fs (local filesystem) using FUSE; S3; Linux-only

### Integrations (Rust Crates)

- `dav-server-opendalfs`: WebDAV server implementation built with OpenDAL

- `object_store_opendal`: Object storage implementation built with OpenDAL

- `fuse3_opendal`: Mount and access storage via FUSE3 integration

- `virtiofs_opendal`: Access storage through VirtioFS protocol (vhost-user backend integration)

- `unftp-sbe-opendal`: unFTP storage backend using OpenDAL

- `parquet_opendal`: Efficient Parquet I/O utilities with OpenDAL

- `spring`: Integration between OpenDAL and Spring Framework

- `opendal-compat`: Compatibility functions for OpenDAL (compatibility between different versions)

- `cloud_filter_opendal`: Integrates OpenDAL with cloud sync engines

### Services

1. **Standard Storage Protocols**
    - `ftp`: File Transfer Protocol
    - `http`: Hypertext Transfer Protocol
    - `sftp`: SSH File Transfer Protocol
    - `webdav`: Web-based file sharing protocol

2. **Object Storage Services**
    - `azblob`: Azure Blob
    - `cos`: Tencent Cloud
    - `gcs`: Google Cloud
    - `obs`: Huawei Cloud
    - `oss`: Alibaba Cloud
    - `s3`: AWS S3
    - `b2`: Backblaze B2
    - `openstack_swift`: OpenStack
    - `upyun`: Chinese cloud
    - `vercel_blob`: Vercel Blob

3. **File Storage Services**
    - `fs`: Local file system
    - `alluxio`: Virtual Distributed file system
    - `azdls`: Azure Data Lake Storage
    - `azfile`: Azure Files Service
    - `chainsafe`: Decentralized File Storage  
    - `compfs`: Completion-based asynchronous file system for high-performance I/O
    - `dbfs`: Databricks File System
    - `gridfs`: MongoDB file storage system
    - `hdfs`: Apache Hadoop distributed file system
    - `hdfs_native`: Native Rust HDFS
    - `ipfs`: Decentralized File Storage
    - `webhdfs`: Web-based HDFS interface

4. **Consumer Cloud Storage Services**
    - `aliyun_drive`: Alibaba Cloud Personal Storage
    - `gdrive`: Google Drive Cloud Storage
    - `onedrive`: Microsoft Cloud Personal Storage
    - `dropbox`: Cloud File Sharing Storage
    - `icloud`: Apple Cloud Personal Storage
    - `koofr`: European Cloud Storage
    - `pcloud`: Cloud Storage and Backup Service
    - `seafile`: Open Source File Sync and Share Cloud Storage
    - `yandex_disk`: Yandex Cloud Storage

5. **Key-Value Storage Services**
    - `cacache`: On-disk caching (Rust Crate)
    - `cloudflare_kv`: Cloudflare Workers KV Storage
    - `dashmap`: Concurrent hash map for Rust
    - `memory`: In-memory key-value store
    - `etcd`: Distributed key-value store
    - `foundationdb`: Open Source, Distributed, Fault-tolerant Database
    - `persy`: Transactional Storage (Rust Crate)
    - `redis`: Key-value store Database/Cache
    - `rocksdb`: Key-value store Database for Fast Storage
    - `sled`: Transactional Embedded key-value Database (Rust Crate)
    - `redb`: Embedded key-value Database (Rust Crate)
    - `tikv`: Distributed key-value Database
    - `atomicserver`: Open Source Graph Database

6. **Database Storage Services**
    - `d1`: Cloudflare Distributed SQL Database
    - `mongodb`: NoSQL Database
    - `mysql`: Relational Database
    - `postgresql`: Open Source Relational Database
    - `sqlite`: Embedded Relational Database
    - `surrealdb`: Distributed Multi Query-Language (SQL, GraphQL, etc) Database

7. **Cache Storage Services**
    - `ghac`: GitHub Actions Cache Storage
    - `memcached`: Open Source Distributed Memory Object Caching System
    - `mini_moka`: Concurrent In-memory Cache (Rust Crate; Moka Light Edition)
    - `moka`: Concurrent In-memory Cache (Rust Crate)
    - `vercel_artifacts`: Vercel Remote Caching Service

8. **Git-Based Storage Services**
    - `huggingface`: ML Models, Datasets and Demo Apps Storage

## Installation

### Rust Core

```bash
cargo add opendal
```

Try it out: [Rust Demo](demos/rust_demo.rs)

### C Binding

#### Prerequisites to build OpenDAL (there are others specific to contribution)

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

#### Build OpenDAL

Clone the OpenDAL repository and add a .env file with the absolute path to the repository \
Follow the [.env.example](.env.example)

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

#### Remove OpenDAL

Run the makefile clean OpenDAL rule:

```bash
make clean-opendal
```

## Rust Features

TO DO

- Supports async operations

- write_with provides options but does not seem to support random writes. Docs: [write_with](https://opendal.apache.org/docs/rust/opendal/struct.Operator.html#method.write_with)

- Supports concurrent writes and reads

- Supports random reads

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
